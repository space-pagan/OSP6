#include <iostream>
#include <string>
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
		targs = makeargv(line);
		forkexecandwait(targs[0], targs);
		//freestrarray(targs);
	}
}

void freestrarray(char** array) {
	int rowsize = sizeof(array) / sizeof(array[0]);
	for (int x = 0; x < rowsize; x++) {
		delete[] array[x];
	}
	delete[] array;
}
