#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

void forkexecandwait(char* file, char** argv) {
	pid_t forkpid = fork();
	if (forkpid == -1) {
		//no child created, errno set
		return;
	} else if (forkpid) {
		//the parent
		//cout << "Waiting on PID " << forkpid << "\n";
		//cout << "wait returned " << wait(NULL) << "\n";
		wait(NULL);
	} else {
		//the child
		int execreturn = execv(file, argv);
		cout << "exec failed with code: " << execreturn << "\n";
		exit(execreturn);
	}
}
