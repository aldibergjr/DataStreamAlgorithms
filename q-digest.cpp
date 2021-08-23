#include<bits/stdc++.h>

using namespace std;

vector<string> readCsvLine(istream& str)
{
    vector<string> result;
    string line;
    getline(str,line);

    stringstream lineStream(line);
    string cell;

    while(getline(lineStream,cell, ','))
    {
        result.push_back(cell);
    }
    if (!lineStream && cell.empty())
    {
        result.push_back("");
    }
    return result;
}

struct CONFIGS {
    int id_field_no;
    double eps;
    long long universe_size;
    string query_args;
    string filename;
    string execution;
    float query;
    bool compare;
    bool help_exec;
};

CONFIGS appConfig;

CONFIGS parseArgs(int argc, char * argv[]){
    CONFIGS appConfig;
    appConfig.compare = false;
    for(int i = 1; i < argc; i++){
        string paramName = argv[i];
        if(paramName.compare("--val") == 0){
            appConfig.id_field_no = stoi(argv[++i]);
        }else if(paramName.compare("--eps") == 0){
            appConfig.eps = atof(argv[++i]);
        }else if(paramName.compare("--univ") == 0){
            appConfig.universe_size = stoll(argv[++i]);
        // }else if(paramName.compare("--in") == 0){
        //     appConfig.query_args = paramName;
        }else if(paramName.compare("rank") == 0 || paramName.compare("quant") == 0){
            appConfig.execution = paramName;
            appConfig.query = atof(argv[++i]);
        }else if(paramName.compare("-h") == 0 || paramName.compare("--help") == 0){
            appConfig.help_exec = true;
        // }else if(paramName.compare("--compare") == 0){
        //     appConfig.compare = true;
        }else {
            appConfig.filename = argv[i];
        }

    }
    return appConfig;
}

class Node{ 
    public:
        long long weight;
        Node* childs[2];

    Node(long long w){
        weight=w;
        childs[0] = NULL;
        childs[1] = NULL;
    }

    long long tree_weight();
};

long long Node::tree_weight() {
    if(this == NULL) return 0;

    long long rank = weight;
    rank += childs[0]->tree_weight();
    rank += childs[1]->tree_weight();
    return rank;
}

class Qdigest{
    public: 
        long long univ;  
        double eps; 
        long long total_weight; 
        long long capacity; 
        Node* root; 
    Qdigest(long long u, double e){
        univ = u;
        eps = e;
        capacity = 0;
        total_weight = 0;
        root = new Node(0);
    }

    void update(long long x, long long w);
    long long rank(long long x);
    void compress();
    long long quantile(double q, long long streamSize);
};

void Qdigest::update(long long x, long long w){
    total_weight += w; 
    capacity = eps*total_weight/(ceil(log2(univ))); 
    Node* v = root;
    long long l = 0, r = univ;
    while (w && (r-l)>1){
        long long avail = capacity - v->weight;
        long long added = min(w,avail);
        v->weight += added;
        // cout<<capacity<<endl;
        w = w - added;
        long long mid = (l+r)/2;
        
        int next_curr = 0;
        if(x<mid) {
            next_curr = 0;
            r = mid;
        } else {
            next_curr = 1;
            l = mid;
        }
        if(v->childs[next_curr]==NULL){
            v->childs[next_curr] = new Node(0);
        }
        v = v->childs[next_curr];
    }
    v->weight+=w;
}

long long Qdigest::rank(long long x){
    if(x < 0 || univ<=x){
        return 0;
    }
    Node* v = root;
    long long l = 0, r = univ;
    long long result = 0;
    while (v!=NULL && (r-l)>1){
        long long mid = (l+r)/2;
        if(x<mid){
            r = mid;
            v = v->childs[0];
        } else {
            result += v->childs[0] ? v->childs[0]->tree_weight() : 0;
            v = v->childs[1];
            l = mid;
        }
    }
    return result;
}

pair<Node*,long long> _compress(Node* crawl,  long long capacity, long long avail_up){
    // cout<<"oi"<<endl;
    cout<<crawl->weight<<endl;

    long long move_up = 0;
        
    for (int i = 0 ;i<2;i++){
        if(crawl->childs[i]!=NULL){
            Node* child = crawl->childs[i];
            long long avail_here = capacity - crawl->weight;
            
            pair<Node*, long long> result =_compress(child, capacity, avail_up + avail_here);
            Node* new_child = result.first; 
            long long move_up_from_child = result.second;

            crawl->childs[i] = new_child;
            long long put_here = min(avail_here, move_up_from_child);
            crawl->weight += put_here;
            move_up += (move_up_from_child - put_here);             
        }
    }

    long long move_up_from_here = min(avail_up, crawl->weight);
    move_up+=move_up_from_here;
    crawl->weight -= move_up_from_here;

    if(crawl->weight == 0 && crawl->childs[0] == NULL && crawl->childs[1] == NULL)
        return {NULL, move_up};
    else
        return {crawl, move_up};
}

void Qdigest::compress(){
    capacity = eps*total_weight/(ceil(log2(univ)));

    cout<<"Compressing with capacity: "<<capacity<<endl;
    _compress(root,capacity,0);
    cout<<endl;

    return;
}

long long Qdigest::quantile(double q, long long streamSize){
    long long target = round(streamSize * q);
    cout<<target<<endl;
    long long l = 0, r = univ;

    long long x = (r+l)/2;
    long long c_rank = 0;

    for(long long i = 0;i<=round(log2(univ));i++){
        if(target==c_rank){
            return x;
        }
        c_rank = rank(x);
        if(c_rank > target){
            r = x;
        } else {
            l = x;
        }
        x = ((float)(r+l)/2);
    }
    return x;
}

int cnt = 1;
void traverse(Node* root){
    if(root == NULL){
        cout<<cnt++<<": "<<"null"<<endl;
        return;
    }
    Node* crawl = root;
    cout<<cnt++<<": "<<crawl->weight<<endl;
    traverse(crawl->childs[0]);
    traverse(crawl->childs[1]);
}

Qdigest sketch(appConfig.universe_size, appConfig.eps);

vector<pair<long long,long long>> readCSV(string input_f){
    ifstream input(input_f);
    //read headers 
    readCsvLine(input);

    vector<pair<long long, long long>> data;
    while(input.peek() != EOF){        
        vector<string> line = readCsvLine(input);
        int value = stoll(line[appConfig.id_field_no]);

        if(value<=appConfig.universe_size){
            sketch.update(value,1);
            if(true)
                data.push_back({value,1});
        }
    }
    sort(data.begin(), data.end());
    return data;
}

void ExecuteQD(){
    random_device rd;
    mt19937 gen(rd());

    // long long univ = 256;
    // int max_w = 50;
    // double eps = 0.1;
    // int n = 1000;
    vector<pair<long long,long long>> stream = readCSV(appConfig.filename);
    
    // for (int i=0;i<n;i++){
    //     int x = uniform_int_distribution<long long>(1, univ)(gen);
    //     int w = uniform_int_distribution<long long>(1, max_w)(gen);
    //     stream.push_back({x,w});
    // }
    // stream.push_back({3,29});
    // stream.push_back({2, 9});
    // stream.push_back({2, 44});
    // stream.push_back({6, 200});
    // stream.push_back({1, 28});
    // stream.push_back({1, 48});
    
    vector<long long>true_ranks(appConfig.universe_size+1,0);
    long long total_weight = 0;

    for(auto x:stream){
        // cout<<x.first<<" "<<x.second<<endl;
        total_weight +=x.second;
        true_ranks[x.first] += x.second;
    }
    // cout<<endl;

    for(int i=1;i<=appConfig.universe_size;i++){
        // cout<<i<<" "<<true_ranks[i]<<endl;
        true_ranks[i] += true_ranks[i-1];
    }
    // cout<<endl;

    for(int x=1;x<=appConfig.universe_size;x++){
        cout<<"x = "<<x<<endl;
        cout<<"rank: "<<sketch.rank(x)<<" ";
        cout<<"true rank: "<<true_ranks[x-1]<<endl;
        cout<<"error: "<<true_ranks[x-1] - sketch.rank(x)<<" expected max error: "<<total_weight*appConfig.eps<<endl;
        cout<<"------"<<endl;
    }
    // cout<<sketch.quantile(0.5,total_weight)<<endl;

    // cout<<"estimated: "<<sketch.rank(4)<<endl;
    // cout<<"real rank: "<<true_ranks[3]<<endl;
    
    // traverse(sketch.root);
    // cout<<endl<<endl;
    // sketch.compress();
    // traverse(sketch.root);
    return; 
}

int main(int argc, char * argv[]){
    appConfig = parseArgs(argc, argv);
    sketch = Qdigest(appConfig.universe_size, appConfig.eps);
    // cout<<appConfig.id_field_no<<endl;
    // cout<<appConfig.eps<<endl;
    // cout<<appConfig.universe_size<<endl;
    // cout<<appConfig.filename<<endl;
    // cout<<appConfig.execution<<endl;
    // cout<<appConfig.query<<endl;
    ExecuteQD();
    return 0;
}