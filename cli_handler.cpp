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
