#ifndef SYS_CLK_H
#define SYS_CLK_H

#include <string>               //std::string, to_string()

struct clk{
    long clk_s;
    long clk_n;

    clk();
    float tofloat();
    std::string tostring();
    long tonano();
    void set(float time);
    void set(std::string time);
    void inc(long ns);
    void dec(long ns);
    float nextrand(long maxns);
};

std::string ClockPadding(clk* shclk);

#endif
