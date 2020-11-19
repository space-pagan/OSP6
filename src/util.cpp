/* Author:      Zoya Samsonov
 * Created:     November 17, 2020
 * Last Edit:   November 17, 2020
 */

#include "util.h"

const int* range::begin() {
    return ptr;
}

const int* range::end() {
    return &(this->data[this->size]);
}

std::string epochstr() {
    return std::to_string(time(0));
}

long floattimetons(float time) {
    long s = (long)time;
    long n = (long)((time - s) * 1e9);
    return s*1e9 + n;
}
