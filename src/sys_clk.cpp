/* Author:      Zoya Samsonov
 * Date:        November 3, 2020
 * Last Edit:   November 5, 2020
 */

#include <ctime>
#include <cstring>
#include "util.h"
#include "sys_clk.h"

float clk::tofloat() {
    return this->clk_s + (float)this->clk_n/(float)1e9;
}

std::string clk::tostring() {
    char buf[256];
    sprintf(buf, "%ld.%09ld", this->clk_s, this->clk_n);
    return std::string(buf);
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
    while (this->clk_n >= 1e9) {
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

std::string ClockPadding(clk* shclk) {
    std::string out = "  ";
    for (int i : range(shclk->tostring().size())) out += " ";
    return out;
}
