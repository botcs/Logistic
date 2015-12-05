#include <iostream>

using namespace std;

#include "path_find.h"
#include <ctime>
int main()
{
    ofstream log ("log.txt");
    try{
        InstanceHandler inst(log);
        inst.showProcess = false;
        inst.showStatus  = false;

        inst.loadData("menetrend.txt", "rakomany.txt");
        //inst.loadData("hajok.txt", "kontenerek.txt");
        auto time = clock();
        inst.solveRequests();
        cout << "SOLUTION GIVEN IN " << clock() - time << " miliseconds\n";
        //inst.printContainers(cout);

    }catch (exception& e)
    {
        cout << e.what() << '\n';
    }
    return 0;
}
