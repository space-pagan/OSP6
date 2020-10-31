#ifndef SYS_CLK_H
#define SYS_CLK_H

#include <stdlib.h>             //srand(), rand()
#include <string>               //std::string, to_string()

struct clk{
    long clk_s;
    long clk_n;

    clk() {
        clk_s = 0;
        clk_n = 0;
    }

    float tofloat();
    std::string tostring();
    void set(float time);
    void set(std::string time);
    void inc(long ns);
    void dec(long ns);
    float nextrand(long maxns);
};

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

#endif
