#include <iostream>

using namespace std;
//#define FringeHeurestics

#define VERBOSE

#include "path_find.h"

#include <ctime>
int main()
{
    ofstream OP("parancsok.txt");
    try{
        InstanceHandler inst(cout);
        inst.showProcess = false;

        auto time = clock();
        //inst.loadData("large_test_map.txt", "large_test_container.txt");
        //inst.loadData("small_test_map.txt", "small_test_container.txt");
        //inst.loadData("test_ship.txt", "test_cont.txt");
        inst.loadData("menetrend.txt", "rakomany.txt");
        cout << "FILE LOADING FINISHED IN " << clock() - time << " miliseconds\n\n";

        cout << "SUMMING LOADED DATA: \n";
        inst.printSum(cout);

        time = clock();
        inst.solveAll();
        cout << "\nSOLUTION GIVEN IN " << clock() - time << " miliseconds\n"
             << "\nTOTAL CITY LOOKUPS: " << inst.getStepCount() << endl;
        inst.printDetail(cout);
        ints.printOperations(OP);
    }catch (exception& e)
    {
        cout << e.what() << '\n';
    }



    cout<<"\n\n\nNYOMJ ENTERT A KILEPESHEZ";
    cin.get();
    return 0;
}
