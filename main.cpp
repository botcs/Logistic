#include <iostream>

using namespace std;
//#define FringeHeurestics

#define VERBOSE

#include "instance.h"

#include <ctime>
int main()
{

    try{
        InstanceHandler inst;
        inst.loadData("menetrend.txt", "rakomany.txt");
        inst.printDetail(cout);

        inst.solveAll(true);

        //inst.printContainers(cout);

    }catch (exception& e)
    {
        cout << e.what() << '\n';
    }

    return 0;
}
