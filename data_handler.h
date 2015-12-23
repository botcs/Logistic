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
#define sep_width 70
#define separator setfill (fill) << setw (sep_width) << '\n'


#include "data_classes.h"



struct DataHandler{
    using c = shared_ptr<Container>;
    using e = shared_ptr<edge>;



    ///REQUESTS
    long unsigned total = 0;
    long unsigned processed = 0;
    list<c> pending;
    list<c> solved;
    list<c> unsolved;

    priority_queue<Operation> operations;

    size_t num_of_ships = 0;
    unordered_map<string, city > cities;

    int getClientPercent(){
        return 100 * (solved.size()+unsolved.size())/
                (solved.size()+unsolved.size()+pending.size());
    }

    double getLoadPercent(){
        return 100 * processed/total;
    }



    float getLoadProgress(){
        return processed/float(total);
    }

    inline void reservePath(shared_ptr<Container> client){
        client -> travelTime = 0;
        for(auto& e : client->travelPath){
                client->travelTime += e->getDist(client->travelTime);
                e->reserve(client->stack_size);
                operations.emplace(client, e);
        }
    }


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
    void printPending(ostream& o){
        o << separator
         <<"REQUESTS UNDER PROCESS\n"
         <<separator;
         if(!pending.empty()){
            for(auto us : pending){
                us->print(o);
                o << "\n";
            }
        } else {
            o << "\t\tEMPTY\n";
        }
    }

    void printUnsolved(ostream& o){
        o << separator
         <<"UNSOLVED pending\n"
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

    void printRequests(ostream& o){
        printPending(o);
        printSolutions(o);
        printUnsolved(o);
    }

    void print(ostream& o){
        printCities(o);
        printRequests(o);
        printOperations(o);
    }

    void printOperations(ostream& o){
         o << separator
         <<"OPERATIONS\n";
        auto copy = operations;
        while(!copy.empty()){
            copy.top().print(o);
            copy.pop();
        }
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

        ++num_of_ships;

    };

    inline city& operator[] (const string& cityIndex) {return cities[cityIndex];}


    ~DataHandler(){
    }

};

#endif // DATA_HANDLER_H_INCLUDED
