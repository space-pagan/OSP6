/* Author: Zoya Samsonov
 * Date: October 6, 2020
 */

#include <sys/types.h>           //pid_t
#include <sys/wait.h>            //wait()
#include <unistd.h>              //fork(), execvp()
#include <csignal>               //kill(), SIGTERM
#include <sstream>               //istringstream
#include <vector>                //vector
#include <set>                   //set
#include <string>                //string
#include <cstring>               //strcpy
#include "error_handler.h"       //perrandquit
#include "child_handler.h"       //function defs for self

std::set<pid_t> PIDS;

char** makeargv(std::string line, int& size) {
    // tokenizes an std::string on whitespace and converts it to char**
    // with the last element being a nullptr. Saves the column size to 'size'
    // does not account for quote-enclosed arguments with whitespace
    std::istringstream iss(line);
    std::vector<std::string> argvector;
    while (iss) {
        // extract characters until the next whitespace, and add it to
        // argvector
        std::string sub;
        iss >> sub;
        argvector.push_back(sub);
    }
    // instantiate the char** array to be the correct size to hold all the
    // tokens, plus nullptr
    size = argvector.size();
    char** out = new char*[size];
    for (int i = 0; i < size-1; i++) {
        // instantiate the inner array and copy back the token
        out[i] = new char[argvector[i].size()];
        strcpy(out[i], argvector[i].c_str());
    }
    // the last token extracted from iss is "" (empty string), it's safe to
    // overwrite this with nullptr
    out[size-1] = nullptr;
    return out;
}

void freeargv(char** argv, int size) {
    // deallocate a char*[x] created with new[] where x=size
    for (int x = 0; x < size; x++) {
        // deallocate inner arrays
        delete[] argv[x];
    }
    // deallocate the outer array
    delete[] argv;
}

int forkexec(std::string cmd, int& pr_count) {
    // alias if cmd is a std::string instead of a char*
    return forkexec(cmd.c_str(), pr_count);
}

int forkexec(const char* cmd, int& pr_count) {
    // fork a child process, equivalent to running cmd in bash
    // and increment external pr_count
    int child_argc;
    // convert cmd to argv format
    char** child_argv = makeargv(cmd, child_argc);
    const pid_t child_pid = fork();
    switch(child_pid) {
        case -1:
            // fork() failed. Print the error and terminate.
            perrandquit();
            return -1;
        case 0:
            // fork() succeeded. Only the child process runs this section
            if (execvp(child_argv[0], child_argv) == -1) {
                customerrorquit("Unable to exec '" +\
                        std::string(child_argv[0]) + "'");
            }
            return 0; // not reachable but gcc complains if its not there
        default:
            // fork() succeeded. Add child pid to list in case we need to
            // terminate children with killallchildren()
            // Update external pr_count and deallocate the argv array.
            PIDS.insert(child_pid);
            pr_count++;
            freeargv(child_argv, child_argc);
            return child_pid;
    }
}

int updatechildcount(int& pr_count) {
    // Does not wait for children to exit, only checks if any have exited
    // since last wait() call. If yes, decrement external pr_count
    int wstatus;
    pid_t pid;
    switch((pid = waitpid(-1, &wstatus, WNOHANG))) {
        case -1:
            // waitpid() failed. Print the error and terminate.
            perrandquit();
            return -1; // unreachable but gcc complains if its not there
        case 0:
            // no children have terminated. return and do not wait.
            return 0;
        default:
            // a child has terminated. Remove it from PIDS and decrement
            // external pr_count, before returning the child exit status
            PIDS.erase(pid);
            pr_count--;
            return wstatus;
    }
}

int waitforanychild(int& pr_count) {
    // waits for a child to exit, and decrements pr_count when one has.
    int wstatus;
    pid_t pid;
    switch((pid = waitpid(-1, &wstatus, 0))) {
        case -1:
            // waitpid() failed. Print the error and terminate.
            perrandquit();
            return -1; // unreachable but gcc complains if its not there
        default:
            // a child has terminated. Remove it from PIDS and decrement
            // external pr_count, before returning the child exit status
            PIDS.erase(pid);
            pr_count--;
            return wstatus;
    }
}

int waitforchildpid(int pid, int& pr_count) {
    // waits for a child with PID to exit and decrements pr_count
    int wstatus;
    pid_t pid_exit;
    switch((pid_exit = waitpid(pid, &wstatus, 0))) {
        case -1:
            // waitpid() failed. Print the error and terminate.
            perrandquit();
            return -1;
        default:
            // the child terminated. Remove it from PIDS and decrement
            // external pr_count, before returning the child exit status
            PIDS.erase(pid);
            pr_count--;
            return wstatus;
    }
}

void killallchildren() {
    // sends SIGTERM to any PID in the PIDS vector
    for (int p : PIDS) 
        if (kill(p, SIGINT) == -1) 
            perrandquit();
}
