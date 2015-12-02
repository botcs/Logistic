#ifndef LIFEHACK_H_INCLUDED
#define LIFEHACK_H_INCLUDED


#include "data_io.h"


class InstanceHandler
{

    DataHandler DATA;

    bool show;
    ostream& logOutput;

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

    InstanceHandler(const char* ship_file, const char* cargo_file, const char* command_file,
                    bool show_process = true, ostream& o = cout) :
        show(show_process), logOutput (o)
    {
        dataReaderWriter(ship_file, cargo_file, DATA);
        logOutput<<"File loading finished \n";

        //DATA.print(logOutput);

        ofstream commands(command_file);
        while(!DATA.requests.empty()){
            setOperations(DATA.requests.front());
            DATA.requests.pop_front();
        }

        //print(logOutput);

    }

    void print(ostream& o){

        o<<"\nRecords: ";
        DATA.print(o);

        o<<"Operations \n";
        printOp(o);

    }
    void printOp(ostream& o){
        auto copy = operations;
        while(!copy.empty()){
            copy.top().print(o);
            copy.pop();
        }
    }


    void setOperations( Container* client){
        if (show){
            logOutput << "\nCURRENT CLIENT:\n";
            client->print(logOutput, false);
        }
        if(client->From != lastSource){
            setInstance();
            lastSource = client->From;
            cities[client->From].dist = 0;
        }

        //Validated  =  knows the shortest path to itself
        if(!cities[client->To].validated){
            if(!findRoute(client->From, client->To))
            {
                logOutput <<"\nPATH not found for ";
                client->print(logOutput, true);
                logOutput <<"\n*********************\n";
            }
        }

        reserveRoute(client);
    }


    void reserveRoute(Container* client){

        const index& start = client->From;
        const index& goal = client->To;
        client -> travelTime = cities[goal].dist;



        //GET MAX CAPACITY ON CURRENT SOLUTION
        index  last_invalid = cities.size();
        index  curr = goal;
        size_t max_load = -1;
        while(curr != start)
        {
            auto currMax = cities[curr].incoming->getFreeSize();
            if(currMax<=max_load){
                max_load = currMax;
                last_invalid = curr;
            }
            curr = cities[curr].parent;
        }

        Container* remainder = client->splitCont(max_load);

        if( !client->processed && !client->solvable() ){
            client->clear();
            DATA.requests.push_back(client);
            DATA.requests.pop_front();
            return;
        }

        //RESERVE SHIPS
        curr = goal;
        while(curr != start){

            auto& currCity = cities[curr];
            operations.emplace(client, currCity.incoming, cities[curr].dist);
            currCity.incoming->reserve(max_load);
            client->addShip(currCity.incoming);

            curr = cities[curr].parent;
        }


        //IF ONE OF THE ROUTES GOT FULLY LOADED
        //SO RESTORE THE NODES TO GIVE SHORTEST ROUTE
        /**
        if(last_invalid != cities.size()){

            invalidateFromSource(last_invalid);

            //NOW THE CURR IS THE LATEST VALID START THE SEARCH FROM HERE
            addLeavesToFringe(curr);
            findRoute(curr, goal);
        }**/

        if(remainder){
            DATA.requests.push_front(remainder);
        }
    }
    /**Not fast as complex
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
    */

    bool findRoute(const index& From, const index& goal){

        if(show){
            logOutput << "\n\n****************************************\n"
                      << "shortest path finding initialized from: "
                      << DATA[From].name<<" to: "<<DATA[goal].name<<'\n'
                      << "****************************************\n\n";
        }



        auto comp = [&](const index& lhs, const index& rhs)
        {return cities[lhs].dist > cities[rhs].dist;};
        priority_queue<index, vector<index>, decltype(comp) > openSet(comp, Fringe);


        openSet.push(From);
        cities[From].validated = false;


        while(!openSet.empty()){
            node& curr = cities[openSet.top()];
            openSet.pop();

            /*if(show){
                logOutput<<"  Before mapping neighbours\n";
                for(auto& c : cities){
                    logOutput<<DATA[c.cityInd].name<<'\t';

                    if(c.parent != -1) logOutput<<DATA[c.parent].name<<'\t';
                        else logOutput<<"-\t";
                    if(c.incoming) logOutput<<c.incoming->ID<<'\t';
                        else logOutput<<"-\t";
                    if(c.incoming) logOutput<<c.incoming->length<<'\t';
                        else logOutput<<"-\t";
                    if(c.validated) logOutput<<"VALID\t";
                        else logOutput<<"-\t";
                    if(c.dist != -1) logOutput<<c.dist;
                        else logOutput<<"-\t";
                    logOutput<<"\n";
                }
            }*/

            if(curr.validated) continue;
            curr.validated = true;

            if(curr.cityInd == goal) {
                if(show){
                    logOutput<<"Found goal: "<<DATA[curr.cityInd].name
                             <<" Final Distance: "<<curr.dist<<"\n"
                             << "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
                }
                return true;
            }


            const auto& neighbours = DATA[curr.cityInd].getShortestEdges(curr.cityInd, curr.dist);
            for(auto n : neighbours)
            {
                node& To = cities[n.second->To];
                if(To.dist > n.first+curr.dist){
                    curr.children.push_back(To.cityInd);

                    To.dist     = n.first + curr.dist;
                    To.incoming = n.second;
                    To.parent     = curr.cityInd;

                    openSet.push(To.cityInd);
                }
            }

            if (show){
                logOutput<<"****************************************\n";
                logOutput<<"TEST VERTEX "<<DATA[curr.cityInd].name<<"\n";
                logOutput<<"\n  After mapping neighbours\n";
                for(auto& c : cities){
                    logOutput<<DATA[c.cityInd].name<<'\t';
                    if(c.parent != -1) logOutput<<DATA[c.parent].name<<'\t';
                        else logOutput<<"-\t";
                    if(c.incoming) logOutput<<c.incoming->ID<<"\t";
                        else logOutput<<"-\t";
                    if(c.validated) logOutput<<"VALID\t";
                        else logOutput<<"-\t";
                    logOutput<<c.dist;
                    logOutput<<"\n";
                }
                logOutput<<"---------------------------------\n";
                logOutput<<"Actual priority queue\n";
                auto copy = openSet;
                while(!copy.empty()){

                    auto test = cities[copy.top()];
                    logOutput<<DATA[test.cityInd].name<<"\t"<<test.dist<<'\t';
                    if(test.incoming) logOutput<<test.incoming->ID;
                    logOutput<<"\n";
                    copy.pop();
                }
                logOutput<<"****************************************\n";
            }

        }

        return false;


    }
};

#endif // LIFEHACK_H_INCLUDED
