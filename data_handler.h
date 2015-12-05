#ifndef DATA_HANDLER_H_INCLUDED
#define DATA_HANDLER_H_INCLUDED

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

#include <iomanip>

#define fill '*'
#define width 70
#define separator setfill (fill) << setw (width) << '\n'


#include "data_classes.h"



struct DataHandler{
    using c = shared_ptr<Container>;
    using e = shared_ptr<edge>;

    list<c> requests;
    list<c> solved;
    list<c> unsolved;

    unordered_map<string, city > cities; //Vertices with edges in list

    void printCities(ostream& o){
        o << separator
         <<"CITIES\n";
        for(auto& vertex : cities){
            o << separator
             <<vertex.first<<"\n";
            vertex.second.print(o);
        }
    }
    void printSolutions(ostream& o){
        o << separator
         <<"SOLVED REQUESTS\n"
         <<separator;
        if(!solved.empty()){

            for(auto s : solved){
                s->print(o);
                o << "\n";
            }
        } else {
            o << "\t\tEMPTY\n";
        }
    }
    void printRequests(ostream& o){
        o << separator
         <<"REQUESTS UNDER PROCESS\n"
         <<separator;
         if(!requests.empty()){
            for(auto us : requests){
                us->print(o);
                o << "\n";
            }
        } else {
            o << "\t\tEMPTY\n";
        }
    }

    void printUnsolved(ostream& o){
        o << separator
         <<"UNSOLVED REQUESTS\n"
         <<separator;
        if(!unsolved.empty()){

            for(auto s : unsolved){
                s->print(o);
                o << "\n";
            }
        } else {
            o << "\t\tEMPTY\n";
        }
    }

    void printClients(ostream& o){
        printRequests(o);
        printSolutions(o);
        printUnsolved(o);
    }

    void print(ostream& o){
        printCities(o);
        printClients(o);
    }

    void insert(const string& From, const string& To,
                const string& ID, const size_t& capac,
                const unsigned& length, const unsigned& back_length,
                const unsigned& phase){



        shared_ptr<edge> E = make_shared<edge>(To, ID, capac, length, phase);
        shared_ptr<edge> backE = make_shared<edge>(From, ID, capac, back_length, phase+length);

        ///LINKING FOR FAST DISTANCE CALCULATION
        E->back=backE;
        backE->back=E;

        cities[From][To].push_back(E);
        cities[To][From].push_back(backE);


    };

    city& operator[] (const string& cityIndex) {return cities[cityIndex];}

    ~DataHandler(){
    }

};

#endif // DATA_HANDLER_H_INCLUDED
