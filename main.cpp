#include <iostream>

using namespace std;

#include "path_find.h"

int main()
{
    ofstream log ("log.txt");
    try{
        InstanceHandler inst("hajok.txt", "kontenerek.txt", "parancsok.txt", false, cout);

    }catch (exception& e)
    {
        cout << e.what() << '\n';
    }
    return 0;
}
