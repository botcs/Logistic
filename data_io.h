#ifndef DATA_IO_H_INCLUDED
#define DATA_IO_H_INCLUDED

#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <queue>
#include <string>
#include <sstream>
#include <list>
#include <queue>
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
    map<string, index> assoc;

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


        priority_queue<Container> PQ;

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
                auto From = assoc[start];
                auto To   = assoc[finish];
                if(From && To)
                    PQ.push(Container{ID, From-1, To-1, unsigned(time), size_t(amount)});
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

        while (!PQ.empty()){
            data.requests.push_back(PQ.top());
            PQ.top().print(cout);
            PQ.pop();
        }
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

                data.insert(start, finish, ID, capac, to, back, phase, assoc);
            }

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
