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

    inline void reservePath(shared_ptr<Container> client){
        client -> travelTime = 0;
        for(auto& e : client->travelPath){
                client->travelTime += e->getDist(client->travelTime);
                e->reserve(client->stackSize);
                operations.emplace(client, e);
        }
    }

    ///REQUESTS
    long unsigned total = 0;
    long unsigned processed = 0;
    list<c> pending;
    list<c> solved;
    list<c> unsolved;

    priority_queue<Operation> operations;

    size_t num_of_ships = 0;
    unordered_map<string, city > cities;



    ///STATUS QUERIES
    ///IMPLEMENTATION IN "data_io.h"
    float getClientPercent();

    float getLoadPercent();



    float getLoadProgress();




    void printCities(ostream& o);

    void printSolutions(ostream& o);

    void printPending(ostream& o);

    void printUnsolved(ostream& o);

    void printRequests(ostream& o);

    void print(ostream& o);

    void printOperations(ostream& o);


    inline city& operator[] (const string& cityIndex) {return cities[cityIndex];}


    ~DataHandler(){
    }

};

#endif // DATA_HANDLER_H_INCLUDED
