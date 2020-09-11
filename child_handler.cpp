#include "child_handler.h"

void forkexec(char* file, char** argv, int& pr_count) {
	switch(fork()) {
		case -1:
			perrandquit();
			return;
		case 0:
			execvp(file, argv);
			perrandquit();
			return;
		default:
			pr_count++;
			return;
	}
}

int updatechildcount(int& pr_count) {
	int wstatus;
	switch(waitpid(-1, &wstatus, WNOHANG)) {
		case -1:
			perrandquit();
			return -1;
		case 0:
			return 0;
		default:
			pr_count--;
			return wstatus;
	}
}

int waitforanychild(int& pr_count) {
	int wstatus;
	switch(waitpid(-1, &wstatus, 0)) {
		case -1:
			perrandquit();
			return -1;
		default:
			pr_count--;
			return wstatus;
	}
}
