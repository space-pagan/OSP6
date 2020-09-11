#ifndef CHILD_HANDLER_H
#define CHILD_HANDLER_H

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "error_handler.h"

void forkexec(char*, char**, int&);
int updatechildcount(int&);
int waitforanychild(int&);
#endif
