#ifndef LIFEHACK_H_INCLUDED
#define LIFEHACK_H_INCLUDED


#include "data_io.h"

class InstanceHandler
{
    using c = shared_ptr<Container>;
    DataHandler DATA;

    ostream& log;


    struct node{
        string ID;
        shared_ptr<edge> incoming;
        node* parent = nullptr;
        list<node*> children;
        unsigned distance = -1;
        enum{
            unvisited = 0,
            visited,
            valid
        } state = unvisited;
    };

    struct PQ{
        static unsigned long count;

        typedef pair<unsigned, string> PQElement;
        priority_queue<PQElement, vector<PQElement>, std::greater<PQElement> > elements;
        inline bool empty() { return elements.empty(); }

        inline void put(string item, unsigned priority) {
            count++;
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

    bool showStatus = true;
    bool showProcess = false;

    InstanceHandler(const char* ship_file, const char* cargo_file, const char* command_file,
                    ostream& logOutput = cout) : log (logOutput){
        loadData(ship_file, cargo_file);
        DATA.print(logOutput);
        solveAll();
        DATA.print(logOutput);
        ofstream OP(command_file);
        DATA.printOperations(OP);
    }

    InstanceHandler(ostream& logOutput = cout) : log (logOutput){
        PQ::count = 0;
    }

    void loadData(const char* ship_file, const char* cargo_file){
        dataReader(ship_file, cargo_file, DATA);
    }

    void solveAll(){
        while(!DATA.pending.empty()){
            auto currClient = DATA.pending.front();
            #ifdef VERBOSE
            if(showStatus){

                if(showProcess){
                    log << "\n\n" << separator << separator << separator;
                    log << "CURRENT CLIENT: ";
                    currClient->print(log);
                    log << separator << separator << separator << separator;
                }

                log << "\n PROGRESS: " << DATA.getLoadPercent() << "% \t"
                    << DATA.processed << " / "
                    << DATA.total << endl;


            }
            #endif // VERBOSE
            solveTopClient();

        }
        #ifdef VERBOSE
        if(showStatus){
            log << "\n PROGRESS: " << DATA.getLoadPercent() << "%\t Process terminated\n";
        }
        #endif // VERBOSE
    }

    void solveTopClient(){
        auto currClient = DATA.pending.front();
        DATA.pending.pop_front();

        if(currClient->returned){
            setPath(currClient);
            DATA.solved.push_back(currClient);

            return;
        }

        bool solvable = initPath(currClient);

        if(!currClient->bonus() && !DATA.pending.empty()){
            DATA.pending.push_back(currClient);
            currClient->returned = true;
            return;
        }

        if(solvable){
            setPath(currClient);
            DATA.solved.push_back(currClient);
        }
        else
            DATA.unsolved.push_back(currClient);

        DATA.processed += currClient -> stack_size;
    }


    void print(ostream& o){

        o<<"\nRecords: \n";
        DATA.print(o);
    }

    void printContainers(ostream& o){
        DATA.printRequests(o);
    }

    void printOperations(ostream& o){
        DATA.printOperations(o);
    }

    unsigned long getStepCount(){
        return PQ::count;
    }



protected:
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
        nodes[client->From].parent = nullptr;
        nodes[client->From].ID = client->From;


    }

    bool initPath(c client){


        #ifdef FringeHeurestics
        if(client->From != last_source)
            resetInstance(client);
        #else
            resetInstance(client);
        #endif

        //Valid  =  knows the shortest path to itself
        if(nodes[client->To].state != node::valid){
            Fringe.put(client->From, 0);
            nodes[client->From].state = node::visited;
            if(!findHeuresticPath(client->To, client->bonusTime)){
                return false;
            }
        }

        return true;
    }

    void setPath(c client){


        const string& goal = client->To;

        //GET MAX CAPACITY ON CURRENT SOLUTION
        node* last_invalid = nullptr;
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
            //FILL CONTAINER PATH LIST
            client->addShip(curr->incoming);
            curr = curr->parent;
        }

        c remainder = client->splitCont(max_load);
        client->travelTime = nodes[goal].distance;

        //RESERVE SHIPS
        DATA.reservePath(client);


        //IF ONE OF THE SHIPS GETS FULL
        if(last_invalid){
            last_valid = last_invalid->parent->ID;
            #ifdef FringeHeurestics
                Fringe.clear();
                addLeavesToFringe(last_invalid->parent);
            #endif // FringeHeurestics
        }

        if(remainder){
            DATA.pending.push_front(remainder);
        }
    }
    #ifdef FringeHeurestics
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
    #endif // FringeHeurestics

    ///HEURESTICS ARE THE FOLLOWING:
    /**
     *  IF A THE GOAL IS FOUND WITHIN THE BONUS TIME,
     *  NO FURTHER LOOKUPS ARE MADE
     *
     *  IF COMPILED WITH \FringeHeurestics\ SWITCH ON,
     *  LOOKUPS ARE ONLY MADE ON PATHS THAT ARE OBSOLETE
     *  - OBSOLETE ~ NO FREE CAPACITY ON SHIPS
     */

    bool findHeuresticPath(const string& goal, const unsigned& bonus){

        while (!Fringe.empty()){
            auto curr = Fringe.get();
            auto& curr_node = nodes[curr];



            if(curr_node.state == node::valid) {
                #ifdef VERBOSE
                if(showProcess)
                    log <<  "\n\n" << separator
                        << "SKIPPING VALID VERTEX\n" << curr << "\n";

                #endif // VERBOSE

                continue;
            }

            curr_node.state = node::valid;
            curr_node.children.clear();

            if(curr == goal){
                #ifdef VERBOSE
                if(showProcess)
                    log <<  "\n\n" << separator
                        << "FOUND GOAL\n" << curr << "\n";
                #endif // VERBOSE

                return true;
            }

            #ifdef VERBOSE
            if(showProcess){
                log <<  "\n\n" << separator
                    <<"TEST VERTEX "<<curr<<"\n"
                    <<separator
                    <<"\tBEFORE VISITING NEIGHBOURS\n";
                printNodes(log);
            }

            if(showProcess){
                log << "\n\n\tNEIGHBOUR EDGES \n";
                log << separator;
            }
            #endif // VERBOSE


            for (auto& e : DATA[curr].getShortestEdges(curr_node.distance)){

                auto& neighbour = nodes[e.first->To];
                auto nb_dist = curr_node.distance + e.second;
                if(neighbour.state == node::unvisited ||
                    (neighbour.state == node::visited && neighbour.distance > nb_dist)){
                    ///SHORTCUT IF A SOLUTION FOUND
                    if(e.first->To == goal && bonus >= nb_dist)
                        Fringe.put(e.first->To, 0);
                    else
                        Fringe.put(e.first->To, nb_dist);

                    neighbour.ID       = e.first->To;
                    neighbour.state    = node::visited;
                    neighbour.distance = nb_dist;
                    neighbour.incoming = e.first;
                    neighbour.parent   = &curr_node;
                    curr_node.children.push_back(&neighbour);
                    #ifdef VERBOSE
                        if(showProcess) log << "+++ ";
                    #endif // VERBOSE
                }
                #ifdef VERBOSE
                    else if(showProcess) log << "    ";

                    if(showProcess) {
                        log << "distance: " << nb_dist << " \t";
                        e.first->print(log);
                    }

                #endif // VERBOSE
            }
            #ifdef VERBOSE
            if(showProcess) {
                log << "\n\tAFTER VISITING NEIGHBOURS\n";
                printFringe(log);
            }
            #endif // VERBOSE
        }
        return false;
    }
};

unsigned long InstanceHandler::PQ::count = 0;

#endif // LIFEHACK_H_INCLUDED
