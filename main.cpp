#include <iostream>
#include <string>
#include <string.h>
#include <sstream>
#include <vector>
#include <stdio.h>
#include "cli_handler.h"
#include "child_handler.h"

using namespace std;

char** parseinput(std::string line);
void freestrarray(char** array);

int main(int argc, char **argv) {
	int pr_limit;

	if (getcliarg(argc, argv, 'n', pr_limit)) {
		return -1;
	}
	cout << "Started " << argv[0] << " with " << pr_limit << " maximum concurrent child(ren)!\n";
	std::string line;
	char** targs;
	while (std::getline(std::cin, line)) {
		targs = parseinput(line);
		forkexecandwait(targs[0], targs);
		//freestrarray(targs);
	}
}

char** parseinput(std::string line) {
	istringstream iss(line);
	std::vector<std::string> argvector;
	while (iss) {
		std::string sub;
		iss >> sub;
		argvector.push_back(sub);
	}
	char** out = new char*[argvector.size()];
	for (int i = 0; i < argvector.size(); i++) {
		out[i] = new char[argvector[i].size()];
		strcpy(out[i], argvector[i].c_str());
	}
	return out;
}

void freestrarray(char** array) {
	int rowsize = sizeof(array) / sizeof(array[0]);
	for (int x = 0; x < rowsize; x++) {
		delete[] array[x];
	}
	delete[] array;
}
