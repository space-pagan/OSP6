#ifndef LOG_HANDLER_H
#define LOG_HANDLER_H

#include <string>
#include "sys_clk.h"
#include "res_handler.h"

void createLogFile(std::string runpath, int max, bool verbose);
void logNewPID(clk* shclk, int pid, int max_count);
void logMaxClaim(clk* shclk, Data d);
void logReqGranted(clk* shclk, Data d);
void logReqDenied(clk* shclk, Data d);
void logReqDeadlock(clk* shclk, Data d);
void logRel(clk* shclk, Data d, int blockSize);
void logUnblock(clk* shclk, Data d);
void logTerm(clk* shclk, Data d, int blockSize);
std::string logExit(clk* shclk, int quittype, int max_count);
std::string getLogPath();

#endif
