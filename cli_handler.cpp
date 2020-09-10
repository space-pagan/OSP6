#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <string.h>
#include <vector>
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

char** makeargv(std::string line, int& size) {
	istringstream iss(line);
	std::vector<std::string> argvector;
	while (iss) {
		std::string sub;
		iss >> sub;
		argvector.push_back(sub);
	}
	char** out = new char*[argvector.size()];
	for (int i = 0; i < argvector.size()-1; i++) {
		out[i] = new char[argvector[i].size()];
		strcpy(out[i], argvector[i].c_str());
	}
	size = argvector.size();
	out[size-1] = NULL;
	return out;
}
