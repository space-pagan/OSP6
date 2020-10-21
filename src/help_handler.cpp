/* Author:      Zoya Samsonov
 * Created:     October 6, 2020
 * Last edit:   October 16, 2020
 */

#include <iostream>        //std::cout
#include "help_handler.h"  //Self func decs

void printhelp(const char* progname) {
    // print help instructions for the current program, attempts to follow
    // MAN format as closely as possible.
    std::cout << "\nUsage: ";
    std::cout << progname << " -l FILE  [OPTION]...\n";
    std::cout << "Creates children and simulates a system clock to get the";
    std::cout << " PID and time of child termination.\n";

    std::cout << "\nDefaults to '-c 5 -t 20' unless explicitly set.\n";

    std::cout << "\n  -l FILE\t";
    std::cout << "Set the output file to log the simulation information.";
    std::cout << " Note that this option is mandatory and a FILE must be";
    std::cout << " specified for " << progname << " to run";
    
    std::cout << "\n  -c MAXTHRD\tSet the maximum number of threads";

    std::cout << "\n  -t TIMEOUT\tSet the timeout period in seconds.";
    std::cout << " After this time has elapsed, " << progname;
    std::cout << " will terminate";

    std::cout << "\n  -h        \tPrints this message\n\n";
}
