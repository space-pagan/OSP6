#ifndef CLI_HANDLER_H
#define CLI_HANDLER_H

#include <iostream>
#include <sstream>
#include <unistd.h>
#include <string>
#include <string.h>
#include <vector>

int getcliarg(int, char**, char, int&);
bool hascliflag(int, char**, char);
char** makeargv(std::string, int&);
#endif
