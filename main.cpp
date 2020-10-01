  /* Author: Zoya Samsonov
 * Date: Semptember 28, 2020
 */

#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <csignal>
#include <stdio.h>
#include "cli_handler.h"
#include "child_handler.h"
#include "error_handler.h"
#include "shm_handler.h"

void signalhandler(int signum) {
	if (signum == SIGALRM) {
		customerrorquit("SIGALRM received!");
	} else if (signum == SIGINT) {
		customerrorquit("SIGINT received!");
	}
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

int main(int argc, char **argv) {
	// register SIGINT (^C) with signal handler
	signal(SIGINT, signalhandler);

	// set perror to display the correct program name
	setupprefix(argv[0]);

	// local variables
	int max;
	int conc;
	int max_time;
	int max_count = 0;
	int conc_count = 0;
	char* infile;
	int opts[3] = {4, 2, 100}; // defaults for max, conc, max_time
	bool flags[1] = {false}; // only -h flag for this program

	// parse runtime arguments
	int optind = getcliarg(argc, argv, "nst", "h", opts, flags);
	
	// print help message and quit
	if (flags[0]) {
		printhelp(argv[0]);
		return 0;
	}

	// save parsed arguments to separate variables for easier access
	if (argc > optind) {
		infile = argv[argc-1];
	} else {
		customerrorquit("FILE is required!");
	}
	max = opts[0];
	if (max < 1) {
		customerrorquit("option -n must be an integer greater than 0");
	}
	conc = std::min(opts[1], 20); // do not allow > 20 concurrent children
	if (conc < 1) {
		customerrorquit("option -s must be an integer greater than 0");
	} else if (conc < opts[1]) {
		std::cout << "This system does not allow more than 20 concurrent";
		std::cout << " processes. Option -s has been set to 20.\n";
	}
	max_time = opts[2];
	if (max_time < 1) {
		customerrorquit("option -t must be an integer greater than 0");
	}
	// set up kill timer
	// signal(SIGALRM, signalhandler);
	// alarm(max_time);

	// SHM TEST, tested 9/30
	int startid = 0;
	int currid = 0;
	int child_argc;
	char** child_argv;
	shmfromfile(infile, currid, max);

	while (max_count < max) {
		while (conc_count >= conc) {
			waitforanychild(conc_count);
		}
		char cmd[16];
		sprintf(cmd, "palin %d", startid+max_count);
		child_argv = makeargv(cmd, child_argc);
		forkexec(child_argv[0], child_argv, conc_count);
		max_count++;
		updatechildcount(conc_count);
		freeargv(child_argv, child_argc);
	}
	// reached maximum total children. Wait for all remaining to quit
	while (conc_count > 0) {
		waitforanychild(conc_count);
	}

	for (int i = startid; i < currid; i++) {
		shmdestroy(i);
	}
	return 0;
}
