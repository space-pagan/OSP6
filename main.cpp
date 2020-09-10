#include <iostream>
#include <string>
#include <stdio.h>
#include "cli_handler.h"
#include "child_handler.h"

using namespace std;

void freestrarray(char**, int);

int main(int argc, char **argv) {
	int pr_limit;

	if (getcliarg(argc, argv, 'n', pr_limit)) {
		return -1;
	}
	cout << "Started " << argv[0] << " with " << pr_limit << " maximum concurrent child(ren)!\n";
	std::string line;
	char** targs;
	while (std::getline(std::cin, line)) {
		int size;
		targs = makeargv(line, size);
		forkexecandwait(targs[0], targs);
		freestrarray(targs, size);
	}
}

void freestrarray(char** array, int size) {
	for (int x = 0; x < size; x++) {
		delete[] array[x];
	}
	delete[] array;
}
