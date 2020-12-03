/* Author:      Zoya Samsonov
 * Created:     September 9, 2020
 * Last edit:   November 17, 2020
 */

#include <sys/types.h>           //pid_t
#include <sys/wait.h>            //wait()
#include <unistd.h>              //fork(), execvp()
#include <csignal>               //kill(), SIGTERM
#include <sstream>               //istringstream
#include <vector>                //vector
#include <set>                   //set
#include <string>                //string
#include <cstring>               //strcpy()
#include <sys/stat.h>            //stat()
#include "error_handler.h"       //perrandquit()
#include "util.h"                //range()
#include "child_handler.h"       //function defs for self

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
    for (int i : range(size-1)) {
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
    for (int x : range(size)) {
        // deallocate inner arrays
        delete[] argv[x];
    }
    // deallocate the outer array
    delete[] argv;
}

int childman::findfirstunset() {
    // returns the first unset bit in a bitmap (used for PID)
    for (int i : range(18)) if (!this->bitmap[i]) return i;
    return -1;
}

int childman::findandsetpid() {
    // finds the first unset bit in a bitmap, sets it, and returns the index
    int pid;
    if ((pid = findfirstunset()) != -1) {
        this->bitmap.set(pid);
    }
    return pid;
}

void childman::unsetpid(int pid) {
    // unsets a bit in a bitmap (releases PID back into the pool)
    this->bitmap.reset(pid);
}

int childman::forkexec(std::string cmd) {
    // fork a child process, equivalent to running cmd in the shell
    // Return PID of forked child
    int virt_pid = this->findandsetpid();
    if (virt_pid == -1) return -1;
    cmd += " " + std::to_string(virt_pid);
    int child_argc;
    // convert cmd to argv format
    char** child_argv = makeargv(cmd, child_argc);
    const pid_t child_pid = fork();
    switch(child_pid) {
        case -1:
            // fork() failed. Print the error and terminate.
            perrandquit();
            return -1; // not reachable but gcc complains if its not there
        case 0:
            // fork() succeeded. Only the child process runs this section
            if (execvp(child_argv[0], child_argv) == -1) {
                customerrorquit("Unable to exec '" +\
                        std::string(child_argv[0]) + "'");
            }
            return 0; // not reachable but gcc complains if its not there
        default:
            // fork() succeeded. Map real PID to virtual PID, free memory used
            // for argv, and return the virtual PID of the child.
            this->PIDS[child_pid] = virt_pid;
            freeargv(child_argv, child_argc);
            return virt_pid; // return the PID of the created child
    }
}

int childman::updatechildcount() {
    return this->waitforchildpid(-1, WNOHANG);
}

int childman::waitforanychild() {
    return this->waitforchildpid(-1, 0);
}

int childman::waitforchildpid(int pid) {
    return this->waitforchildpid(pid, 0);
}

int childman::waitforchildpid(int pid, int status) {
    // waits for a child with PID to exit and decrements pr_count
    int wstatus;
    pid_t pid_exit;
    switch((pid_exit = waitpid(pid, &wstatus, status))) {
        case -1:
            // waitpid() failed. Print the error and terminate.
            perrandquit();
            return -1;
        case 0:
            if (status == WNOHANG)
                return -1;
            return 0;
        default:
            // the child terminated. Remove it from PIDS and decrement
            // external pr_count, before returning the pid of the exited child 
            this->unsetpid(this->PIDS[pid_exit]);
            this->PIDS.erase(pid_exit);
            return pid_exit;
    }
}

void childman::killallchildren() {
    // sends SIGTERM to any PID in the PIDS vector
    for (auto p : this->PIDS) 
        if (kill(p.first, SIGINT) == -1) 
            perrandquit();
}

void childman::cleanup() {
    this->killallchildren();
    while (this->PIDS.size()) 
        this->waitforanychild();
}


