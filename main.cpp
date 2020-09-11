#include <iostream>
#include <string>
#include "cli_handler.h"
#include "child_handler.h"
#include "error_handler.h"

using namespace std;

void freestrarray(char**, int);

int main(int argc, char **argv) {
	int pr_limit;
	int pr_count = 0;

	setupprefix(argv[0]);

	if (getcliarg(argc, argv, 'n', pr_limit)) {
		return -1;
	}

	std::string line;
	int child_argc;
	char** child_argv;
	while (std::getline(std::cin, line)) {
		child_argv = makeargv(line, child_argc);
		while (pr_count >= pr_limit) {
			waitforanychild(pr_count);
		}
		forkexec(child_argv[0], child_argv, pr_count);
		updatechildcount(pr_count);
		freestrarray(child_argv, child_argc);
	}

	while (pr_count > 0) {
		waitforanychild(pr_count);
	}
}

void freestrarray(char** array, int size) {
	for (int x = 0; x < size; x++) {
		delete[] array[x];
	}
	delete[] array;
}
