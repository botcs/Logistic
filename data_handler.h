#ifndef DATA_HANDLER_H_INCLUDED
#define DATA_HANDLER_H_INCLUDED

#include "data_classes.h"



struct DataHandler{
    using c = shared_ptr<Container>;
    using e = shared_ptr<edge>;

    list<c> requests;
    list<c> solved;

    unordered_map<string, city > cities; //Vertices with edges in list
    void print(ostream& o){
        o<<"********************\n"
         <<"CITIES\n";
        for(auto& vertex : cities){
            o<<"*********************************************\n"
             <<vertex.first<<"\n";
            vertex.second.print(o);
        }


        if(!solved.empty()){

            o<<"********************\n"
             <<"SOLVED REQUESTS\n"
             <<"********************\n";
            for(auto s : solved){
                s->print(o);
                o << "\n";
            }
        }
        if(!requests.empty()){
            o<<"********************\n"
             <<"UNSOLVED REQUESTS\n"
             <<"********************\n";
            for(auto us : requests){
                us->print(o);
                o << "\n";
            }
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
