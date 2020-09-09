#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <string>
using namespace std;

int getcliarg(int argc, char** argv, char opt, int &out) {
	int c;
	int arg;
	opterr = 0;
	char optstr[2];
	sprintf(optstr, "%c:", opt);

	if (argc == 1) {
		cout << argv[0] << " requires the option -" << opt;
		cout << " followed by an argument in order to run!\n";
		return 1;
	}

	while ((c = getopt(argc, argv, optstr)) != -1) {
		if (c == opt) {
			out = stoi(optarg);
			return 0;
		} else if (c == '?') {
			if (optopt == opt) {
				cout << "Option -" << opt << " requires an argument.\n";
			} else {
				cout << "Unknown option -" << (char)optopt << ".\n";
			}
			return 1;
		} else {
			return 1;
		}
	}
	return 1;
}
