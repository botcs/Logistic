#ifndef DATA_IO_H_INCLUDED
#define DATA_IO_H_INCLUDED

#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <sstream>
#include <list>
#include <queue>
#include <algorithm>
#include <stdexcept>
#include <stack>
#include <set>

#include "data_handler.h"
using namespace std;

string conv (double i)
{
    stringstream ss;
    string s;
    ss<<i;
    ss>>s;
    return s;
}

double conv (string i)
{
    stringstream ss;
    double s;
    ss<<i;
    ss>>s;
    return s;
}

class dataReaderWriter
{
public:

    void loadCont(const char* file_name, DataHandler& data){
        ifstream infile (file_name);
        if(infile.bad()){
            string exc = "wrong filename: ";
            exc+= file_name;
            exc+= "\n";
            throw logic_error(exc);
        }

        string  ID, start, finish;
        int  amount, time;

        unsigned line_count = 0;


        vector<shared_ptr<Container> > load;

        size_t TotalAmount = 0;
        while(infile.good())
        {
            line_count++;
            infile>>ws;
            char commentSymbol = infile.peek();
            getline(infile,ID);
            if(commentSymbol == '#' || commentSymbol == -1) continue;

            stringstream ss (ID);
            //ONLY VALID LINES ARE ACCEPTED
            if(ss >>  ID >> amount >> start >> finish >> time){


                if(time < 0 || amount < 1){
                    string s("\n\nERROR: Invalid parameter in line: ");
                    s+=conv(line_count);
                    s+="\nIN FILE: ";
                    s+=file_name;
                    s+="\nThe bonus time given is negative, or the amount is less then 1";
                    s+="\n*********************\n";
                    throw logic_error(s);
                }
                if(data.cities.count(start) && data.cities.count(finish))
                    load.push_back(make_shared<Container>(ID, start, finish, unsigned(time), size_t(amount)));
                else {
                    string s("\n\nERROR: Invalid parameter in line: ");
                    s+=conv(line_count);
                    s+="\nIN FILE: ";
                    s+=file_name;
                    s+="\nContainer Starting or End point not existing, or are the same";
                    s+="\n*********************\n";
                    throw logic_error(s);
                }

                TotalAmount+=amount;
            }


        }

        if(TotalAmount < 1) {
            string s("\n\nERROR: end of file reached: ");
            s+=file_name;
            s+="\nWith line count: ";
            s+=conv(line_count);
            s+="\nBut no usable data was found";
            s+="\n*********************\n";
            throw logic_error(s);
        }

        auto comp = [](const shared_ptr<Container> lhs, const shared_ptr<Container> rhs){
            return (lhs->bonusTime < rhs->bonusTime ||
                    (lhs->bonusTime == rhs->bonusTime && lhs->From < rhs->From) );
        };
        sort(load.begin(), load.end(), comp);
        for (auto& p : load) data.requests.push_back(p);


    }
    void loadMap(const char* file_name, DataHandler& data){
        fstream infile (file_name);
        string  ID, start, finish;
        int  capac, to, back, phase;

        if(infile.bad()){
            string exc = "wrong filename: ";
            exc+= file_name;
            exc+= "\n";
            throw logic_error(exc);
        }

        size_t line_count = 0;
        while(infile.good())
        {
            line_count++;
            infile>>ws;
            char commentSymbol = infile.peek();
            getline(infile,ID);

            if(commentSymbol == '#' || commentSymbol == -1) continue;

            stringstream ss (ID);
            //ONLY VALID LINES ARE ACCEPTED
            if(ss >>  ID >> capac >> start >> finish >> to >> back >> phase){
                if(capac < 1 || to < 1 || back < 1){
                    string s("\n\nERROR: Invalid parameter in line: ");
                    s+=conv(line_count);
                    s+="\nIN FILE: ";
                    s+=file_name;
                    s+="\n*********************\n";
                    throw logic_error(s);
                }

                data.insert(start, finish, ID, capac, to, back, phase);
            }

        }
        if(data.cities.empty()) {
            string s("\n\nERROR: end of file reached: ");
            s+=file_name;
            s+="\nWith line count: ";
            s+=conv(line_count);
            s+="\nBut no usable data was found";
            s+="\n*********************\n";
            throw logic_error(s);
        }

    }

    dataReaderWriter(const char* ship_file, const char* cargo_file, DataHandler& data)
    {
        loadMap(ship_file, data);
        loadCont(cargo_file, data);
    }

    void test(){

    }


};

#endif // DATA_IO_H_INCLUDED
