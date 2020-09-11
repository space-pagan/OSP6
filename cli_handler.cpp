#include "cli_handler.h"

using namespace std;

int getcliarg(int argc, char** argv, char opt, int &out) {
	int c;
	char optstr[4] = {':', opt, ':', '\0'};
	opterr = 0;

	c = getopt(argc, argv, optstr);
	if (c == opt) {
		out = stoi(optarg);
		return 0;
	} else if (c == ':') {
		cout << "Option -" << opt << " requires an argument.\n";
	} else if (c == -1) {
		cout << argv[0] << " requires the option -" << opt;
		cout << " followed by an argument in order to run!\n";
	} else {
		cout << "Unknown option -" << (char)optopt << ".\n";
	}

	return 1;
}

bool hascliflag(int argc, char** argv, char opt) {
	int c;
	char optstr[2] = {opt, '\0'};
	opterr = 0;

	c = getopt(argc, argv, optstr);
	if (c == opt) return 1;
	return 0;
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
