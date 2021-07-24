#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
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

int main(int argc, char * argv[]) 
{
    ifstream myFile("./network_flows.csv");
    getNextLineAndSplitIntoTokens(myFile);
    vector<string> mcsv = getNextLineAndSplitIntoTokens(myFile);
    
    for (std::vector<string>::const_iterator i = mcsv.begin(); i != mcsv.end(); ++i)
        std::cout << *i << ' ';
    return 0;
}

