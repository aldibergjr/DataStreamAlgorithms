#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include<bits/stdc++.h> 
#define ll long long
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
    

    double t;

    void update(int size, net_flow e){
        //step 1
        int s = size;
        int sketch_size = (int)H.size() +(int) C.size();
        
        if(sketch_size < s){
            H.push_back(e);
            return;
        }

        double wl = (double) t * (int)C.size();
        vector<net_flow> newL;

        if(e.weight < t){
            newL.push_back(e);
            wl += e.weight;
        }else{
            H.push_back(e);
        }

        make_heap(H.begin(), H.end(), netComparator());
        sort_heap(H.begin(), H.end(),netComparator()); 


        //Step 2
       
         while(H.size() != 0 && wl >= (sketch_size - (int)H.size()) * H[0].weight){
            net_flow e; 
            e.id = H[0].id;
            e.weight = H[0].weight;
            wl = wl + (double)e.weight;
            newL.push_back(e);
            H.erase(H.begin()); 
        }

       
        int size_dif = (sketch_size - (int)H.size());
        t = (double)wl/size_dif;
        

        //Step 3
        double p = (double) rand() / (RAND_MAX + 1.);
        int j = 0;
        while(p >= 0 && j < (int)newL.size()){ 
            p = p - (1 - (double)newL[j].weight/t);
            
            j++;
        }
        
        if(p <= 0){
           
            newL.erase(newL.begin() + j-1);   
            
        }else{     
                   
            if(C.size() > 0)
                C.erase(C.begin() + (rand() % (int)C.size()));
            else{
                newL.erase(newL.begin() + (rand() % (int)newL.size()));
            }
        }
        for(int i = 0; i < (int)newL.size(); i++){
            C.push_back(newL[i]);
        }
        

    }

    
    ll query(string param){
        ll result = 0;
        for (std::vector<net_flow>::const_iterator i = H.begin(); i != H.end(); ++i){
            if(((net_flow)*i).param.compare(param) == 0)
                result += ((net_flow)*i).weight;
        }
        for (std::vector<net_flow>::const_iterator i = C.begin(); i != C.end(); ++i){
            if(((net_flow)*i).param.compare(param) == 0)
                result += t;
        }
        return result;
    }
   
};


//delete after


sketch sample;

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
        //if(data_line.weight > 0)
        sample.update(appConfig.size, data_line);
        // if(sample.H.size() > appConfig.size)
        //     cout << sample.t << endl;
        data.push_back(data_line);
        
            
    }
    return data;
}

int main(int argc, char * argv[]) 
{
    
    
    appConfig = parseArgs(argc, argv);
  
  
        srand ((unsigned)time(NULL));

        vector<net_flow> data = readCSV(appConfig.filename);
    
        ll est = sample.query(appConfig.filter_param);
       
        cout<< "est: " << est << endl;
      
    return 0;
}




