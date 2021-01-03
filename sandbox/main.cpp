#include <iostream>
#include "lsquic.h"

int main()
{
    std::cout << "Hello, World!" << std::endl;
    if(0 != lsquic_global_init(LSQUIC_GLOBAL_CLIENT|LSQUIC_GLOBAL_SERVER))
    {
        exit(EXIT_FAILURE);
    }
    std::cout << "lsquic: init" << std::endl;
    return 0;
}

