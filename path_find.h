#ifndef LIFEHACK_H_INCLUDED
#define LIFEHACK_H_INCLUDED


#include "data_io.h"


class InstanceHandler
{
    using c = shared_ptr<Container>;
    DataHandler DATA;

    bool show;
    ostream& logOutput;

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

    void printNodes(ostream& o){
        o<<"\n\tMAPPED NODES \n";
        o<<"\t**********\n";
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

    void printFringe(ostream& o){
        o<<"\n\tFRINGE\n";
        o<<"\t***********\n";
        auto cp = Fringe.elements;
        while(!cp.empty()){
            logOutput<<cp.top().first<<'\t'<<cp.top().second<<'\n';
            cp.pop();
        }
    }

    void setInstance(c client){
        last_source = last_valid = client->From;
        nodes.clear();
        nodes.reserve(DATA.cities.size());

        nodes[client->From].distance = 0;
        nodes[client->From].ID = client->From;
        Fringe.put(client->From, 0);
    }
    void setOperations( c client){

        if(client->From != last_source)
            setInstance(client);
        //Valid  =  knows the shortest path to itself
        if(nodes[client->To].state != node::valid){
            if(!findRoute(client->To))
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
                curr->state = node::unvisited;
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

            cout<<"\n\nBefore\n";
            printFringe(cout);
            last_valid = last_invalid->parent->ID;
            Fringe.clear();
            cout<<"\n\nADDING LEAVES\n"
                <<"--------------------\n\n";
            addLeavesToFringe(last_invalid->parent);
            cout<<"--------------------";
            cout<<"\n\nAFTER\n";
            printFringe(cout);
        }

        if(remainder){
            DATA.requests.push_front(remainder);
        }
    }

    bool addLeavesToFringe(node* root, int indent = 0){
        auto& test = *root;
        if(root->state != node::unvisited){
            bool hasValidLeaf = false;
            for(auto& child : root->children)
                if(addLeavesToFringe(child, indent+1)) hasValidLeaf = true;
                for (int i = 0; i < indent; i++) logOutput<<' ';
                if(indent) logOutput << "-> ";
                else       logOutput << "SOURCE: ";
                logOutput << root-> ID;
            if(!hasValidLeaf){
                //IF A NODE IS VALID, THEN THE PATHFINDER WILL SKIP
                logOutput<< " ADDED\n";
                root->state = node::visited;
                Fringe.put(root->ID, root->distance);
                return true;
            } else logOutput<< "\n";
        }

        return false;
    }

    bool findRoute(const string& goal){

        while (!Fringe.empty()){

            auto curr = Fringe.get();
            auto& curr_node = nodes[curr];

            if(show){
                logOutput<<"\n\n****************************************\n";
                logOutput<<"TEST VERTEX "<<curr<<"\n";
                logOutput<<"****************************************\n";
                printNodes(logOutput);
            }

            if(curr_node.state == node::valid) continue;
            curr_node.state = node::valid;
            curr_node.children.clear();
            if(curr == goal)
                return true;


            if(show){
                logOutput<<"\n\n\tNEIGHBOUR EDGES \n";
                logOutput<<"\t**********\n";
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
                    if(show) logOutput<<"+++ ";
                } else if(show) logOutput<<"    ";

                if(show) {
                    logOutput<<"distance: "<<nb_dist<<'\t';
                    e.first->print(logOutput);
                }
            }
            if(show) printFringe(logOutput);
        }
        return false;
    }
};

#endif // LIFEHACK_H_INCLUDED
