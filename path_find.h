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
        node* parent;
        list<node*> children;
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
    } Fringe;
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
        node* last_invalid = NULL;
        node* curr = &nodes[goal];
        node* start_ptr = &nodes[start];
        size_t max_load = -1;
        while(curr != start_ptr)
        {
            auto currMax = curr->incoming->getFreeSize();
            if(currMax<=max_load){
                max_load = currMax;
                last_invalid = curr;
            }
            curr = curr->parent;
        }

        c remainder = client->splitCont(max_load);

        if( !client->processed && !client->solvable() ){
            client->clear();
            DATA.requests.push_back(client);
            DATA.requests.pop_front();
            return;
        }

        //RESERVE SHIPS
        curr = &nodes[goal];
        while(curr != start_ptr){
            operations.emplace(client, curr->incoming, curr->distance);
            curr->incoming->reserve(max_load);
            client->addShip(curr->incoming);

            curr = curr->parent;
        }

        //IF ROUTE

        if(remainder){
            DATA.requests.push_front(remainder);
        }
    }


    void invalidateFromSource(node* source){

        auto& children = source->children;
        while(!children.empty()){
            auto& child = children.front();
            invalidateFromSource(child);
            children.pop_front();
        }
        source->valid = false;
    }

    bool findRoute(const string& start, const string& goal){
        Fringe.put(start, 0);
        nodes[start].parent = NULL;
        while (!Fringe.empty()){
            auto curr = Fringe.get();
            if(curr == goal) return true;

            for (auto& e : DATA[curr].getShortestEdges(nodes[curr].distance)){
                if(!nodes[e.first->To].valid || nodes[e.first->To].distance > e.second){
                    Fringe.put(e.first->To, e.second);
                    nodes[e.first->To].distance = e.second;
                }


            }
        }
        return false;
    }
};

#endif // LIFEHACK_H_INCLUDED
