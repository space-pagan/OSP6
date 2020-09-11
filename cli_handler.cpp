/* Author: Zoya Samsonov
 * Date: September 11, 2020
 */

#include "cli_handler.h"

int getcliarg(int argc, char** argv, char opt, int &out) {
	// Searches argv for a single option 'opt' and stores its argument
	// if opt is not found or is missing the argument, return 1. Otherwise
	// return 0
	int c;
	// getopt( , , ":opt:") searches for -opt arg, and returns ':' 
	// if opt is present but arg is not
	char optstr[4] = {':', opt, ':', '\0'}; 
	opterr = 0;  // disable getopt printing error messages

	c = getopt(argc, argv, optstr);
	if (c == opt) {
		// -opt arg was found in argv. Convert arg to an int and store in 'out'
		out = std::stoi(optarg);
		return 0;
	} else if (c == ':') {
		// -opt is present with no argument. return 1.
		std::cout << "Option -" << opt << " requires an argument.\n";
	} else if (c == -1) {
		// -opt is not present. return 1
		std::cout << argv[0] << " requires the option -" << opt;
		std::cout << " followed by an argument in order to run!\n";
	} else {
		// getopt returned some other character. return 1
		std::cout << "Unknown option -" << (char)optopt << ".\n";
	}

	return 1;
}

bool hascliflag(int argc, char** argv, char opt) {
	// Searches argv for the flag 'opt', returning 1 if found or 0 if not.
	// This function is not used for Project 1, but was easy to write and may
	// be useful for future projects
	char optstr[2] = {opt, '\0'};
	opterr = 0;

	if (opt == getopt(argc, argv, optstr)) return 1;
	return 0;
}

char** makeargv(std::string line, int& size) {
	// tokenizes an std::string on whitespace and converts it to char**
	// with the last element being a nullptr. Saves the column size to 'size'
	std::istringstream iss(line);
	std::vector<std::string> argvector;
	while (iss) {
		// extract characters until the next whitespace, and add it to
		// argvector
		std::string sub;
		iss >> sub;
		argvector.push_back(sub);
	}
	// instantiate the char** array to be the correct size to hold all the
	// tokens, plus nullptr
	char** out = new char*[argvector.size()];
	for (int i = 0; i < argvector.size()-1; i++) {
		// instantiate the inner array and copy back the token
		out[i] = new char[argvector[i].size()];
		strcpy(out[i], argvector[i].c_str());
	}
	size = argvector.size();
	// the last token extracted from iss is "" (empty string), it's safe to
	// overwrite this with nullptr
	out[size-1] = nullptr;
	return out;
}
