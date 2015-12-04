#include <iostream>

using namespace std;

#include "path_find.h"

int main()
{
    ofstream log ("log.txt");
    try{
        InstanceHandler inst(false);
        inst.loadData("hajok.txt", "kontenerek.txt");
        inst.solveRequests();
        inst.printContainers(cout);

    }catch (exception& e)
    {
        cout << e.what() << '\n';
    }
    return 0;
}
