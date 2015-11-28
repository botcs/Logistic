#include <iostream>

using namespace std;

#include "path_find.h"

int main()
{
    try{
        InstanceHandler inst("hajok.txt", "kontenerek.txt", "parancsok.txt");

    }catch (exception& e)
    {
        cout << e.what() << '\n';
    }
    return 0;
}
