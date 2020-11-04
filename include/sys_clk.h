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

#endif
