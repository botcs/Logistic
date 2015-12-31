#include <iostream>

using namespace std;

#define VERBOSE

#include "instance.h"
#include "randomGen.h"

#define FringeHeurestics
#include <ctime>
int main()
{
    ofstream OP("parancsok.txt"), log("lepesek.txt");
    try{
        //generate(500, 500);
        InstanceHandler inst;
        //inst.LogType(InstanceHandler::showAll);
        auto time = clock();
        //inst.loadData("large_test_map.txt", "large_test_container.txt");
        //inst.loadData("small_test_map.txt", "small_test_container.txt");
        //inst.loadData("test_ship.txt", "test_cont.txt");
        inst.loadData("menetrend.txt", "rakomany.txt");
        cout << "FILE LOADING FINISHED IN " << clock() - time << " miliseconds\n\n";

        inst.printSum(cout);

        time = clock();
        inst.solveAll(true);
        cout << "\nSOLUTION GIVEN IN " << clock() - time << " miliseconds\n"
             << "\nTOTAL CITY LOOKUPS: " << inst.getStepCount() << endl;
        inst.printOperations(OP);
    }catch (exception& e)
    {
        cout << e.what() << '\n';
    }

    cout<<"\n\n\nNYOMJ ENTERT A KILEPESHEZ";
    cin.get();
    return 0;
}
