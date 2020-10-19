#ifndef CHILD_HANDLER_H
#define CHILD_HANDLER_H

char** makeargv(std::string line, int& sizeout);
void freeargv(char** argv, int size);
int forkexec(std::string cmd, int& pr_count);
int forkexec(const char* cmd, int& pr_count);
int updatechildcount(int& pr_count);
int waitforanychild(int& pr_count);
int waitforchildpid(int pid, int& pr_count);
void killallchildren();

#endif
