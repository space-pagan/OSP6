/* Author: Zoya Samsonov
 * Date: September 11, 2020
 */

#include "cli_handler.h"

char* getoptstr(const char* options, const char* flags);

int getcliarg(int argc, char** argv, const char* options, \
		const char* flags, int* out, bool* flagout) {
	// Searches argv for options and stores argument
	// if any arguments are not present or unknown flags are used,
	// returns -1 to tell main() to quit. Otherwise returns 0;
	char* optstr = getoptstr(options, flags);
	char errstr[256];
	int c;
	opterr = 0;  // disable getopt printing error messages

	while ((c = getopt (argc, argv, optstr)) != -1) {
		if (c == '?') {
			sprintf(errstr, "Unknown argument '-%c'", optopt);
			customerrorquit(errstr);
		}
		if (c == ':') {
			sprintf(errstr, "Option '-%c' requires an argument!", optopt);
			customerrorquit(errstr);
		}
		// check if the matched item is an argument
		int optindex = -1;
		for (int j = 0; j < strlen(options); j++) {
			if (c == options[j]) {
				optindex = j;
				out[optindex] = std::stoi(optarg);
				break;
			}
		}
		if (optindex == -1) {
			// must be a flag
			for (int j = 0; j < strlen(flags); j++) {
				if (c == flags[j]) {
					flagout[j] = true;
					break;
				}
			}
		}
	}
	return optind;
}

char* getoptstr(const char* options, const char* flags) {
	int optlen = strlen(options);
	int flglen = strlen(flags);
	char* optstr = new char[optlen * 2 + flglen + 1];
	optstr[0] = ':';
	int i = 1;
	for (int j = 0; j < strlen(options); j++) {
		optstr[i++] = options[j];
		optstr[i++] = ':';
	}
	for (int j = 0; j < strlen(flags); j++) {
		optstr[i++] = flags[j];
	}
	return optstr;
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
