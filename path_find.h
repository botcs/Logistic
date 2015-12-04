#ifndef LIFEHACK_H_INCLUDED
#define LIFEHACK_H_INCLUDED


#include "data_io.h"

class InstanceHandler
{
    using c = shared_ptr<Container>;
    DataHandler DATA;

    bool show;
    ostream& log;

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

        inline void clear() {
            elements = priority_queue<PQElement, vector<PQElement>, std::greater<PQElement> > ();
        }
    } Fringe;
    unordered_map<string, node> nodes;

    string last_valid;
    string last_source;

public:



    InstanceHandler(const char* ship_file, const char* cargo_file, const char* command_file,
                    bool show_process = true, ostream& logOutput = cout) :
        show(show_process), log (logOutput)
    {
        loadData(ship_file, cargo_file);
        solveRequests();
        ofstream OP(command_file);
        printOp(OP);
    }

    InstanceHandler(bool show_process = true, ostream& logOutput = cout):
        show(show_process), log (logOutput){}

    void loadData(const char* ship_file, const char* cargo_file){
        dataReader(ship_file, cargo_file, DATA);
        log << "File loading finished \n";
    }

    void solveRequests(){
        while(!DATA.requests.empty()){
            auto currClient = DATA.requests.front();
            if(show){
                log << separator
                    << "CURRENT CLIENT: ";

                currClient->print(log);

                log << separator << separator;
            }

            DATA.requests.pop_front();

            bool solvable = setRoute(currClient);

            if(solvable)
                reserveRoute(currClient);
        }
    }

    void print(ostream& o){

        o<<"\nRecords: ";
        DATA.print(o);

        o<<"Operations \n";
        printOp(o);

    }

    void printContainers(ostream& o){
        DATA.printClients(o);
    }

    void printOp(ostream& o){
        auto copy = operations;
        while(!copy.empty()){
            copy.top().print(o);
            copy.pop();
        }
    }

    void printNodes(ostream& o){
        o<<"\n\tMAPPED NODES \n";
        o<<separator;
        for(auto n : nodes){
            log <<  n.first << "\t";
            auto& c = n.second;

            if(c.incoming) log << c.incoming->ID<<"\t";
                else log << "-\t";
            if(c.state == node::valid) log << "VALID\t";
            else if (c.state == node::visited) log << "!\t";
            else log << "-\t";
            log << c.distance;
            log << "\n";
        }
    }

    void printFringe(ostream& o){
        o<<"\n\tFRINGE\n";
        o<<separator;
        auto cp = Fringe.elements;
        while(!cp.empty()){
            log << cp.top().first<<'\t'<<cp.top().second<<'\n';
            cp.pop();
        }
    }

    void resetInstance(c client){
        last_source = last_valid = client->From;
        nodes.clear();
        Fringe.clear();
        nodes.reserve(DATA.cities.size());
        nodes[client->From].distance = 0;
        nodes[client->From].ID = client->From;


    }
    bool setRoute( c client){
        auto& test = *client;
        if(client->From != last_source)
            resetInstance(client);
        //Valid  =  knows the shortest path to itself
        if(nodes[client->To].state != node::valid){
            Fringe.put(client->From, 0);
            nodes[client->From].state = node::visited;
            if(!findRoute(client->To)){
                DATA.unsolved.push_back(client);
                return false;
            }
        }

        return true;
    }


    void reserveRoute(c client){


        const string& goal = client->To;
        client -> travelTime = nodes[goal].distance;


        //GET MAX CAPACITY ON CURRENT SOLUTION
        node* last_invalid = NULL;
        node* curr = &nodes[goal];
        size_t max_load = -1;
        while(curr -> parent)
        {
            auto currMax = curr->incoming->getFreeSize();
            if(currMax<=max_load){
                max_load = currMax;
                curr->state = node::unvisited;
                last_invalid = curr;
            }
            curr = curr->parent;
        }

        c remainder = client->splitCont(max_load);

        if( !client->processed && !client->bonus()  && !DATA.requests.empty()){
            client->clear();
            DATA.requests.push_back(client);
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

            last_valid = last_invalid->parent->ID;
            Fringe.clear();
            addLeavesToFringe(last_invalid->parent);
        }

        if(remainder){
            DATA.requests.push_front(remainder);
        }
    }

    bool addLeavesToFringe(node* root, int indent = 0){
        if(root->state != node::unvisited){
            bool hasValidLeaf = false;
            for(auto& child : root->children)
                if(addLeavesToFringe(child, indent+1)) hasValidLeaf = true;
            if(!hasValidLeaf){
                //IF A NODE IS VALID, THEN THE PATHFINDER WILL SKIP

                root->state = node::visited;
                Fringe.put(root->ID, root->distance);
                return true;
            }
        }

        return false;
    }

    bool findRoute(const string& goal){

        while (!Fringe.empty()){

            //if(goal=="A") printNodes(cout);
            auto curr = Fringe.get();
            auto& curr_node = nodes[curr];

            if(show){
                log <<  "\n\n" << separator
                    <<"TEST VERTEX "<<curr<<"\n"
                    <<separator
                    <<"\tBEFORE VISITING NEIGHBOURS\n";
                printNodes(log);
            }

            if(curr_node.state == node::valid) continue;
            curr_node.state = node::valid;
            curr_node.children.clear();
            if(curr == goal)
                return true;


            if(show){
                log << "\n\n\tNEIGHBOUR EDGES \n";
                log << separator;
            }

            for (auto& e : DATA[curr].getShortestEdges(curr_node.distance)){

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
                    if(show) log << "+++ ";
                } else if(show) log << "    ";

                if(show) {
                    log << "distance: "<<nb_dist<<'\t';
                    e.first->print(log);
                }
            }
            if(show) {
                log << "\n\tAFTER VISITING NEIGHBOURS\n";
                printFringe(log);
            }
        }
        return false;
    }
};

#endif // LIFEHACK_H_INCLUDED
