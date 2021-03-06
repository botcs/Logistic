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
        return capacity/length < rhs.capacity/rhs.length;
    }

    inline unsigned getDist(const unsigned& cont_phase)const{
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

    inline bool reserve(const size_t& client_load){
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
        o<<To<<'\t'<<ID<<'\t'<<capacity<<'\t'<<length<<'\t'<<phase<<"\n";
    }


    edge(const string& to, const string& n, size_t c, unsigned l, unsigned p) :
        To(to), ID(n), capacity(c), length(l), phase(p) {}
};

struct Container{
    string ID;
    string From;
    string To;
    unsigned bonusTime;

    size_t stackSize;

    unsigned travelTime = container_default_phase;


    list<shared_ptr<edge> > travelPath;

    inline bool bonus() const{
        return travelTime<=bonusTime;
    }

    bool returned = false;

    inline void addShip (shared_ptr<edge> ship){
        travelPath.push_front(ship);
    }

    void clear() {
        travelPath.clear();
    }

    inline shared_ptr<Container> splitCont (size_t unload){
      ///considers itself solved for a given size,
      ///and throws back a ptr to a remainder
      ///with clear history
      if(unload >= stackSize)
        return NULL;
      shared_ptr<Container> remainder = make_shared<Container>();
      remainder->ID = ID;
      remainder->From = From;
      remainder->To = To;
      remainder->bonusTime = bonusTime;
      remainder->stackSize = stackSize - unload;
      stackSize = unload;
      return remainder;
    }



    void unloadCont (size_t unload){
        ///considers itself the remainder of the
        ///original stack of containers
        ///the history of the solved stack will not be
        ///stored on the level of Containers, but can be recovered from
        ///the Operations
        stackSize -= unload;
        clear();
        returned = false;
    }

    Container(){}
    Container(const string& i, const string& f,
              const string& to, const unsigned& ti,
              const size_t& size):
                  ID(i), From(f), To(to), bonusTime(ti), stackSize(size){}



    void print(ostream& o)const{

        o << '{' << ID << '}'
          << "\n\tWith stack size: " << stackSize
          << "\n\tadressed with bonus Time: " << bonusTime
          << "\n\tFrom: " << From
          << "\n\tTo: " << To << "\n";


        if(!travelPath.empty()){
            if(bonus()) o << "\n\t[*BONUS*] (";
            else           o << "\n\t[Out of time] (";

            o << travelTime << " of " << bonusTime << ")";

            o << "\n\tTrace: \n\t" << From;
            int indent = 1;
            for(auto e : travelPath){
                o << " --> [" << e->ID << "]" << " --> " << e->To << "\n\t";
                for(int i = 0; i< indent; i++) o << ' ';
                o << e->To;
                indent++;
            }
            o << " --- ARRIVAL\n";
        } else {
            o << " [not solved yet]\n";
        }

    }
};

using e = shared_ptr<edge>;
struct city
{
    map< string, list<e> > harbours;

    inline list<e>& operator [] (const string& i) {return harbours[i];}

    void print(ostream& o){
        o << "\nTo\tID\tcap\tlength\tphase\n"
          << separator;
        for(auto& p : harbours){
            for(auto& path : p.second){
                path->print(o);
            }
            o<<"\n";
        }
        o<<"\n\n";

    }

    inline vector<pair< e, unsigned> > getShortestEdges(const unsigned& phase)
    {
        vector<pair< e, unsigned> > result;
        for(auto& harb : harbours)
        {
            unsigned min_dist = -1;
            e best;
            for(auto& path : harb.second){
                auto act_dist = path->getDist(phase);
                if(act_dist < min_dist){
                    min_dist = act_dist;
                    best     = path;
                }
            }

            result.emplace_back(best, min_dist);
        }
        return result;
    }

    ~city(){
    }

};

struct Operation
{
    unsigned day;
    string   cont_ID;
    string   ship_ID;
    int bonus_address;
    unsigned amount;

    Operation(const shared_ptr<Container> client, const shared_ptr<edge> incoming):
        cont_ID(client->ID), ship_ID(incoming->ID), amount(client->stackSize)
    {
        day = client->travelTime - incoming->length;
        bonus_address = client->bonusTime - day;
        if(bonus_address < 0)  bonus_address = 0;
    }

    void print(ostream& o) const{
        o<<day<<'\t'<<ship_ID<<'\t'<<cont_ID<<'\t'<<bonus_address<<'\t'<<amount<<'\n';
    }

    bool operator < (const Operation& rhs) const{
        return day > rhs.day;
    }
};

#endif // DATA_CLASSES_H_INCLUDED
