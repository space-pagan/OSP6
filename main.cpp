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

volatile bool earlyquit = false;
volatile int quittype = 0;

void signalhandler(int signum) {
	if (signum == SIGALRM) {
		quittype = SIGALRM;
		earlyquit = true;
	} else if (signum == SIGINT) {
		quittype = SIGINT;
		earlyquit = true;
	}
}

void printhelp(const char* progname) {
	std::cout << "\nUsage: ";
	std::cout << progname << " [OPTION]... FILE...\n";
	std::cout << "Determine whether lines in FILE are palindromes. If yes,";
	std::cout << " the line is copied to 'palin.out', otherwise it is";
	std::cout << " copied to 'nopalin.out'.\n";

	std::cout << "\nDefaults to '-n 4 -s 2 -t 100' unless explicitly set.\n";

	std::cout << "\n  -n MAXLINE\t";
	std::cout << "Set the maximum number of lines to test from the file";
	
	std::cout << "\n  -s MAXTHRD\tSet the maximum number of threads";

	std::cout << "\n  -t TIMEOUT\tSet the timeout period in seconds.";
	std::cout << " After this time has elapsed, " << progname;
	std::cout << " will terminate";

	std::cout << "\n  -h        \tPrints this message\n";
}

void testopts(int argc, char** argv, int optind, int max, int& conc,\
		int max_time, bool* flags) {
	// print help message and quit
	if (flags[0]) {
		printhelp(argv[0]);
		exit(0);
	}

	if (argc <= optind) customerrorquit("FILE is required!");
	if (argc > optind+1) {
		std::string errmsg("Unknown argument '");
		errmsg += argv[optind];
		errmsg += "'";
		customerrorquit(errmsg.c_str());
	}
	if (max < 1) customerrorquit(
			"option -n must be an integer greater than 0");
	if (conc < 1) customerrorquit(
			"option -s must be an integer greater than 0");
	if (conc > 20) {
		std::cout << "This system does not allow more than 20 concurrent";
		std::cout << " processes. Option -s has been set to 20.\n";
		conc = 20;
	}
	if (max_time < 1) customerrorquit(
			"option -t must be an integer greater than 0");
}

void main_loop(int max, int conc, char* infile) {
	int max_count = 0;
	int conc_count = 0;
	int startid = 0;
	int currid = 0;
	int child_argc;
	char** child_argv;

	shmfromfile(infile, currid, max);
	// change max to be the number of lines read, may be fewer than -n option
	max = currid - startid;
	int semid = semcreate(1, currid);
	semunlock(semid, 0);

	while (max_count++ < max) {
		if (earlyquit) {
			// semdestroy(semid); // forces sleeping threads to wake up
			killallchildren();
			if (quittype == SIGINT) {
				std::cerr << "SIGINT received! Terminating...\n";
			} else if (quittype == SIGALRM) {
				std::cerr << "Timeout limit exceeded! Terminating...\n";
			}
			break;
		}
		while (conc_count >= conc) {
			waitforanychild(conc_count);
		}
		char cmd[16];
		sprintf(cmd, "palin %d %d", startid+max_count-1, semid);
		forkexec(cmd, conc_count);
		updatechildcount(conc_count);
	}
	// reached maximum total children. Wait for all remaining to quit
	while (conc_count > 0) {
		waitforanychild(conc_count);
	}

	// release all shared memory created
	for (int i = startid; i < currid; i++) {
		shmdestroy(i);
	}
	// if (!earlyquit)	semdestroy(semid);
	semdestroy(semid);
}

int main(int argc, char **argv) {
	// register SIGINT (^C) and SIGALRM with signal handler
	signal(SIGINT, signalhandler);
	signal(SIGALRM, signalhandler);
	// set perror to display the correct program name
	setupprefix(argv[0]);

	// parse runtime arguments
	int opts[3] = {4, 2, 100}; // defaults for max, conc, max_time
	bool flags[1] = {false}; // only -h flag for this program
	int optind = getcliarg(argc, argv, "nst", "h", opts, flags);

	// local variables
	int max = opts[0];
	int conc = opts[1];
	int max_time = opts[2];
	char* infile = argv[argc-1];
	// this line will terminate the program if any options are mis-set
	testopts(argc, argv, optind, max, conc, max_time, flags);
	// set up kill timer
	alarm(max_time);
	main_loop(max, conc, infile);

	return 0;
}
