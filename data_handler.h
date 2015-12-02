#ifndef DATA_HANDLER_H_INCLUDED
#define DATA_HANDLER_H_INCLUDED

#include "data_classes.h"



struct DataHandler{

    list<Container*> requests;
    list<Container*> solved;

    vector< city > cities; //Vertices with edges in list
    void print(ostream& o){
        o<<"********************\n"
         <<"CITIES\n";
        for(auto& vertex : cities)
            vertex.print(o);

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
                const unsigned& phase, map<string, index>& assoc){


        index& indFrom = assoc[From];
        index& indTo   = assoc[To];

        edge* E = new edge{ID, capac, length, phase};
        edge* backE = new edge{ID, capac, back_length, phase+length};


        ///LINKING FOR FAST DISTANCE CALCULATION
        E->back=backE;
        backE->back=E;

        if (indFrom && indTo){

            cities[indFrom-1][indTo-1].push_back(E);
            cities[indTo-1][indFrom-1].push_back(backE);
        } else {
            //SOME OF THE CITIES ARE NOT FOUND
            //ONLY WORKS WHEN RETURN EDGES ARE GUARANTEED

            if(!indFrom && !indTo) {
                indFrom=cities.size()+1;
                indTo  =cities.size()+2;

                cities.emplace_back(indTo-1, E, From);
                cities.emplace_back(indFrom-1, backE, To);

            } else

            if(!indFrom && indTo) {
                indFrom=cities.size() + 1;
                cities.emplace_back(indTo-1, E, From);
                cities[indTo-1][indFrom-1].push_back(backE);
            } else

            if(!indTo && indFrom){
                indTo = cities.size() + 1;
                cities[indFrom-1][indTo-1].push_back(E);
                cities.emplace_back(indFrom-1, backE, To);
            }
        }

        E->To = indTo-1;
        backE->To = indFrom-1;
    };

    city& operator[] (const index& cityIndex) {return cities[cityIndex];}

    ~DataHandler(){
    }


};

#endif // DATA_HANDLER_H_INCLUDED
