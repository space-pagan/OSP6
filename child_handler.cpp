/* Author: Zoya Samsonov
 * Date: September 11, 2020
 */

#include "child_handler.h"

void forkexec(char* file, char** argv, int& pr_count) {
	// wrapper to perform a fork() call, immediately followed by an exec() call
	// to 'file' with arguments 'argv'. If the fork() is successful, pr_count
	// (number of active children) is increased
	switch(fork()) {
		case -1:
			// fork() failed. Print the error and terminate.
			perrandquit();
			return;
		case 0:
			execvp(file, argv);
			// this line is only reachable if execvp() failed. Print the error
			// and terminate (the child process)
			perrandquit();
			return;
		default:
			// fork() succeeded. Increment pr_count and return
			pr_count++;
			return;
	}
}

int updatechildcount(int& pr_count) {
	// checks if any children have exited, without waiting.
	// if a child has exited, decrement pr_count
	int wstatus;
	switch(waitpid(-1, &wstatus, WNOHANG)) {
		case -1:
			// waitpid() failed. Print the error and terminate.
			perrandquit();
			return -1;
		case 0:
			// no children have terminated. return and do not wait.
			return 0;
		default:
			// a child has terminated. Decrement pr_count and return
			// the child's exit status.
			pr_count--;
			return wstatus;
	}
}

int waitforanychild(int& pr_count) {
	// waits for a child to exit, and decrements pr_count when one has.
	int wstatus;
	switch(waitpid(-1, &wstatus, 0)) {
		case -1:
			// waitpid() failed. Print the error and terminate.
			perrandquit();
			return -1;
		default:
			// wait until a child terminates, then decrement pr_count
			// and return the child's exit status.
			pr_count--;
			return wstatus;
	}
}
