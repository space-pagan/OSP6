/* Author: Zoya Samsonov
 * Date: October 6, 2020
 */

#include <string>				//string
#include <iostream>				//cerr
#include "error_handler.h"		//Self func defs

// global variable to store argv[0] for error messages
std::string rawprefix;
std::string prefix;

void setupprefix(const char* arg0) {
	// sets the value of prefix. Run this at the very start
	// of the program to ensure that perror always displays
	// the correct error message
	prefix = arg0;
	rawprefix = arg0;
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

void customerrorquit(std::string error) {
	customerrorquit(error.c_str());
}

void custerrhelpprompt(const char* error) {
	std::cerr << prefix << ": " << error << "\n";
	std::cerr << "Please run '" << rawprefix;
	std::cerr << " -h' for more assistance!\n\n";
	exit(0);
}

void custerrhelpprompt(std::string error) {
	custerrhelpprompt(error.c_str());
}
