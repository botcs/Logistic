#ifndef INSTANCE_H_INCLUDED
#define INSTANCE_H_INCLUDED

#include "path_find.h"

class InstanceHandler
{
    using c = shared_ptr<Container>;
    DataHandler DATA;
    PathHandler PATH;
    bool showProcess = false;

    stringstream log;

public:


    enum{
        showNothing,
        showClient,
        showSearch,
        showAll
    };

    void LogType(char show_type){
        switch (show_type)
        {
        case showAll :
            PATH.showProcess = showProcess = true;
            break;

        case showClient :
            showProcess = true;
            break;

        case showSearch :
            PATH.showProcess = true;
            break;

        default :
            PATH.showProcess = showProcess = false;
        }
    }

    void printLog(ostream& o){
        o << log.str();
    }

    void printDetail(ostream& o){
        o<<"\nRecords: \n";
        DATA.print(o);
    }

    void printSum(ostream& o){
        o << "Number of..."
          << "\nCities:  \t" << DATA.cities.size()
          << "\nShips:   \t" << DATA.num_of_ships
          << "\nDeliveries:\t" << DATA.total;

    }

    void printContainers(ostream& o){
        DATA.printRequests(o);
    }

    void printOperations(ostream& o){
        DATA.printOperations(o);
    }

    unsigned long getStepCount(){
        return PATH.stepsTotal;
    }




    InstanceHandler() : PATH(DATA, log){log << "INSTANCE CREATED AT: " << this << '\n';}

    void loadData(const char* ship_file, const char* cargo_file){
        dataReader(ship_file, cargo_file, DATA);
    }

    void solveAll(bool verbose){
        if(!verbose) solveAll();

        log << "\nPROCESSING ALL REQUESTS... Current status:\n";

        progressBar PB(DATA.total, DATA.processed);
        while(!DATA.pending.empty()){
            auto currClient = DATA.pending.front();

            if(showProcess){
                log << separator << separator
                    << "CURRENT CLIENT: "
                    << "\n PROGRESS: "
                    << DATA.getClientPercent()<< "% \t"
                    << DATA.processed << " / "
                    << DATA.total << "\n\n" ;
                currClient->print(log);
                log << separator << separator << separator;
            }

            PB.refresh();

            solveTopClient();
        }
        PB.refresh();
        log << "\nProcessing terminated successfully\n";
    }

    void solveAll(){
        while(!DATA.pending.empty()){
            auto currClient = DATA.pending.front();

            solveTopClient();
        }
    }


    void solveTopClient(){
        auto currClient = DATA.pending.front();
        DATA.pending.pop_front();

        if(currClient->returned){
            PATH.setPath(currClient);
            DATA.solved.push_back(currClient);

            return;
        }

        bool solvable = PATH.initPath(currClient);

        if(!currClient->bonus() && !DATA.pending.empty()){
            DATA.pending.push_back(currClient);
            currClient->returned = true;
            return;
        }

        if(solvable){
            PATH.setPath(currClient);
            DATA.solved.push_back(currClient);
        } else
            DATA.unsolved.push_back(currClient);

        DATA.processed += currClient -> stackSize;
    }


};


#endif // INSTANCE_H_INCLUDED
