#ifndef DATA_CLASSES_H_INCLUDED
#define DATA_CLASSES_H_INCLUDED

#define index size_t
#define container_default_phase 0

struct edge
{
    size_t load = 0;
    shared_ptr<edge> back = 0;
    string To;
    string ID;
    size_t capacity;
    unsigned length;
    unsigned phase;

    bool operator < (const edge& rhs) const {
        return capacity/double(length) < rhs.capacity/double(rhs.length);
    }

    unsigned getDist(const unsigned& cont_phase)const{
        unsigned native_phase;
        if(cont_phase<=phase)
            native_phase=phase-cont_phase;
        else{
            auto backforth=(length+back->length);
            auto m = (cont_phase+phase)%backforth;
            if(m) native_phase=(backforth)-m;
            else native_phase=0;
        }

        unsigned D = length + native_phase;
        return D;
    }

    bool reserve (){
        load=(load+1)%capacity;
        if(!load) phase+=(length+back->length);
        return load;
    }

    bool reserve(const size_t& client_load){
        auto overload = (load+client_load)/capacity;
        load = (load + client_load) % capacity;
        if(!load) phase += overload * (length+back->length);
        return load;
    }

    size_t getFreeSize ()const{
        return capacity - load%capacity;
    }

    string print(){
        stringstream ss;
        print(ss);
        return ss.str();
    }
    void print(ostream& o){
        o<<ID<<'\t'<<capacity<<'\t'<<length<<'\t'<<phase<<"\n\n";
    }


    edge(const string& n, size_t c, unsigned l, unsigned p) :
        ID(n), capacity(c), length(l), phase(p) {}
};

struct Container{
    string ID;
    string From;
    string To;
    unsigned bonusTime;

    size_t stack_size;

    unsigned travelTime = container_default_phase;


    list<shared_ptr<edge> > travelRoute;

    bool solvable() const{
        return travelTime<=bonusTime;
    }

    bool processed = false;

    void addShip (shared_ptr<edge> ship){
        travelRoute.push_back(ship);
        processed = true;
    }

    void clear() {
        travelRoute.clear();
        travelTime = container_default_phase;
        processed = true;
    }

    shared_ptr<Container> splitCont (size_t unload){
      ///considers itself solved for a given size,
      ///and throws back a ptr to a remainder
      ///with clear history
      if(unload == stack_size) return NULL;
      shared_ptr<Container> remainder = make_shared<Container>();
      remainder->ID = ID;
      remainder->From = From;
      remainder->To = To;
      remainder->bonusTime = bonusTime;
      remainder->stack_size = stack_size - unload;
      stack_size = unload;
      return remainder;
    }

    void unloadCont (size_t unload){
        ///considers itself the remainder of the
        ///original stack of containers
        ///the history of the solved stack will not be
        ///stored on the level of Containers, but can be recovered from
        ///the Operations
        stack_size -= unload;
        clear();
        processed = false;
    }

    Container(){}
    Container(const string& i, const string& f,
              const string& to, const unsigned& ti,
              const size_t& size):
                  ID(i), From(f), To(to), bonusTime(ti), stack_size(size){}


    string print()const{
        stringstream ss;
        print(ss, true);
        return ss.str();
    }
    void print(ostream& o, bool fail = false)const{

        o << '{' << ID << '}';

        if(fail){
             o<< "\n\tadressed with bonus Time: " << bonusTime
              << "\n\tWith stack size: " << stack_size
              << "\n\tFrom: " << From
              << "\n\tTo: " << To << "\n";
            return;
        }

        if(!travelRoute.empty()){
            if(solvable()) o << "\t*BONUS* (";
            else           o << "\tOut of time (";

            o << travelTime << " of " << bonusTime << ")";

            o << " Backtracking: \n";
            int indent = 0;
            for(auto e : travelRoute){
                for(int i = 0; i< indent; i++) o << ' ';
                o << " -->[" << e->ID << "]\n";
                indent++;
            }
            o << '\n';
        } else {
            o << " Have not been solved yet\n";
        }

    }
};

using e = shared_ptr<edge>;
struct city
{
    map< string, list<e> > harbours;

    list<e>& operator [] (const string& i) {return harbours[i];}

    void print(ostream& o){
        o<<"\nTo\tID\tcap\tlength\tphase\n"
         <<"************************************************\n";
        for(auto& p : harbours){
            o<<p.first;
            for(auto& route : p.second){
                o<<'\t';
                route->print(o);
            }
            o<<"\n";
        }
        o<<"\n\n";

    }

    vector<pair<unsigned, e> > getShortestEdges(const unsigned& phase)
    {
        vector<pair<unsigned, e> > result;
        for(auto& harb : harbours)
        {
            unsigned min_dist = -1;
            e best;
            for(auto& route : harb.second){
                auto act_dist = route->getDist(phase);
                if(act_dist < min_dist){
                    min_dist = act_dist;
                    best     = route;
                }
            }
            result.emplace_back(min_dist, best);
        }
        return result;
    }

    ~city(){
    }
    //const list<edge *>& operator [] (const index& i) const {return harbours[i];}
};

struct Operation
{
    unsigned day;
    string   cont_ID;
    string   ship_ID;
    unsigned bonus;
    unsigned amount;

    Operation(const shared_ptr<Container> client, const shared_ptr<edge> incoming, unsigned arrival_day):
        cont_ID(client->ID), ship_ID(incoming->ID), bonus(client->bonusTime), amount(client->stack_size)
    {
        day = arrival_day - incoming->length;
        //print(cout);
    }

    void print(ostream& o) const{
        o<<day<<'\t'<<ship_ID<<'\t'<<cont_ID<<'\t'<<bonus<<'\t'<<amount<<'\n';
    }

    bool operator < (const Operation& rhs) const{
        return day > rhs.day;
    }
};

#endif // DATA_CLASSES_H_INCLUDED
