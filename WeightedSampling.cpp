#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>

using namespace std;



struct CONFIGS {
    int id_col;
    int weight_col;
    int search_col;
    string filter_param;
    int size;
    string filename;
};

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

int main(int argc, char * argv[]) 
{
    ifstream myFile("./network_flows.csv");
    readCsvLine(myFile);
    vector<string> mcsv = readCsvLine(myFile);
    CONFIGS appConfig = parseArgs(argc, argv);
    // for (std::vector<string>::const_iterator i = mcsv.begin(); i != mcsv.end(); ++i)
    //     std::cout << *i << ' ';
    cout << appConfig.filename << ' ' << appConfig.filter_param << ' ' << appConfig.id_col << ' ' << appConfig.search_col << ' ' << appConfig.size << ' ' << appConfig.weight_col << '\n';
    
    return 0;
}




