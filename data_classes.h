#ifndef DATA_CLASSES_H_INCLUDED
#define DATA_CLASSES_H_INCLUDED

#define index size_t
#define container_default_phase 0

struct edge;
struct Container{
    string ID;
    index From;
    index To;
    unsigned Time;

    size_t stack_size;

    unsigned phase = container_default_phase;


    list<edge*> travel;

    bool   solvable = true;

    Container(const string& i, const index& f,
              const index& to, const unsigned& ti,
              const size_t& size):
                  ID(i), From(f), To(to), Time(ti), stack_size(size){}


    bool operator < (const Container& rhs) const{
            return Time>rhs.Time;
    }
    string print()const{
        stringstream ss;
        print(ss);
        return ss.str();
    }
    void print(ostream& o)const{
        o<<ID<<'\t'<<From<<'\t'<<To<<'\t'<<Time<<'\n';
    }
};

struct edge
{
    size_t load = 0;
    edge* back = 0;
    index To;
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

    bool addContainer (){
        load=(load+1)%capacity;
        if(!load) phase+=(length+back->length);
        return load;
    }

    bool addContainer(const size_t& client_load){
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

struct city
{
    string name;

    city(const index& i, edge* e, const string& _name) : name(_name) {
        harbours[i] = list<edge*> {e};
    }

    map< index, list<edge *> > harbours;

    list<edge *>& operator [] (const index& i) {return harbours[i];}

    void print(ostream& o){
        string separator = "************************************************\n";
        o<<separator<<name;
        o<<"\nTo\tID\tcap\tlength\tphase\n"<<separator;
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

    vector<pair<unsigned,edge*> > getShortestEdges(const index& From, const unsigned& phase)
    {
        vector<pair<unsigned, edge*> > result;
        for(auto& harb : harbours)
        {
            unsigned min_dist = -1;
            edge* best = 0;
            for(auto& route : harb.second){
                if(harb.first == From) continue;
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

    Operation(const Container& client, const edge* incoming, unsigned arrival_day):
        cont_ID(client.ID), ship_ID(incoming->ID), bonus(client.Time), amount(client.stack_size)
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
