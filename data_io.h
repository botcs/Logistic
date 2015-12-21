#ifndef DATA_IO_H_INCLUDED
#define DATA_IO_H_INCLUDED


#include "data_handler.h"


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

class dataReader
{
public:

    void loadCont(const char* file_name, DataHandler& data){
        ifstream infile (file_name);
        if(infile.bad()){
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
                if(data.cities.count(start) && data.cities.count(finish))
                    load.push_back(make_shared<Container>(ID, start, finish, unsigned(time), size_t(amount)));
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

        if(infile.bad()){
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

#endif // DATA_IO_H_INCLUDED
