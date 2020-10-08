#ifndef CHILD_HANDLER_H
#define CHILD_HANDLER_H

char** makeargv(std::string line, int& sizeout);
void freeargv(char** argv, int size);
void forkexec(std::string cmd, int& pr_count);
void forkexec(const char* cmd, int& pr_count);
int updatechildcount(int& pr_count);
int waitforanychild(int& pr_count);
void killallchildren();

#endif
