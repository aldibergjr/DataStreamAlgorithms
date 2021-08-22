#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <sys/time.h>
#include <sys/resource.h>
#include <fstream>
#include<bits/stdc++.h> 
#define ll long long
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
    float eps;
    float delta;
    float mult;
    int universe_size;
    string execution;
    float query;
    bool compare;
    string filename;
    int weight_col;
    bool help_exec;
    bool analysis;
};

CONFIGS appConfig;

struct sketch{
    vector<vector<int>> B; // Buffer
    int K; // max capacity
    float c; //const
    int h; // max level
    int stream_size;
    int universe;

    void init(){
        h = 0;
        vector<int> aux;
        B.clear();
        B.push_back(aux);
        c = appConfig.mult;
        float eps = appConfig.eps;
        float delta = appConfig.delta;
        double power = (pow(c, 2) * (2 * c -1));
        double factor1 = sqrt(1/power * log10(2./delta));
        K = round(1./eps * factor1 );
        stream_size = 0;
        universe = 0;
    }
    void update(int x){
        stream_size ++;
        if(x > universe)
            universe = x;
        B[0].insert(upper_bound(B[0].begin(), B[0].end(), x), x);       
        compress();
    }
    void compress(){
        for(int l = 0; l <= h; l++){
            int capacity = round(K * pow(c, h-l));

            if( B[l].size() >=  max(2 , capacity)){
                
                double coin = (double) rand() / (RAND_MAX + 1.);
              
                vector<int> aux;

                if(coin < 0.5){
                    
                    for(int i = 0; i< B[l].size(); i+=2){
                        aux.push_back(B[l][i]);
                    }
                }else{
                    
                    for(int i = 1; i< B[l].size(); i+=2){
                        aux.push_back(B[l][i]);
                    }
                }
                B[l].clear();

                //Take up
                if(l+1 > h){
                    B.push_back(aux);
                    h++;
                }else{
                    for(int i = 0; i < aux.size(); i ++)
                        B[l+1].insert(upper_bound(B[l+1].begin(), B[l+1].end(), aux[i]), aux[i]); 
                }
               

            }else
                break;
        }
        
    }
    int rank(int x){
        int w = 1;
        int rank = 0;

        for (int i = 0; i < B.size(); i++)
        {
            int count = 0;
            for (int j = 0; j < B[i].size(); j++)
            {
                
                if(B[i][j] < x){
                    count ++;
                }
            }
            rank += count * w;
            w = w * 2;
            
        }
        return rank;
        
    }
    int quantile(float q){
        int target = round(stream_size * q);
        int u_bound = universe;
        int l_bound = 0;
        int x = (u_bound + l_bound)/2;
        int c_rank = 0;
        for(int i = 0; i <= round(log2(universe)); i++){
            if(target == c_rank){
                 return x;
            }
            c_rank = rank(x);
            if(c_rank > target){
                u_bound = x;
            }else{
                l_bound = x;
            }
            x = ((float)(u_bound + l_bound)/2.);
        }
        return x;

    }
    void print_buffer(){
        for (int i = B.size() -1; i >=0;  i--)
        {
            for (int j = 0; j < B[i].size(); j++)
            {
                cout << B[i][j] << " " ;
            }
            cout << endl;
        }
        
    }
};

sketch sample;

CONFIGS parseArgs(int argc, char * argv[]){
    CONFIGS appConfig;
    appConfig.compare = false;
    appConfig.help_exec = false;
    appConfig.analysis = false;
    for(int i = 1; i < argc; i++){
        string paramName = argv[i];
        if(paramName.compare("--val") == 0){
            appConfig.weight_col = stoi(argv[++i]);
        }else if(paramName.compare("--eps") == 0){
            appConfig.eps = atof(argv[++i]);
        }else if(paramName.compare("--delta") == 0){
            appConfig.delta = atof(argv[++i]);
        }else if(paramName.compare("--mult") == 0){
            appConfig.mult = atof(argv[++i]);
        }else if(paramName.compare("rank") == 0 || paramName.compare("quant") == 0){
            appConfig.execution = paramName;
            appConfig.query = atof(argv[++i]);
        }else if(paramName.compare("-h") == 0 || paramName.compare("--help") == 0){
            appConfig.help_exec = true;
        }else if(paramName.compare("--compare") == 0){
            appConfig.compare = true;
        }else {
            appConfig.filename = argv[i];
        }

    }
    return appConfig;
}


vector<int> readCSV(string input_f){
    ifstream input(input_f);
    //read headers 
    readCsvLine(input);



    vector<int> data;
    while(input.peek() != EOF){
        //net_flow data_line;
        
        vector<string> line = readCsvLine(input);
        int weight = stoi(line[appConfig.weight_col]);
        //if(data_line.weight > 0)
        sample.update(weight);
        // if(sample.H.size() > appConfig.size)
        //     cout << sample.t << endl;
        if(appConfig.compare)
            data.push_back(weight);

    }
    sort(data.begin(), data.end());
    return data;
}



int GenerateCompare(vector<int> data_set){
    if(appConfig.execution == "rank" ){
        int rank = 0;
        for(int i = 0; i< data_set.size(); i++){
            if(data_set[i] < (appConfig.query)){
                rank++;
            }else{
                break;
            }
        }
        cout<< "real : " << rank << endl;
        return rank;
    }else{
        int real = sample.quantile((appConfig.query));
        cout<< "real : " << data_set[round(data_set.size() * appConfig.query)] << endl;
        return real;
    }
    
}


void executeKLL(){
    struct rusage usage;
    srand ((unsigned)time(NULL));
    time_t start, end;
    
    cout << appConfig.compare;
    sample.init();
    time(&start);
    vector<int> data_set = readCSV(appConfig.filename);
    //data_set.size()
    int real = 0;
    if(appConfig.compare){
        real = GenerateCompare(data_set);
    }
    int result = 0;
    if(appConfig.execution == "rank" ){
        result = sample.rank((appConfig.query));
        cout <<"est: " << result << endl;
    } else{
        result = sample.quantile((appConfig.query));
        cout << "est :" << result << endl;
    }
    time(&end);
    getrusage(RUSAGE_SELF, &usage);
    double ttaken = double(end -start);
    long memory = usage.ru_maxrss;

    if(appConfig.analysis){
        stringstream execution_line ; 
        ofstream of;
        of.open("./reports/report1.csv", ios_base::app);
        execution_line << appConfig.weight_col << "," << appConfig.delta << "," << appConfig.eps << "," <<appConfig.mult << "," <<appConfig.execution << "," <<appConfig.query<<","<<result << "," <<real << "," <<ttaken << "," <<memory;
        cout << execution_line.str() << endl;
        of << execution_line.str() << "\n";
        of.close();
    }
    
}

int main(int argc, char * argv[]) 
{
    appConfig = parseArgs(argc, argv);
    executeKLL();
   
    
    
}