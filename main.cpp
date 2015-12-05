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

        auto time = clock();
        inst.loadData("large_test_map.txt", "large_test_container.txt");
        //inst.loadData("small_test_map.txt", "small_test_container.txt");
        cout << "FILE LOADING FINISHED IN " << clock() - time << " miliseconds\n";

        time = clock();
        inst.solveRequests();
        cout << "SOLUTION GIVEN IN " << clock() - time << " miliseconds\n";
        inst.printContainers(log);

    }catch (exception& e)
    {
        cout << e.what() << '\n';
    }
    return 0;
}
