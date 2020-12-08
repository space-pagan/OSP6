#ifndef LOG_HANDLER_H
#define LOG_HANDLER_H

#include <string>
#include "sys_clk.h"
#include "res_handler.h"
#include "file_handler.h"

struct Log {
    File logfile;
    int linecount = 0;
    int maxlinecount;
    bool verbose;
    
    Log(std::string logpath, int max, bool verb);
    void logline(std::string msg, bool force=false);
    void logNewPID(clk* shclk, int pid, int max_count);
    void logTerm(clk* shclk, Data d, int remain);
    std::string logExit(clk* shclk, int quittype, int max_count);
};


#endif
