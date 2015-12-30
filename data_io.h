#ifndef DATA_IO_H_INCLUDED
#define DATA_IO_H_INCLUDED


#include "data_handler.h"
#include <windows.h>
#include <chrono>

using namespace std;

string conv (double i)
{
    stringstream ss;
    string s;
    ss<<i;
    ss>>s;
    return s;
}

double conv (string i)
{
    stringstream ss;
    double s;
    ss<<i;
    ss>>s;
    return s;
}

void setTextRed(){
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_INTENSITY);
}

void setTextGreen(){
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdout, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
}


void setTextWhite(){
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdout,
                                  FOREGROUND_RED |
                                  FOREGROUND_GREEN |
                                  FOREGROUND_BLUE |
                                  FOREGROUND_INTENSITY);
}

void setTextDef(){
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdout,
                                  FOREGROUND_RED |
                                  FOREGROUND_GREEN |
                                  FOREGROUND_BLUE);
}

class progressBar
{
public:

    const size_t barWidth;
    progressBar(const long unsigned& total,
                const long unsigned& processed,
                const size_t& width = 20):
          barWidth(width),
          _total(total),
          _processed(processed),
          _begin(std::chrono::steady_clock::now())
    {}
    void refresh()
    {
        using namespace std::chrono;

        if ( _processed == _total || (_processed-_last) * 1000 /_total > 0 )
        {
            _last = _processed;
            duration time_taken = Clock::now() - _begin;
            float percent_done = (float)_processed/_total;
            duration time_left = time_taken * (1/percent_done - 1);
            minutes minutes_left = duration_cast<minutes>(time_left);
            seconds seconds_left = duration_cast<seconds>(time_left - minutes_left);


            const size_t pos = barWidth * percent_done;
            setTextWhite();
            cout << "[";
            for (size_t i = 0; i < barWidth; ++i){
                if (i < pos){
                    setTextGreen();
                    cout << ".";
                }else if (i == pos){
                    setTextRed();
                    cout << "_@_/";
                }else{
                    setTextWhite();
                    cout << ".";
                }
            }
            setTextWhite();
            cout << "] ";
            setTextRed();
            cout << fixed << setprecision(2) << percent_done * 100 << "% ";

            setTextGreen();
            std::cout << "ETL: " << minutes_left.count() << "m " << seconds_left.count() << "s        \r";
            cout.flush();
        }


    }
    ~progressBar()
    {
        setTextDef();
    }
private:

    typedef std::chrono::steady_clock Clock;
    typedef Clock::time_point time_point;
    typedef Clock::period period;
    typedef std::chrono::duration<float, period> duration;
    const long unsigned& _total;
    const long unsigned& _processed;
    long unsigned _last = 0;
    time_point _begin;
    //...
};




class dataReader
{
public:

    void loadCont(const char* file_name, DataHandler& data){
        ifstream infile (file_name);
        if(!infile){
            string exc = "wrong filename: ";
            exc+= file_name;
            exc+= "\n";
            throw logic_error(exc);
        }

        string  ID, start, finish;
        int  amount, time;



        vector<shared_ptr<Container> > load;

        unsigned line_count = 0;
        size_t total_amount = 0;
        while(infile.good())
        {
            line_count++;
            infile>>ws;
            char commentSymbol = infile.peek();
            getline(infile,ID);
            if(commentSymbol == '#' || commentSymbol == -1) continue;

            stringstream ss (ID);
            //ONLY VALID LINES ARE ACCEPTED
            if(ss >>  ID >> amount >> start >> finish >> time){


                if(time < 0 || amount < 1){
                    stringstream err;
                    err << "\n\nERROR: Invalid parameter in line: ";
                    err << conv(line_count);
                    err << "\nIN FILE: ";
                    err << file_name;
                    err << "\nThe bonus time given is negative, or the amount is less then 1";
                    err << "\n" << separator;
                    throw logic_error(err.str() );
                }
                if(data.cities.count(start) && data.cities.count(finish)){
                    data.total += amount;
                    load.push_back(make_shared<Container>(ID, start, finish, unsigned(time), size_t(amount)));
                }
                else {
                    /*stringstream err;
                    cout << "\n\nERROR: Invalid parameter in line: "
                        << conv(line_count)
                        << "\nIN FILE: "
                        << file_name
                        << "\nContainer Starting or End point not existing, or are the same"
                        << "\n" << separator;
                    */
                    continue;
                    //throw logic_error(err.str() );
                }

                total_amount+=amount;
            }


        }

        if(total_amount < 1) {
            stringstream err;
            err << "\n\nERROR: end of file reached: "
                << file_name
                << "\nWith line count: "
                << conv(line_count)
                << "\nBut no usable data was found"
                << "\n" << separator;
            throw logic_error(err.str() );
        }

        auto comp = [](const shared_ptr<Container> lhs, const shared_ptr<Container> rhs){
            return (lhs->bonusTime < rhs->bonusTime ||
                    (lhs->bonusTime == rhs->bonusTime && lhs->From < rhs->From) );
        };
        sort(load.begin(), load.end(), comp);
        for (auto& p : load) data.pending.push_back(p);


    }
    void loadMap(const char* file_name, DataHandler& data){
        fstream infile (file_name);
        string  ID, start, finish;
        int  capac, to, back, phase;

        if(!infile){
            string exc = "wrong filename: ";
            exc+= file_name;
            exc+= "\n";
            throw logic_error(exc);
        }

        size_t line_count = 0;
        size_t total_amount = 0;
        while(infile.good())
        {
            line_count++;
            infile>>ws;
            char commentSymbol = infile.peek();
            getline(infile,ID);

            if(commentSymbol == '#' || commentSymbol == -1) continue;

            stringstream ss (ID);
            //ONLY VALID LINES ARE ACCEPTED
            if(ss >>  ID >> capac >> start >> finish >> to >> back >> phase){
                if(capac < 1 || to < 1 || back < 1){
                    stringstream err;
                    cout << "\n\nERROR: Invalid parameter in line: "
                        << conv(line_count)
                        << "\nIN FILE: "
                        << file_name
                        << "\nShip capacity, or one of the lengths are < 1"
                        << "\n" << separator;

                    continue;
                    //throw logic_error(err.str() );
                }

                data.insert(start, finish, ID, capac, to, back, phase);
                total_amount++;
            }

        }
        if(!total_amount) {
            stringstream err;
            err << "\n\nERROR: end of file reached: "
                << file_name
                << "\nWith line count: "
                << conv(line_count)
                << "\nBut no usable data was found"
                << "\n" << separator;
            throw logic_error(err.str() );
        }

    }

    dataReader(const char* ship_file, const char* cargo_file, DataHandler& data)
    {
        loadMap(ship_file, data);
        loadCont(cargo_file, data);
    }

    void test(){

    }


};


///DATA HANDLER PRINT FUNCTIONS IMPLEMENT
    float DataHandler::getClientPercent(){
        return 100 * (solved.size()+unsolved.size())/
                (solved.size()+unsolved.size()+pending.size());
    }

    float DataHandler::getLoadPercent(){
        return 100 * processed/total;
    }



    float DataHandler::getLoadProgress(){
        return processed/float(total);
    }


    void DataHandler::printCities(ostream& o){
        o << separator
         <<"CITIES\n";
        for(auto& vertex : cities){
            o << separator
             <<vertex.first<<"\n";
            vertex.second.print(o);
        }
    }
    void DataHandler::printSolutions(ostream& o){
        o << separator
         <<"SOLVED REQUESTS\n"
         <<separator;
        if(!solved.empty()){

            for(auto s : solved){
                s->print(o);
                o << "\n";
            }
        } else {
            o << "\t\tEMPTY\n";
        }
    }
    void DataHandler::printPending(ostream& o){
        o << separator
         <<"REQUESTS UNDER PROCESS\n"
         <<separator;
         if(!pending.empty()){
            for(auto us : pending){
                us->print(o);
                o << "\n";
            }
        } else {
            o << "\t\tEMPTY\n";
        }
    }

    void DataHandler::printUnsolved(ostream& o){
        o << separator
         <<"UNSOLVED pending\n"
         <<separator;
        if(!unsolved.empty()){

            for(auto s : unsolved){
                s->print(o);
                o << "\n";
            }
        } else {
            o << "\t\tEMPTY\n";
        }
    }

    void DataHandler::printRequests(ostream& o){
        printPending(o);
        printSolutions(o);
        printUnsolved(o);
    }

    void DataHandler::print(ostream& o){
        printCities(o);
        printRequests(o);
        printOperations(o);
    }

    void DataHandler::printOperations(ostream& o){
         o << separator
         <<"OPERATIONS\n";
        auto copy = operations;
        while(!copy.empty()){
            copy.top().print(o);
            copy.pop();
        }
    }


#endif // DATA_IO_H_INCLUDED
