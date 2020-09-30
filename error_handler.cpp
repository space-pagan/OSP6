/* Author: Zoya Samsonov
 * Date: September 11, 2020
 */

#include "error_handler.h"

// global variable to store argv[0] for error messages
std::string prefix;

void setupprefix(char* arg0) {
	// sets the value of prefix. Run this at the very start
	// of the program to ensure that perror always displays
	// the correct error message
	prefix = arg0;
	prefix += ": Error";
}

void perrandquit() {
	// print a detailed error message in the format 
	// argv[0]: Error: detailed error message
	// then, terminate the program
	perror(prefix.c_str());
	exit(0);
}

void customerrorquit(const char* error) {
	// print a custom error message in the format
	// argv[0]: Error: custom message
	// then, terminate the program
	std::cerr << prefix << ": " << error << "\n";
	exit(0);
}
