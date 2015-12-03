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
        string ID;
        shared_ptr<edge> incoming;
        node* parent = NULL;
        list<node*> children;
        unsigned distance = -1;
        enum{
            unvisited = 0,
            visited,
            valid
        } state = unvisited;
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

        DATA.printCities(logOutput);
        ofstream commands(command_file);
        while(!DATA.requests.empty()){
            setOperations(DATA.requests.front());
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
        if(nodes[client->To].state != node::valid){
            nodes[client->From].distance = 0;
            nodes[client->From].ID = client->From;
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
        DATA.requests.pop_front();

        const string& goal = client->To;
        client -> travelTime = nodes[goal].distance;

        /*logOutput<<"NODES:\n\n";
        for(auto& n : nodes){
            logOutput << n.first << "\t" << n.second.valid << "\t"
            << n.second.distance << "\t" << n.second.incoming << "\n";
        }*/

        //GET MAX CAPACITY ON CURRENT SOLUTION
        node* last_invalid = NULL;
        node* curr = &nodes[goal];
        size_t max_load = -1;
        while(curr -> parent)
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
        while(curr -> parent){
            operations.emplace(client, curr->incoming, curr->distance);
            curr->incoming->reserve(max_load);
            client->addShip(curr->incoming);

            curr = curr->parent;
        }

        DATA.solved.push_back(client);


        //IF ONE OF THE SHIPS GETS FULL
        if(last_invalid){
            auto last_valid = last_invalid->parent;
            invalidateFromSource(last_invalid);

            PQ copy;
            while (!Fringe.empty()){
                const auto& candidate = Fringe.elements.top();
                if(nodes[candidate.second].state == node::valid)
                    copy.put(candidate.second, candidate.first);
                Fringe.elements.pop();
            }
            addBranchesToFringe(last_valid);
        }


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
        source->state = node::unvisited;
    }

    void addBranchesToFringe(node* root){

            if(root->state){
                Fringe.put(root->ID, root->distance);
                for(auto& child : root->children)
                    addBranchesToFringe(child);
            }


    }

    bool findRoute(const string& start, const string& goal){


        Fringe.put(start, nodes[start].distance);
        nodes[start].state = node::unvisited;

        while (!Fringe.empty()){


            auto curr = Fringe.get();
            auto& curr_node = nodes[curr];

            if(show){
                logOutput<<"\n\n****************************************\n";
                logOutput<<"TEST VERTEX "<<curr<<"\n";
                logOutput<<"****************************************\n";
                logOutput<<"\n\tMAPPED NODES \n";
                logOutput<<"\t**********\n";
                for(auto n : nodes){
                    logOutput<< n.first << "\t";
                    auto& c = n.second;

                    if(c.incoming) logOutput<<c.incoming->ID<<"\t";
                        else logOutput<<"-\t";
                    if(c.state == node::valid) logOutput<<"VALID\t";
                    else if (c.state == node::visited) logOutput<<"!\t";
                    else logOutput<<"-\t";
                    logOutput<<c.distance;
                    logOutput<<"\n";
                }

            }

            if(curr_node.state == node::valid) continue;
            curr_node.state = node::valid;
            if(curr == goal)
                return true;


            if(show){
                logOutput<<"\n\n\tNEIGHBOUR EDGES \n";
                logOutput<<"\t**********\n";
            }

            for (auto& e : DATA[curr].getShortestEdges(curr_node.distance)){
                if(show) {
                    logOutput<<"distance: "<<e.first->getDist(curr_node.distance)<<'\t';
                    e.first->print(logOutput);

                }
                auto& neighbour = nodes[e.first->To];
                auto nb_dist = curr_node.distance + e.second;
                if(neighbour.state == node::unvisited ||
                    (neighbour.state == node::visited && neighbour.distance > nb_dist)){
                    Fringe.put(e.first->To, nb_dist);
                    neighbour.ID       = e.first->To;
                    neighbour.state    = node::visited;
                    neighbour.distance = nb_dist;
                    neighbour.incoming = e.first;
                    neighbour.parent   = &curr_node;
                    curr_node.children.push_back(&neighbour);
                }
            }

            if(show){
                logOutput<<"\n\tFRINGE\n";
                logOutput<<"\t***********\n";
                auto cp = Fringe.elements;
                while(!cp.empty()){
                    logOutput<<cp.top().first<<'\t'<<cp.top().second<<'\n';
                    cp.pop();
                }
            }
        }
        return false;
    }
};

#endif // LIFEHACK_H_INCLUDED
