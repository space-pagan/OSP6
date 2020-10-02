#ifndef CHILD_HANDLER_H
#define CHILD_HANDLER_H

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include "error_handler.h"

char** makeargv(std::string, int&);
void freeargv(char**, int);
void forkexec(char*, int&);
int updatechildcount(int&);
int waitforanychild(int&);
void killallchildren();
#endif
