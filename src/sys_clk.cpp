/* Author:      Zoya Samsonov
 * Date:        November 3, 2020
 * Last Edit:   November 5, 2020
 */

#include <ctime>
#include "sys_clk.h"

float clk::tofloat() {
    return this->clk_s + (float)this->clk_n/(float)1e9;
}

std::string clk::tostring() {
    int zeropad = 9 - std::to_string(this->clk_n).size();
    std::string repr = std::to_string(this->clk_s) + ".";
    while (zeropad--) {
        repr += "0";
    }
    return repr + std::to_string(this->clk_n);
}

void clk::set(float time) {
    this->clk_s = (long)time;
    this->clk_n = (long)((time - this->clk_s) * 1e9);
}

void clk::set(std::string time) {
    float ftime = std::stof(time);
    this->set(ftime);
}

void clk::inc(long ns) {
    this->clk_n += ns;
    while (this->clk_n > 1e9) {
        this->clk_n -= 1e9;
        this->clk_s += 1;
    }
}

void clk::dec(long ns) {
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
    clk copy;
    copy.clk_s = this->clk_s;
    copy.clk_n = this->clk_n;
    copy.inc(rand() % maxns);
    return copy.tofloat();
}

long clk::tonano() {
    return this->clk_s*1e9 + this->clk_n;
}

std::string epochstr() {
    // returns a string containing the unix epoch time for log naming
    return std::to_string(time(0));
}

long floattimetonano(float time) {
    // converts a time float formatted as s.ns to a long formatted as ns
    long s = (long)time;
    long n = (long)((time - s) * 1e9);
    return s*1e9 + n;
}
