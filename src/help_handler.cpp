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
    std::cout << "Creates children and simulates memory requests and";
    std::cout << " LRU paging algorithm.\n";

    std::cout << "\n  -h        \tPrints this message";
    std::cout << "\n  -m x      \tSpecify user memory selection method. Defaults to 0.";
    std::cout << "\n            \t    0 = random address, 1 = weighted address.\n\n";
}
