/* Author: Zoya Samsonov
 * Date: Semptember 11, 2020
 */

#include <iostream>
#include <string>
#include "cli_handler.h"
#include "child_handler.h"
#include "error_handler.h"

void freestrarray(char**, int);

int main(int argc, char **argv) {
	int pr_limit;
	int pr_count = 0;

	// set perror to display the correct program name
	setupprefix(argv[0]);

	if (getcliarg(argc, argv, 'n', pr_limit)) {
		// get value of option -n as pr_limit or exit if 
		// the option is poorly formated or not present.
		return -1;
	}

	std::string line;
	int child_argc;
	char** child_argv;
	while (std::getline(std::cin, line)) {
		// while there is a line to read on stdin, tokenize it and store in
		// child_argv. The number of tokens + 1 (for the NULL pointer) is
		// stored in child_argc, and is used to free the array later.
		child_argv = makeargv(line, child_argc);
		while (pr_count >= pr_limit) {
			// if the maximum concurrent number of children are running
			// do not create any more until pr_count falls below pr_limit
			waitforanychild(pr_count);
		}
		// fork and exec a child
		forkexec(child_argv[0], child_argv, pr_count);
		// check if any children have exited and update the counter
		updatechildcount(pr_count);
		// free the child_argv array
		freestrarray(child_argv, child_argc);
	}

	// EOF read on stdin, do not create any more children, wait for
	// remaining children to terminate
	while (pr_count > 0) {
		waitforanychild(pr_count);
	}
}

void freestrarray(char** array, int size) {
	// frees memory of a 2d array created with nested new[] calls
	// size represents the number of "columns"
	for (int x = 0; x < size; x++) {
		delete[] array[x];
	}
	delete[] array;
}
