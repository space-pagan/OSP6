/* Author: Zoya Samsonov
 * Date: September 11, 2020
 */

#include "cli_handler.h"

int getcliarg(int argc, char** argv, const char* options, int* out) {
	// Searches argv for options and stores argument
	// if any arguments are not present or unknown flags are used,
	// returns -1 to tell main() to quit. Otherwise returns 0;
	char* optstr = new char[strlen(options) * 2 + 1];
	optstr[0] = ':';
	int i = 1;
	for (int j = 0; j < strlen(options); j++) {
		optstr[i++] = options[j];
		optstr[i++] = ':';
	}
	int c;
	opterr = 0;  // disable getopt printing error messages

	while ((c = getopt (argc, argv, optstr)) != -1) {
		int optindex = -1;
		if (c == '?') {
			std::cerr << "Unknown argument '-" << char(optopt) << "'\n";
			return -1;
		}
		if (c == ':') {
			std::cerr << "Option '-" << char(optopt) << "' requires an argument!\n";
			return -1;
		}
		for (int j = 0; j < strlen(options); j++) {
			if (c == options[j]) {
				optindex = j;
				break;
			}
		}
		out[optindex] = std::stoi(optarg);
	}
	return 0;
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
