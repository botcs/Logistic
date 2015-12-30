#include <iostream>

using namespace std;
//#define FringeHeurestics

#define VERBOSE

#include "instance.h"

#include <ctime>
int main(int argc, char* argv[])
{

    unordered_map<const char*, const char*> options;

    options["-ship"] = "menetrend.txt";
    options["-cont"] = "rakomany.txt";

    for(int i = 0; i < argc; i++){
        if(argv[i][0] == '-')
            options[ argv[i] ] = argv[i + 1];
    }

    try{
        InstanceHandler inst;

        inst.LogType(InstanceHandler::showAll);

        inst.loadData(options["-ship"], options["-cont"]);

        inst.solveAll(true);

        inst.printLog(cout);

    }catch (exception& e)
    {
        cout << e.what() << '\n';
    }

    return 0;
}
