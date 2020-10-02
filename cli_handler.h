#ifndef CLI_HANDLER_H
#define CLI_HANDLER_H

#include <iostream>
#include <unistd.h>
#include <string>
#include <cstring>
#include "error_handler.h"

int getcliarg(int, char**, const char*, const char*, int*, bool*);
bool hascliflag(int, char**, char);
#endif
