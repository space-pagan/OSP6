/*  Author:     Zoya Samsonov
 *  Created:    November 17, 2020
 *  Last Edit:  November 17, 2020
 */

#include <csignal>
#include "file_handler.h"
#include "res_handler.h"
#include "util.h"
#include "log_handler.h"

void Log::logline(std::string msg, bool force) {
    if (force || this->linecount < this->maxlinecount) {
        this->logfile.writeline(msg);
        this->linecount++;
    }
}

void Log::logNewPID(clk* shclk, int pid, int max_count) {
    this->logline(shclk->tostring() + ": Created PID " + 
            std::to_string(pid) + " (" + std::to_string(max_count) + "/40)");
}

void Log::logMaxClaim(clk* shclk, Data d) {
    std::string out = shclk->tostring() + ": PID " + std::to_string(d.pid) +
        " Max Claim: [";
    for (int i : range(19)) {
        out += std::to_string(d.resarray[i]) + ", ";
    }
    out += std::to_string(d.resarray[19]) + "]";
    this->logline(out);
}

std::string logReqBaseMsg(clk* shclk, Data d) {
    return shclk->tostring() + ": PID " + std::to_string(d.pid) +
        " requested " + std::to_string(d.resamount) + " of R" +
        std::to_string(d.resi) + " and was ";
}

void Log::logReqGranted(clk* shclk, Data d) {
    this->logline(logReqBaseMsg(shclk, d) + "granted");
}

void Log::logReqDenied(clk* shclk, Data d) {
    this->logline(logReqBaseMsg(shclk, d) + "denied due to lack of availability");
}

void Log::logReqDeadlock(clk* shclk, Data d) {
    this->logline(logReqBaseMsg(shclk, d) + "denied due to possible deadlock");
}

void Log::logUnblockCheck(clk* shclk, Data d, int blockSize) {
    if (blockSize) {
        this->logline(shclk->tostring() + ": Checking if any processes can be " +
            "unblocked (Currently " + std::to_string(blockSize) + " blocked)");
    }
}

void Log::logRel(clk* shclk, Data d, int blockSize) {
    this->logline(shclk->tostring() + ": PID " + std::to_string(d.pid) +
        " released " + std::to_string(d.resamount) + " of R" +
        std::to_string(d.resi));
    this->logUnblockCheck(shclk, d, blockSize);
}

void Log::logUnblock(clk* shclk, Data d) {
    this->logline(shclk->tostring() + ": PID " + std::to_string(d.pid) +
        " unblocked and request for " + std::to_string(d.resamount) + " of R" +
        std::to_string(d.resi) + " was granted");
}

void Log::logTerm(clk* shclk, Data d, int blockSize) {
    this->logline(shclk->tostring() + ": PID " + std::to_string(d.pid) +
        " terminated and released all allocated resources");
    this->logUnblockCheck(shclk, d, blockSize);
}

std::string Log::logExit(clk* shclk, int quittype, int max_count) {
    std::string out;
    if (quittype == SIGINT) {
        out = "oss interrupted and terminated";
    } else if (quittype == SIGALRM) {
        out = "oss reached simulation time limit and terminated";
    } else if (quittype == 0) {
        out = "oss successfully simulated " + std::to_string(max_count) + 
            " processes and terminated normally";
    } else {
        out = "oss terminated. Unknown quittype " + std::to_string(quittype);
    }
    this->logline(shclk->tostring() + ": " + out, true);
    return out + " at system time " + shclk->tostring();
}
