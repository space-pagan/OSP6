/* Author: Zoya Samsonov
 * Date: October 6, 2020
 */

#include <iostream>
#include "help_handler.h"

void printhelp(const char* progname) {
	std::cout << "\nUsage: ";
	std::cout << progname << " [OPTION]... FILE...\n";
	std::cout << "Determine whether lines in FILE are palindromes. If yes,";
	std::cout << " the line is copied to 'palin.out', otherwise it is";
	std::cout << " copied to 'nopalin.out'.\n";

	std::cout << "\nDefaults to '-n 4 -s 2 -t 100' unless explicitly set.\n";

	std::cout << "\n  -n MAXLINE\t";
	std::cout << "Set the maximum number of lines to test from the file";
	
	std::cout << "\n  -s MAXTHRD\tSet the maximum number of threads";

	std::cout << "\n  -t TIMEOUT\tSet the timeout period in seconds.";
	std::cout << " After this time has elapsed, " << progname;
	std::cout << " will terminate";

	std::cout << "\n  -h        \tPrints this message\n\n";
}
