#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include<bits/stdc++.h> 
#include <math.h>   
#include <random>

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
    double error_bound;
    double error_probability;
    int size;
    string filename;
};

CONFIGS appConfig;
random_device rd;
mt19937 gen(rd());

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
            appConfig.error_bound = stod(argv[++i]);
        }else if(paramName.compare("--delta") == 0){
            appConfig.error_probability = stod(argv[++i]);
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

long long getMedian(vector<long long> values){
    size_t size = values.size();

    if (size == 0){
        return 0; 
    } else {
        sort(values.begin(), values.end());
        if (size % 2 == 0) {
            return (values[size / 2 - 1] + values[size / 2]) / 2;
        } else {
            return values[size / 2];
        }
  }
}

long long R = (1UL <<61) -1;

class KMV{
    public:
        long long k;
        priority_queue<long long> min_vals;
        unordered_set<long long> Kvals;
        long long a = uniform_int_distribution<long long>(1, R)(gen);
        long long b = uniform_int_distribution<long long>(1, R)(gen);


    KMV(long long init_k, long long m){
        k = init_k;
    }

    long long hash(long long x);
    void update(long long val);
    long long query();
};

long long KMV::hash(long long x) {
    return (((a*x) % 18446744073709551615ULL) + b) % R;
}

void KMV::update(long long val) {
    long long h = hash(val);

    if(Kvals.count(h)) return;
    if(min_vals.size() < k || min_vals.top() > h) {
        Kvals.insert(h);
        min_vals.push(h);
    }
    if(min_vals.size() > k) {
        Kvals.erase(min_vals.top());
        min_vals.pop();
    }

}

long long KMV::query(){
    if (min_vals.size() < k)
        return min_vals.size();
    else 
        return R * ((double)k/(double)min_vals.top());
}

int main(int argc, char * argv[]) 
{
    srand(time(0));
    
    // read headers    
    appConfig = parseArgs(argc, argv);
    vector<net_flow> data = readCSV(appConfig.filename);
    set<long long> real;

    clock_t tStart = clock();
    hash<string> hasher;

    long long m = data.size();

    long long k = ((5)/(0.25 * appConfig.error_bound * appConfig.error_bound));
    long long S = 4 * log(1/appConfig.error_probability);

    cout<<"k size: "<<k<<endl;
    cout<<"m size: "<<m <<endl;
    cout<<"S size: "<<S<<endl;
    vector<long long> estimatedValues;

    while (S) {
        KMV sketch(k, m);

        for(auto d:data){
            auto hashed = hasher(d.value);
            real.insert(hashed);
            sketch.update(hashed);
        }

        estimatedValues.push_back(sketch.query());
        S--;
    }

    cout<<"h0: "<<getMedian(estimatedValues)<<" real size: "<<real.size()<<endl;
    cout<<"duration: "<<(clock() - tStart)/CLOCKS_PER_SEC<<endl<<endl;

         
    return 0;
}




