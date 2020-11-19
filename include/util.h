#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <cmath>
#include "error_handler.h"

struct range {
    int* data;
    int* ptr;
    int size;
    range(int T);
    range(int S, int T);
    range(int S, int T, int U);
    const int* begin();
    const int* end();
};

std::string epochstr(); // gets the unix epoch as a string
long floattimetons(float time); // convert s.xxxxxx to ns

#endif
