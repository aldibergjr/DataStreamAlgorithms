#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include<bits/stdc++.h> 

using namespace std;

struct net_flow{
    string value;
    int weight;
    string param;
} ;

struct netComparator{
    bool operator()(const net_flow& a,const net_flow& b) const{
        return a.weight<b.weight;
    }
};

struct CONFIGS {
    int field_no;
    int error_bound;
    int error_probability;
    int size;
    string filename;
};

CONFIGS appConfig;

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

CONFIGS parseArgs(int argc, char * argv[]){
    CONFIGS appConfig;
    for(int i = 1; i < argc; i++){
        string paramName = argv[i];
        if(paramName.compare("--target") == 0){
            appConfig.field_no = stoi(argv[++i]);
        }else if(paramName.compare("--eps") == 0){
            appConfig.error_bound = stoi(argv[++i]);
        }else if(paramName.compare("--delta") == 0){
            appConfig.error_probability = stoi(argv[++i]);
        } else {
            appConfig.filename = argv[i];
        }
    }
    return appConfig;
}

vector<net_flow> readCSV( string input_f){
    ifstream input(input_f);
    //read headers 
    readCsvLine(input);

    vector<net_flow> data;
    while(input.peek() != EOF){
        net_flow data_line;
        vector<string> line = readCsvLine(input);
       
        data_line.value = line[appConfig.field_no];

        data.push_back(data_line);
    }
    return data;
}

class KMV{
    public:
        long long k;
        long long R;
        vector<long long> min_vals;
        long long a = (1UL <<61) -1;


    KMV(long long init_k, long long m){
        k = init_k;
        R = m*m*m;
    }

    long long hash(long long x);
    void update(long long val);
    long long query();
};

long long KMV::hash(long long x) {
    return ((a*x) % 18446744073709551615ULL) % R;
}

void KMV::update(long long val) {
    long long h = hash(val);

    if (find(min_vals.begin(), min_vals.end(), val) == min_vals.end()) {
        min_vals.push_back(h);
        sort(min_vals.begin(), min_vals.end());
        if (min_vals.size() > k) {
            min_vals.pop_back();
        }
    }

}

long long KMV::query(){
    if (min_vals.size() < k)
        return min_vals.size();
    else 
        return (R * k)/ min_vals[k-1];
}

int main(int argc, char * argv[]) 
{
    srand(time(0));
    
    //read headers    
    appConfig = parseArgs(argc, argv);
    vector<net_flow> data = readCSV(appConfig.filename);

    hash<string> hasher;

    long long k = 40;
    KMV sketch(k, data.size());

    for(int i=0;i<10;i++){
        auto hashed = hasher(data[i].value);
        sketch.update(hashed);
    }
    cout<<sketch.query()<<endl;
         
    return 0;
}




