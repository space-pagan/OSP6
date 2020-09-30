#ifndef CLI_HANDLER_H
#define CLI_HANDLER_H

#include <iostream>
#include <sstream>
#include <unistd.h>
#include <string>
#include <string.h>
#include <vector>
#include "error_handler.h"

int getcliarg(int, char**, const char*, const char*, int*, bool*);
bool hascliflag(int, char**, char);
char** makeargv(std::string, int&);
#endif
