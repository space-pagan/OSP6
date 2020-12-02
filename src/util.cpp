/* Author:      Zoya Samsonov
 * Created:     November 17, 2020
 * Last Edit:   November 17, 2020
 */

#include "util.h"       //self func declarations

range::range(int S, int T, int U) {
    // constructor for range, throws errors if invalid values are used
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

// wraps range constructor to be able to take only one value
range::range(int T) : range(0, T, 1) {} 

// wraps range constructor to be able to take only two values
range::range(int S, int T) : range(S, T, 1) {}

// implements begin state for range iterator
const int* range::begin() {
    return ptr;
}

// implements end condition for range iterator
const int* range::end() {
    return &(this->data[this->size]);
}

roullette::roullette(int s) {
    this->size = s;
    for (int i : range(s))
        this->sum.push_back(
            (i > 0 ? this->sum[i-1] : 0) + (1.0f / (i + 1.0f)));
}

int roullette::rand() {
    int i = 0;
    float r = (float)std::rand() / ((float)RAND_MAX / this->sum.back());
    for (i = 0; i < this->size && r > this->sum[i]; i++);
    return i;
}

// provides a string representation of the Unix Epoch Second (monotonically +)
// which is useful for ensuring a name is never reused
std::string epochstr() {
    return std::to_string(time(0));
}

long floattimetons(float time) {
    // assumes a float represents time in seconds and returns the nanosecond
    // equivalent
    long s = (long)time;
    long n = (long)((time - s) * 1e9);
    return s*1e9 + n;
}
