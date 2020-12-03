#ifndef CHILD_HANDLER_H
#define CHILD_HANDLER_H

#include <string>
#include <map>
#include <bitset>

struct childman {
    std::map<pid_t, int> PIDS;
    std::bitset<18> bitmap;
    
    int findfirstunset();
    int findandsetpid();
    void unsetpid(int pid);
    int forkexec(std::string cmd);
    int updatechildcount();
    int waitforanychild();
    int waitforchildpid(int pid);
    int waitforchildpid(int pid, int status);
    void killallchildren();
    void cleanup();
};

char** makeargv(std::string line, int& sizeout);
void freeargv(char** argv, int size);

#endif
