/* Author:      Zoya Samsonov
 * Created:     October 6, 2020
 * Last edit:   November 12, 2020
 */

#include <iostream>        //std::cout
#include "help_handler.h"  //Self func decs

void printhelp(std::string progname) {
    // print help instructions for the current program, attempts to follow
    // MAN format as closely as possible.
    std::cout << "\nUsage: ";
    std::cout << progname << " [OPTION]...\n";
    std::cout << "Creates children and simulates resource management and";
    std::cout << " deadlock avoidance algorithms.\n";

    std::cout << "\n  -v        \tSets logging to verbose mode.";

    std::cout << "\n  -h        \tPrints this message\n\n";
}
