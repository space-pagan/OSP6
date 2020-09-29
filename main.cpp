/* Author: Zoya Samsonov
 * Date: Semptember 28, 2020
 */

#include <iostream>
#include <string>
#include "cli_handler.h"
#include "child_handler.h"
#include "error_handler.h"

int main(int argc, char **argv) {
	int max;
	int conc;
	int max_time;
	int max_count = 0;
	int conc_count = 0;
	int args[3] = {4, 2, 100}; // defaults for max, conc, max_time

	// set perror to display the correct program name
	setupprefix(argv[0]);

	if (getcliarg(argc, argv, "nst", args) != 0) {
		// if -1 is returned, argument parsing failed, quit.
		return -1;
	}
	max = args[0];
	conc = std::min(args[1], 20);
	max_time = args[2];
	char* infile = argv[argc-1];

	std::cout << "Maximum Processes:  " << max << "\n";
	std::cout << "Maximum Concurrent: " << conc << "\n";
	std::cout << "Maximum Run-time:   " << max_time << "\n";
	return 0;

	/*if (pr_limit < 1) {
		// the 'while (pr_count >= pr_limit) statement below cannot ever
		// terminate if pr_limit is less than one, so we force-quit early.
		return 0;
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
	}*/
}

void freestrarray(char** array, int size) {
	// frees memory of a 2d array created with nested new[] calls
	// size represents the number of "columns"
	for (int x = 0; x < size; x++) {
		delete[] array[x];
	}
	delete[] array;
}
