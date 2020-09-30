/* Author: Zoya Samsonov
 * Date: Semptember 28, 2020
 */

#include <iostream>
#include <string>
#include <unistd.h>
#include <csignal>
#include "cli_handler.h"
#include "child_handler.h"
#include "error_handler.h"

void printhelp(const char* progname);

void signalhandler(int signum) {
	if (signum == SIGALRM) {
		customerrorquit("SIGALRM received!");
	} else if (signum == SIGINT) {
		customerrorquit("SIGINT received!");
	}
}

int main(int argc, char **argv) {
	// register SIGINT (^C) with signal handler
	signal(SIGINT, signalhandler);

	// local variables
	int max;
	int conc;
	int max_time;
	int max_count = 0;
	int conc_count = 0;
	int args[3] = {4, 2, 100}; // defaults for max, conc, max_time
	bool flags[1] = {false}; // only -h flag for this program

	// set perror to display the correct program name
	setupprefix(argv[0]);

	// parse runtime arguments
	int optind = getcliarg(argc, argv, "nst", "h", args, flags);
	
	// print help message and quit
	if (flags[0]) {
		printhelp(argv[0]);
		return 0;
	}

	// save parsed arguments to separate variables for easier access
	max = args[0];
	conc = std::min(args[1], 20); // do not allow > 20 concurrent children
	max_time = args[2];
	char* infile;
	if (argc > optind) {
		infile = argv[argc-1];
	} else {
		customerrorquit("FILE is required!");
	}

	// set up kill timer
	signal(SIGALRM, signalhandler);
	alarm(max_time);

	std::cout << "Maximum Processes:  " << max << "\n";
	std::cout << "Maximum Concurrent: " << conc << "\n";
	std::cout << "Maximum Run-time:   " << max_time << "\n";
	std::cout << "INFILE:             " << infile << "\n";
	while (true);
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

void printhelp(const char* progname) {
	std::cout << "\n" << progname << " [OPTIONS] FILE\n";
	std::cout << "Sort strings in FILE as palindromes or not and";
	std::cout << " write to palin.out / nopalin.out\n";
	std::cout << "\nOPTIONS:";
	std::cout << "\n  -n #\tSet the maximum number of strings to test from";
	std::cout << " the file\n\t(Default: 4)";
	std::cout << "\n  -s #\tSet the maximum number of concurrent processes";
	std::cout << "\n\t(Default: 2)";
	std::cout << "\n  -t #\tSet the timeout period in seconds. After this";
	std::cout << " time has elapsed, " << progname << " will terminate";
	std::cout << "\n\t(Default: 100)";
	std::cout << "\n  -h  \tPrints this message\n";
}
