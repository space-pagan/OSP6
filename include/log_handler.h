#ifndef LOG_HANDLER_H
#define LOG_HANDLER_H

#include <string>
#include "sys_clk.h"
#include "res_handler.h"
#include "file_handler.h"

struct Log {
    File logfile;
    int linecount;
    int maxlinecount;
    bool verbose;
    
    Log(std::string logpath, int max, bool verb) { 
        logfile = File(logpath, APPEND);
        maxlinecount = max;
        verbose = verb;
    }

    Log(const Log& old) {
        logfile = old.logfile;
        linecount = old.linecount;
        maxlinecount = old.maxlinecount;
        verbose = old.verbose;
    }

    void logline(std::string msg, bool force=false);
    void logNewPID(clk* shclk, int pid, int max_count);
    void logMaxClaim(clk* shclk, Data d);
    void logReqGranted(clk* shclk, Data d);
    void logReqDenied(clk* shclk, Data d);
    void logReqDeadlock(clk* shclk, Data d);
    void logRel(clk* shclk, Data d, int blockSize);
    void logUnblockCheck(clk* shclk, Data d, int blockSize);
    void logUnblock(clk* shclk, Data d);
    void logTerm(clk* shclk, Data d, int blockSize);
    std::string logExit(clk* shclk, int quittype, int max_count);
};


#endif
