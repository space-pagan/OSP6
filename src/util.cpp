/* Author:      Zoya Samsonov
 * Created:     November 17, 2020
 * Last Edit:   November 17, 2020
 */

#include "util.h"

range::range(int S, int T, int U) {
    if (T == S) customerrorquit("Integer range (" + 
        std::to_string(S) + ":" + std::to_string(T) + ":" +
        std::to_string(U) + ") invalid, Stop cannot equal Start");
    if (T <= S && U > 0) customerrorquit("Integer range (" + 
        std::to_string(S) + ":" + std::to_string(T) + ":" +
        std::to_string(U) + ") invalid, step size non-negative");
    if (T <= S && U > 0) customerrorquit("Integer range (" + 
        std::to_string(S) + ":" + std::to_string(T) + ":" +
        std::to_string(U) + ") invalid, step size non-negative");
    if (T > S && U < 0) customerrorquit("Integer range (" + 
        std::to_string(S) + ":" + std::to_string(T) + ":" +
        std::to_string(U) + ") invalid, step size negative");
    if (U == 0) customerrorquit("Integer range (" + 
        std::to_string(S) + ":" + std::to_string(T) + ":" +
        std::to_string(U) + ") invalid, step size 0");
    size = std::ceil((float)(T-S)/(float)U);
    data = new int[size];
    int i = 0;
    while (i < size) {
        data[i] = S + i*U;
        i++;
    }
    ptr = data;
}

range::range(int T) : range(0, T, 1) {} 

range::range(int S, int T) : range(S, T, 1) {}

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
