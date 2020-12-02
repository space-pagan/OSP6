#include <iostream>
#include <unistd.h>
#include "util.h"

int main(int argc, char** argv) {
    std::srand(getpid());
    roullette r(32);
    int i;
    std::cout << "Picking weighted random values: ";
    for (i = 0; i < 16; i++)
        std::cout << r.rand() << " ";
    std::cout << "\n";

    int picks[32] = {};
    for (i = 0; i < 1<<20; i++)
        picks[r.rand()]++;

    std::cout << "Normalized distribution:\n";
    int min = picks[0];
    for (int i : picks) if (i < min) min = i;
    for (int i : picks) printf("%5.2f ", (float)i/(float)min);
    std::cout << "\n";
}
