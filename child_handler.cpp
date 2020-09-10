#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

void forkexec(char* file, char** argv, int& pr_count) {
	switch(fork()) {
		case -1:
			cout << "fork call failed\n";
			perror("proc_fan: Error");
			exit(-1);
		case 0:
			{	
				int execerr = execvp(file, argv);
				cout << "exec call failed\n";
				perror("proc_fan: Error");
				exit(-1);
			}
		default:
			pr_count++;
			return;
	}
}

void updatechildcount(int& pr_count) {
	int status;
	switch(waitpid(-1, &status, WNOHANG)) {
		case -1:
			cout << "wait call failed\n";
			perror("proc_fan: Error");
			exit(-1);
		case 0:
			return;
		default:
			pr_count--;
			return;
	}
}

void waitforanychild(int& pr_count) {
	int status;
	switch(waitpid(-1, &status, 0)) {
		case -1:
			cout << "wait call failed\n";
			perror("proc_fan: Error");
			exit(-1);
		default:
			pr_count--;
			return;
	}
}
