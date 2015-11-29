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
        index parent;
        list<index> children;

        edge* incoming;
        index cityInd;


        bool operator < (const node& rhs) const {return dist>rhs.dist;}

        node(index this_city, const unsigned& distance=-1, edge* in = 0, index pr = -1) :
            dist(distance), parent(pr), incoming(in), cityInd(this_city){};

        node(): parent(0), incoming(0), cityInd(0){};
    };


    index lastSource = -1;
    vector<node> cities;
    priority_queue<Operation> operations;

    vector<index> Fringe;

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

        ofstream commands(command_file);
        while(!DATA.requests.empty()){
            setOperations(DATA.requests.front());
        }

        print(cout);

    }

    void print(ostream& o){

        o<<"Operations \n";
        auto copy = operations;
        while(!copy.empty()){
            copy.top().print(o);
            copy.pop();
        }

        o<<"\nRecords: "
        DATA.printData(o);
    }


    void setOperations( Container& client){
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


    void reserveRoute(Container client){
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
            curr = cities[curr].parent;
        }

        //RESERVE SHIPS
        curr = goal;

        Container solved = client;
        solved.stack_size = max_load;

        Container remainder = client;
        remainder.stack_size = client.stack_size-max_load;

        auto& length = cities[goal].dist;
        if(client.solvable && client.Time < length){
            client.solvable=false;
            DATA.requests.push_back(client);
            DATA.requests.pop_front();
            return;
        }



        while(curr != start){

            auto& currCity = cities[curr];
            operations.emplace(solved, currCity.incoming, cities[curr].dist);
            currCity.incoming->addContainer(max_load);
            curr = cities[curr].parent;
        }

        //IF ONE OF THE ROUTES GOT FULLY LOADED
        //SO RESTORE THE NODES TO GIVE SHORTEST ROUTE
        if(last_invalid != cities.size()){

            invalidateFromSource(last_invalid);

            //NOW THE CURR IS THE LATEST VALID START THE SEARCH FROM HERE
            addLeavesToFringe(curr);
            findRoute(curr, goal);


        }

        if(remainder.stack_size && max_load < client.stack_size){
            reserveRoute(remainder);
        } else {DATA.requests.pop_front();}

    }

    bool addLeavesToFringe(const index rootInd){
        auto& root = cities[rootInd];
        if(root.children.empty()){
            if(root.validated ){
                root.validated = false;
                Fringe.push_back(root.cityInd);
                return true;
            }
        } else {
            bool hasValidLeaf = false;
            for(auto& child : root.children)
                hasValidLeaf = addLeavesToFringe(child);

            if(!hasValidLeaf){
                root.validated = false;
                Fringe.push_back(root.cityInd);
                return true;
            }
        }
        return false;
    }

    void invalidateFromSource(const index source){

        auto& children = cities[source].children;
        while(!children.empty()){
            auto& child = children.front();
            invalidateFromSource(child);
            children.pop_front();
        }

        cities[source] = node(source);
    }

    bool findRoute(const index& From, const index& goal){



        auto comp = [&](const index& lhs, const index& rhs)
        {return cities[lhs].dist > cities[rhs].dist;};
        priority_queue<index, vector<index>, decltype(comp) > openSet(comp, Fringe);


        openSet.push(From);
        cities[From].validated = false;


        while(!openSet.empty()){
            node& curr = cities[openSet.top()];
            openSet.pop();
            /*cout<<"BEFORE\n";
            for(auto& c : cities){
                cout<<DATA[c.cityInd].name<<"\t"<<c.dist<<'\t';
                if(c.parent != -1) cout<<DATA[c.parent].name<<'\t';
                if(c.validated) cout<<"VALID\t";
                if(c.incoming) cout<<c.incoming->ID;
                cout<<"\n";
            }*/

            if(curr.validated) continue;
            curr.validated = true;

            if(curr.cityInd == goal) {
                Fringe = vector<index>(openSet.size());
                while (!openSet.empty()){
                    Fringe.push_back(openSet.top());
                    openSet.pop();
                }
                return true;
            }


            const auto& neighbours = DATA[curr.cityInd].getShortestEdges(curr.cityInd, curr.dist);
            for(auto n : neighbours)
            {
                node& To = cities[n.second->To];
                if(!To.validated && To.dist > n.first+curr.dist){
                    curr.children.push_back(To.cityInd);

                    To.dist     = n.first + curr.dist;
                    To.incoming = n.second;
                    To.parent     = curr.cityInd;

                    openSet.push(To.cityInd);
                }
            }


            /*auto copy = openSet;
            cout<<"TEST "<<DATA[curr.cityInd].name<<" TEST";
            if(curr.incoming)cout<<" incoming: "<<curr.incoming->ID;
            cout<<"\n\n";
            while(!copy.empty()){
                auto test = cities[copy.top()];
                cout<<DATA[test.cityInd].name<<"\t"<<test.dist<<'\t';
                if(test.incoming) cout<<test.incoming->ID;
                cout<<"\n";
                copy.pop();
            }
            cout<<"*****************************\n\n\n";*/
        }

        return false;


    }
};

#endif // LIFEHACK_H_INCLUDED
