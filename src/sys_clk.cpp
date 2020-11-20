/* Author:      Zoya Samsonov
 * Date:        November 3, 2020
 * Last Edit:   November 19, 2020
 */

#include <ctime>
#include <cstring>
#include "util.h"
#include "sys_clk.h"

clk::clk() {
    // constructor for shared system clock
    clk_s = 0;
    clk_n = 0;
}

float clk::tofloat() {
    // returns the current clock time as a float
    return this->clk_s + (float)this->clk_n/(float)1e9;
}

std::string clk::tostring() {
    // returns the current clock time as a pretty-print string
    char buf[256];
    sprintf(buf, "%ld.%09ld", this->clk_s, this->clk_n);
    return std::string(buf);
}

void clk::set(float time) {
    // sets the current clock time to the same value as time
    this->clk_s = (long)time;
    this->clk_n = (long)((time - this->clk_s) * 1e9);
}

void clk::set(std::string time) {
    // sets the current clock time by parsing the string time
    float ftime = std::stof(time);
    this->set(ftime);
}

void clk::inc(long ns) {
    // increments the clock by ns nanoseconds, accounting for seconds-rollover
    this->clk_n += ns;
    while (this->clk_n >= 1e9) {
        this->clk_n -= 1e9;
        this->clk_s += 1;
    }
}

void clk::dec(long ns) {
    // decrements the clock by ns nanoseconds, accounting for seconds-rollover
    this->clk_n -= ns;
    while (this->clk_n < 0) {
        this->clk_n += 1e9;
        this->clk_s -= 1;
    }
    if (this->clk_s < 0) {
        this->clk_n = 0;
        this->clk_s = 0;
    }
}

float clk::nextrand(long maxns) {
    // returns a future random time no more than maxns nanoseconds ahead of now
    clk copy;
    copy.clk_s = this->clk_s;
    copy.clk_n = this->clk_n;
    copy.inc(rand() % maxns);
    return copy.tofloat();
}

long clk::tonano() {
    // returns the current clock time in nanoseconds
    return this->clk_s*1e9 + this->clk_n;
}

std::string ClockPadding(clk* shclk) {
    // returns a string containing only spaces, equal in length to the 
    // length of the current-time pretty-print string, plus 2
    std::string out = "  ";
    for (int i : range(shclk->tostring().size())) out += " ";
    return out;
}
