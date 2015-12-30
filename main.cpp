#include <iostream>
#include <getopt.h>

using namespace std;
#define FringeHeurestics

#define VERBOSE

#include "instance.h"

#include <ctime>
int main(int argc, char* argv[])
{
    const char *serr = "SYNTAX ERROR use the following:\n -s <ships> -c <container> -o <operations> (output)\n [optional] -l <log> -p (detailed search)\n";
    char *contFile = NULL, *shipFile = NULL, *opFile = NULL, *logFile = NULL;
    char verb = InstanceHandler::showNothing;
    int index;
    int c;
    opterr = 0;
    while ((c = getopt (argc, argv, "s:l:o:c:p")) != -1)
        switch (c)
        {
        case 's':
            shipFile = optarg;
            break;
        case 'l':
            logFile = optarg;
            verb |= InstanceHandler::showClient;
            break;
        case 'o':
            opFile = optarg;
            break;
        case 'c':
            contFile = optarg;
            break;
        case 'v':
            verb |= InstanceHandler::showSearch;
            break;
        case '?':
            cout << serr;
            return 1;
        default:
            abort ();
        }


    for (index = optind; index < argc; index++)
        cout << "Non-option argument " << argv[index] << "\n";

    try{

        InstanceHandler inst;

        inst.LogType(verb);
        if(shipFile && contFile)
            inst.loadData(shipFile, contFile);
        else throw logic_error(serr);

        inst.solveAll(verb);

        if(logFile){
            ofstream log (logFile);
            if(!log){
                inst.printLog(log);
            }
        }

        if(opFile){
            ofstream op (opFile);
            if(!op){
                inst.printOperations(op);
            }
        }else throw logic_error(serr);

    }catch (exception& e)
    {
        cout << e.what() << '\n';
    }


    return 0;
}
