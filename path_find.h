#ifndef LIFEHACK_H_INCLUDED
#define LIFEHACK_H_INCLUDED


#include "data_io.h"


class InstanceHandler
{
    using c = shared_ptr<Container>;
    DataHandler DATA;

    bool show;
    ostream& logOutput;

    string lastSource;
    priority_queue<Operation> operations;

    struct node{
        shared_ptr<edge> incoming;
        string parent;
        list<string > children;
        unsigned distance;
        bool valid = false;
    };

    struct PQ{
        typedef pair<unsigned, string> PQElement;
        priority_queue<PQElement, vector<PQElement>, std::greater<PQElement> > elements;
        inline bool empty() { return elements.empty(); }

        inline void put(string item, unsigned priority) {
            elements.emplace(priority, item);
        }

        inline string get() {
            string best_item = elements.top().second;
            elements.pop();
            return best_item;
        }
    };
    unordered_map<string, node> nodes;

public:

    void setInstance(){
        nodes.clear();
        nodes.reserve(DATA.cities.size());
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


    void setOperations( c client){


        //Valid  =  knows the shortest path to itself
        if(!nodes[client->To].valid){
            if(!findRoute(client->From, client->To))
            {
                logOutput <<"\nPATH not found for ";
                client->print(logOutput, true);
                logOutput <<"\n*********************\n";
            }
        }

        reserveRoute(client);
    }


    void reserveRoute(c client){

        const string& start = client->From;
        const string& goal = client->To;
        client -> travelTime = nodes[goal].distance;



        //GET MAX CAPACITY ON CURRENT SOLUTION
        string last_invalid;
        string curr = goal;
        size_t max_load = -1;
        while(curr != start)
        {
            auto currMax = nodes[curr].incoming->getFreeSize();
            if(currMax<=max_load){
                max_load = currMax;
                last_invalid = curr;
            }
            curr = nodes[curr].parent;
        }

        c remainder = client->splitCont(max_load);

        if( !client->processed && !client->solvable() ){
            client->clear();
            DATA.requests.push_back(client);
            DATA.requests.pop_front();
            return;
        }

        //RESERVE SHIPS
        curr = goal;
        while(curr != start){

            auto& currCity = nodes[curr];
            operations.emplace(client, currCity.incoming, nodes[curr].distance);
            currCity.incoming->reserve(max_load);
            client->addShip(currCity.incoming);

            curr = nodes[curr].parent;
        }

        if(remainder){
            DATA.requests.push_front(remainder);
        }
    }


    void invalidateFromSource(const string& source){

        auto& children = nodes[source].children;
        while(!children.empty()){
            auto& child = children.front();
            invalidateFromSource(child);
            children.pop_front();
        }

        nodes[source].valid = false;
    }

    bool findRoute(const string& start, const string& goal){
        PQ Fringe;
        Fringe.put(start, 0);
        nodes[start].parent = start;
        while (!Fringe.empty()){
            auto curr = Fringe.get();
            if(curr == goal) break;

            for (auto& e : DATA[curr].getShortestEdges(nodes[curr].distance)){

            }
        }

    }
};

#endif // LIFEHACK_H_INCLUDED
