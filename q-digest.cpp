#include <sys/resource.h>
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
    vector<double> queryValues;
    bool compare;
    bool help_exec;
    bool analysis;
};

CONFIGS appConfig;

CONFIGS parseArgs(int argc, char * argv[]){
    CONFIGS appConfig;
    appConfig.compare = false;
    appConfig.analysis = true;
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
            // appConfig.query = atof(argv[++i]);
        }else if(paramName.compare("-h") == 0 || paramName.compare("--help") == 0){
            appConfig.help_exec = true;
        // }else if(paramName.compare("--compare") == 0){
        //     appConfig.compare = true;
        }else if(appConfig.filename==""){
            appConfig.filename = argv[i];
        } else {
            appConfig.queryValues.push_back(atof(argv[i]));
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
    // cout<<crawl->weight<<endl;

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

    // cout<<"Compressing with capacity: "<<capacity<<endl;
    _compress(root,capacity,0);
    // cout<<endl;

    return;
}

long long Qdigest::quantile(double q, long long streamSize){
    long long target = round(streamSize * q);
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
            // sketch.compress();
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
    struct rusage usage;
    time_t start, end;

    time(&start);
    vector<pair<long long,long long>> stream = readCSV(appConfig.filename);
    
    vector<long long>true_ranks(appConfig.universe_size+1,0);
    long long total_weight = 0;

    for(auto x:stream){
        total_weight +=x.second;
        true_ranks[x.first] += x.second;
    }

    for(int i=1;i<=appConfig.universe_size;i++){
        true_ranks[i] += true_ranks[i-1];
    }

    long long real;
    long long result;
    if(appConfig.execution=="rank"){
        for(auto x:appConfig.queryValues){
            cout<<"Estimated Rank "<<x<<": ";
            result = sketch.rank(x);
            real = true_ranks[x-1];
            cout<<result<<" ";
            cout<<"Real Rank"<<x<<": "<<true_ranks[x-1]<<endl;
            cout<<"------"<<endl;
        }
    }else if(appConfig.execution=="quant"){
        for(auto x:appConfig.queryValues){
            cout<<"Estimated Rank for q: "<<x<<", ";
            cout<<"targert Rank: "<<(long long)round(stream.size() * x)<<endl;
            long long aux = sketch.quantile(x,stream.size()); 
            result = sketch.rank(aux);
            real = round(stream.size() * x);
            cout<<aux<<" "<<real<<endl;
            cout<<"------"<<endl;
        }
    }

    time(&end);
    getrusage(RUSAGE_SELF, &usage);
    double ttaken = double(end -start);
    long memory = usage.ru_maxrss;

    if(appConfig.analysis){
        stringstream execution_line; 
        ofstream of;
        of.open("./reports/report8.csv", ios_base::app);
        execution_line << appConfig.id_field_no << "," << appConfig.eps << "," << appConfig.universe_size  << "," <<appConfig.execution << "," <<appConfig.queryValues[0]<<","<<result << "," <<real << "," <<ttaken << "," <<memory;
        cout << execution_line.str() << endl;
        of << execution_line.str() << "\n";
        of.close();
    }
    return; 
}

int main(int argc, char * argv[]){
    appConfig = parseArgs(argc, argv);
    sketch = Qdigest(appConfig.universe_size, appConfig.eps);
    ExecuteQD();
    return 0;
}