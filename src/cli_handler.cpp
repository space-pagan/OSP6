/* Author: Zoya Samsonov
 * Date: October 6, 2020
 */

#include <unistd.h>				//getopt()
#include <string>				//string
#include <cstring>				//strlen()
#include <sstream>				//ostringstream
#include "error_handler.h"		//custerrhelpprompt()
#include "cli_handler.h"		//Self func defs

char* getoptstr(const char* options, const char* flags) {
	int optlen = strlen(options);
	int flglen = strlen(flags);
	char* optstr = new char[optlen * 2 + flglen + 1];
	optstr[0] = ':';
	int i = 1;
	for (int j = 0; j < optlen; j++) {
		optstr[i++] = options[j];
		optstr[i++] = ':';
	}
	for (int j = 0; j < flglen; j++) {
		optstr[i++] = flags[j];
	}
	return optstr;
}

int getcliarg(int argc, char** argv, const char* options, \
		const char* flags, int* optout, bool* flagout) {
	// Searches argv for options and stores argument
	// if any arguments are not present or unknown flags are used,
	// returns -1 to tell main() to quit. Otherwise returns 0;
	char* optstr = getoptstr(options, flags);
	int c;
	opterr = 0;  // disable getopt printing error messages

	while ((c = getopt(argc, argv, optstr)) != -1) {
		if (c == '?') {
			custerrhelpprompt("Unknown argument '-" +\
					std::string(1, (char)optopt) +	"'");
		}
		if (c == ':') {
			custerrhelpprompt("Option '-" +\
					std::string(1, (char)optopt) + "' requires an argument!");
		}
		// check if the matched item is an argument
		int optindex = -1;
		for (int j = 0; j < (int)strlen(options); j++) {
			if (c == options[j]) {
				optindex = j;
				optout[optindex] = std::stoi(optarg);
				break;
			}
		}
		if (optindex == -1) {
			// must be a flag
			for (int j = 0; j < (int)strlen(flags); j++) {
				if (c == flags[j]) {
					flagout[j] = true;
					break;
				}
			}
		}
	}
	return optind;
}
