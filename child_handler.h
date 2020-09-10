#ifndef CHILD_HANDLER_H
#define CHILD_HANDLER_H
void forkexec(char*, char**, int&);
void updatechildcount(int&);
void waitforanychild(int&);
#endif
