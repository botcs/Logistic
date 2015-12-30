#ifndef LIFEHACK_H_INCLUDED
#define LIFEHACK_H_INCLUDED
#include "data_io.h"

class PathHandler
{
    DataHandler& DATA;
    ostream& log;

    using c = shared_ptr<Container>;



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

    bool showProcess = false;
    PathHandler(DataHandler& _DATA, ostream& _log) : DATA(_DATA), log(_log){}
    unsigned long stepsTotal = 0;

    void printNodes(ostream& o){
        o<<"\n\tMAPPED NODES \n";
        o<<"-------------------------------------\n";
        for(auto n : nodes){
            o <<  n.first << "\t";
            auto& c = n.second;

            if(c.incoming) o << c.incoming->ID<<"\t";
                else o << "-\t";
            if(c.state == node::valid) o << "VALID\t";
            else if (c.state == node::visited) o << "!\t";
            else o << "-\t";
            o << c.distance;
            o << "\n";
        }
    }

    void printFringe(ostream& o){
        o<<"\n\tFRINGE\n";
        o<<"-------------------------------------\n";
        auto cp = Fringe.elements;
        while(!cp.empty()){
            o << cp.top().first<<'\t'<<cp.top().second<<'\n';
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
        printNodes(cout);
        if(nodes[client->To].state != node::valid){
            Fringe.put(client->From, 0);
            nodes[client->From].state = node::visited;
            if(!findHeuresticPath(client->To, client->bonusTime, showProcess)){
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
                if(client -> stackSize >= max_load){
                    curr->state = node::unvisited;
                    last_invalid = curr;
                }

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
            stepsTotal++;
            auto curr = Fringe.get();
            auto& curr_node = nodes[curr];

            if(curr_node.state == node::valid)
                continue;

            curr_node.state = node::valid;
            curr_node.children.clear();

            if(curr == goal){

                return true;

            }



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
                }
            }
        }
        return false;
    }

    ///TALKATIVE IMPLEMENTATION
    bool findHeuresticPath(const string& goal, const unsigned& bonus, bool verbose){
        if(!verbose) return findHeuresticPath(goal, bonus);

        while (!Fringe.empty()){
            stepsTotal++;
            auto curr = Fringe.get();
            auto& curr_node = nodes[curr];


            if(showProcess){
                log <<  "\n\n" << separator
                    <<"TEST VERTEX "<<curr<<"\n"
                    <<separator;
            }

            if(curr_node.state == node::valid) {
                if(showProcess)
                    log << "SKIPPING VALID VERTEX\n"
                        << separator;

                continue;
            }

            curr_node.state = node::valid;
            curr_node.children.clear();

            if(curr == goal){
                if(showProcess)
                    log << "FOUND GOAL\n" << separator << "\n\n\n\n\n\n\n";

                return true;
            }

            if(showProcess){
                log <<"\tBEFORE VISITING NEIGHBOURS\n";
                printNodes(log);
                log << "\n\n\tNEIGHBOUR EDGES \n";
                log << "-------------------------------------\n";
            }


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
                        if(showProcess) log << "+++ ";
                }
                    else if(showProcess) log << "    ";

                    if(showProcess) {
                        log << "distance: " << nb_dist << " \t";
                        e.first->print(log);
                    }

            }
            if(showProcess) {
                log << "\n\tAFTER VISITING NEIGHBOURS\n";
                printFringe(log);
            }
        }

        return false;
    }
};



#endif // LIFEHACK_H_INCLUDED
