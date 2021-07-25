#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include<bits/stdc++.h> 

using namespace std;




struct net_flow{
    string id;
    int weight;
    string param;
} ;

struct netComparator{
    bool operator()(const net_flow& a,const net_flow& b) const{
        return a.weight<b.weight;
    }
};

struct CONFIGS {
    int id_col;
    int weight_col;
    int search_col;
    string filter_param;
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
        if(paramName.compare("--id") == 0){
            appConfig.id_col = stoi(argv[++i]);
        }else if(paramName.compare("--weight") == 0){
            appConfig.weight_col = stoi(argv[++i]);
        }else if(paramName.compare("--size") == 0){
            appConfig.size = stoi(argv[++i]);
        }else if(paramName.compare("--filter") == 0){
            appConfig.search_col = stoi(argv[++i]);
            appConfig.filter_param = argv[++i];
        }else {
            appConfig.filename = argv[i];
        }

    }
    return appConfig;
}

struct sketch{
    vector<net_flow> H;
    vector<net_flow> C;
    float t;
};

sketch sample;

sketch update (sketch sample, net_flow e){
    int s = appConfig.size;
    int sketch_size = sample.H.size() + sample.C.size();
    vector<net_flow> h = sample.H;
    vector<net_flow> c = sample.C;

    if(sketch_size < s){
        h.push_back(e);
        
        sample.H = h;
        return sample;
    }

  

    float wl = sample.t * c.size();
    vector<net_flow> newL;
    if(e.weight < sample.t){
        newL.push_back(e);
        wl += e.weight;
    }else{
        h.push_back(e);
    }

    make_heap(h.begin(), h.end(), netComparator());
    sort_heap(h.begin(), h.end(),netComparator());

    //extract funtion prob
    cout<< h[0].weight <<endl;
   
    
    while(h.size() != 0 && wl >= (s-h.size()) * h[0].weight){
        net_flow min_e ;
        min_e.id = h[0].id;
        min_e.weight = h[0].weight;
        h.erase(h.begin());
        newL.push_back(min_e);
        wl += min_e.weight;
        
    }

    sample.t = (float)wl/(s-h.size());
    /*
    //extract func
    float p_sum = 0;
    for(int i = 0; i < c.size(); i++){
        p_sum+= 1 - c[i].weight/sample.t;
    }
    for(int i = 0; i < newL.size(); i++){
        p_sum+= 1 - newL[i].weight/sample.t;
    }
    float p = (float) rand()/RAND_MAX;
    int j = 0;
    if(c.size() > 0){
         while(p >= 0 && j < newL.size()){
            p -= 1 - newL[j].weight/sample.t;
            j++;
        }
        if(p <= 0){
            newL.erase(newL.begin() + j-1);
        }else{
            c.erase(newL.begin() + (rand() % c.size()));
        }
        
    }

   for(int i = 0; i < newL.size(); i++){
            c.push_back(newL[i]);
    }
    */
    
    sample.H = h;
    sample.C = c;
    return sample;
}

vector<net_flow> readCSV( string input_f){
    ifstream input(input_f);
    //read headers 
    
    readCsvLine(input);



    vector<net_flow> data;
    while(input.peek() != EOF){
        net_flow data_line;
        vector<string> line = readCsvLine(input);
        
       
        data_line.id = line[appConfig.id_col];
        data_line.param = line[appConfig.search_col];
        data_line.weight = stoi(line[appConfig.weight_col]);
        sample = update(sample, data_line);
        // if(sample.H.size() > appConfig.size)
        //     cout << sample.t << endl;
        // data.push_back(data_line);
        
            
    }
    return data;
}

int main(int argc, char * argv[]) 
{
   srand(time(0));
    
    //read headers
    

    
    appConfig = parseArgs(argc, argv);
    vector<net_flow> data = readCSV(appConfig.filename);

     
    
    

     for (std::vector<net_flow>::const_iterator i = sample.H.begin(); i != sample.H.end(); ++i)
            std::cout << ((net_flow)*i).id << ' ' << ((net_flow)*i).weight <<  '\n';


    
    return 0;
}




