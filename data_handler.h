#ifndef DATA_HANDLER_H_INCLUDED
#define DATA_HANDLER_H_INCLUDED

#include "data_classes.h"

using c = shared_ptr<Container>;

struct DataHandler{

    list<c> requests;
    queue<c> solved;

    unordered_map< string, city > cities; //Vertices with edges in list
    void printData(ostream& o){
        o<<"********************\n"
         <<"CITIES\n";
        for(auto& vertex : cities){

            o<<"************************************************\n";
            o<<vertex.first;
            vertex.second.print(o);
        }

        o<<"********************\n"
         <<"SOLVED CONTAINERS\n"
         <<"********************\n";
        auto copy = solved;
        while(!copy.empty()){
            copy.front()->print(o);
            copy.pop();
        }
    }

    void insert(const string& From, const string& To,
                const string& ID, const size_t& capac,
                const unsigned& length, const unsigned& back_length,
                const unsigned& phase){



        shared_ptr<edge> E = make_shared<edge>(ID, capac, length, phase);
        shared_ptr<edge> backE = make_shared<edge>(ID, capac, back_length, phase+length);

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
