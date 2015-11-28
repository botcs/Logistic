#ifndef LIFEHACK_H_INCLUDED
#define LIFEHACK_H_INCLUDED


#include "data_io.h"


class InstanceHandler
{

    DataHandler DATA;
    struct node
    {
        unsigned dist;
        bool validated = false;
        index prev;
        edge* incoming;
        index cityInd;


        bool operator < (const node& rhs) const {return dist>rhs.dist;}

        node(index this_city, const unsigned& distance=-1, edge* in = 0, index pr = -1) :
            dist(distance), prev(pr), incoming(in), cityInd(this_city){};

        node(): prev(0), incoming(0), cityInd(0){};
    };


    index lastSource = -1;
    vector<node> cities;
    priority_queue<Operation> operations;
    priority_queue<node> openSet;

public:

    void setInstance(){
        cities.clear();

        cities.reserve(DATA.cities.size());

        for(size_t i = 0; i < DATA.cities.size(); i++ ){
            cities.emplace_back(i);
        }
    }

    InstanceHandler(const char* ship_file, const char* cargo_file, const char* command_file){
        dataReaderWriter(ship_file, cargo_file, DATA);
        cout<<"File loading finished \n";

        setOperations(DATA.requests.top());


        //print(cout);

    }

    void print(ostream& o){

        cout<<"Operations \n";
        auto copy = operations;
        while(!copy.empty()){
            copy.top().print(o);
            copy.pop();
        }


        DATA.printData(o);
    }


    void setOperations(const Container& client){
        if(client.From != lastSource){
            setInstance();
            lastSource = client.From;
            cities[client.From].dist = 0;
        }

        //Validity  =  knows the shortest path to itself
        if(!cities[client.To].validated){
            if(!findRoute(client.From, client.To))
            {
                string s;
                s+="\nPATH not found for \n";
                s+=client.print();
                s+="\n*********************\n";
                throw logic_error(s);
            }
        }

        reserveRoute(client);
    }


    void reserveRoute(const Container& client){

        size_t max_load = -1;

        const index& start = client.From;
        const index& goal = client.To;

        //GET MAX CAPACITY ON CURRENT SOLUTION
        index  last_invalid = cities.size();
        index curr = goal;
        while(curr != start)
        {
            auto currMax = cities[curr].incoming->getFreeSize();
            if(currMax<=max_load){
                max_load = currMax;
                last_invalid = curr;
            }
            curr = cities[curr].prev;
        }

        //RESERVE SHIPS
        curr = goal;

        Container solved = client;
        solved.stack_size = max_load;

        Container remainder = client;
        remainder.stack_size = client.stack_size-max_load;


        while(curr != start){


            auto& currCity = cities[curr];
            operations.emplace(solved, currCity.incoming, cities[curr].dist);
            currCity.incoming->addContainer(max_load);
            curr = cities[curr].prev;
        }


        //IF ONE OF THE ROUTES GOT FULLY LOADED
        //SO RESTORE THE NODES TO GIVE SHORTEST ROUTE

        if(last_invalid != cities.size()){
            curr = goal;
            while(curr != cities[last_invalid].prev)
            {
                cities[curr].validated = false;
                cities[curr].dist = -1;
                curr = cities[curr].prev;
            }
            //NOW THE CURR IS THE LATEST VALID START THE SEARCH FROM HERE

            findRoute(curr, goal);
        }

        if(remainder.stack_size && max_load < client.stack_size){
            reserveRoute(remainder);
        }

    }


    bool findRoute(const index& From, const index& goal){


        for(auto& c : cities){

                cout<<DATA[c.cityInd].name<<"\t"<<c.dist<<'\t';
                if(c.incoming) cout<<c.incoming->ID;
                cout<<"\n";
        }




        cities[From].validated = false;
        openSet.push(cities[From]);
        node curr = cities[From];

        while(!openSet.empty() && curr.cityInd != goal){

            curr = openSet.top();
            openSet.pop();

            if(cities[curr.cityInd].validated) continue;

            curr.validated = true;
            cities[curr.cityInd] = curr;

            const auto& neighbours = DATA[curr.cityInd].getShortestEdges(curr.cityInd, curr.dist);
            for(auto n : neighbours)
            {
                node To = cities[n.second->To];


                if(!To.validated && To.dist > n.first+curr.dist){

                    To.dist     = n.first + curr.dist;
                    To.incoming = n.second;
                    To.prev     = curr.cityInd;

                    openSet.push(To);
                }
            }

            auto copy = openSet;
            cout<<"TEST "<<DATA[curr.cityInd].name<<" TEST\n\n";
            while(!copy.empty()){
                auto test = copy.top();
                cout<<DATA[test.cityInd].name<<"\t"<<test.dist<<'\t';
                if(test.incoming) cout<<test.incoming->ID;
                cout<<"\n";
                copy.pop();
            }
            cout<<"*****************************\n\n\n";

        }






        return curr.cityInd==goal;


    }
};

#endif // LIFEHACK_H_INCLUDED
