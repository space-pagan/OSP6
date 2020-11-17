/*  Author:     Zoya Samsonov
 *  Created:    November 17, 2020
 *  Last Edit:  November 17, 2020
 */

#include <csignal>
#include "file_handler.h"
#include "sys_clk.h"
#include "res_handler.h"
#include "util.h"
#include "log_handler.h"

static int linecount = 0;
static int logid;
static int maxlinecount;
static bool logVerbose = false;
static std::string logfile;

void createLogFile(std::string runpath, int max, bool verbose) {
    logfile = runpath + "output-" + epochstr() + ".log";
    logid = add_outfile_append(logfile);
    maxlinecount = max;
    logVerbose = verbose;
}

void logline(std::string msg) {
    if (linecount < maxlinecount) {
        writeline(logid, msg);
        linecount++;
    }
}

void loglineForce(std::string msg) {
    writeline(logid, msg);
    linecount++;
}

void logNewPID(clk* shclk, int pid, int max_count) {
    logline(shclk->tostring() + ": Created PID " + 
            std::to_string(pid) + " (" + std::to_string(max_count) + "/40)");
}

void logMaxClaim(clk* shclk, Data d) {
    std::string out = shclk->tostring() + ": PID " + std::to_string(d.pid) +
        " Max Claim: [";
    for (int i : range(19)) {
        out += std::to_string(d.resarray[i]) + ", ";
    }
    out += std::to_string(d.resarray[19]) + "]";
    logline(out);
}

std::string logReqBaseMsg(clk* shclk, Data d) {
    return shclk->tostring() + ": PID " + std::to_string(d.pid) +
        " requested " + std::to_string(d.resamount) + " of R" +
        std::to_string(d.resi) + " and was ";
}

void logReqGranted(clk* shclk, Data d) {
    logline(logReqBaseMsg(shclk, d) + "granted");
}

void logReqDenied(clk* shclk, Data d) {
    logline(logReqBaseMsg(shclk, d) + "denied due to lack of availability");
}

void logReqDeadlock(clk* shclk, Data d) {
    logline(logReqBaseMsg(shclk, d) + "denied due to possible deadlock");
}

void logUnblockCheck(clk* shclk, Data d, int blockSize) {
    if (blockSize) {
        logline(shclk->tostring() + ": Checking if any processes can be " +
            "unblocked (Currently " + std::to_string(blockSize) + " blocked)");
    }
}

void logRel(clk* shclk, Data d, int blockSize) {
    logline(shclk->tostring() + ": PID " + std::to_string(d.pid) +
        " released " + std::to_string(d.resamount) + " of R" +
        std::to_string(d.resi));
    logUnblockCheck(shclk, d, blockSize);
}

void logUnblock(clk* shclk, Data d) {
    logline(shclk->tostring() + ": PID " + std::to_string(d.pid) +
        " unblocked and request for " + std::to_string(d.resamount) + " of R" +
        std::to_string(d.resi) + " was granted");
}

void logTerm(clk* shclk, Data d, int blockSize) {
    logline(shclk->tostring() + ": PID " + std::to_string(d.pid) +
        " terminated and released all allocated resources");
    logUnblockCheck(shclk, d, blockSize);
}

std::string logExit(clk* shclk, int quittype, int max_count) {
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
    loglineForce(shclk->tostring() + ": " + out);
    return out + " at system time " + shclk->tostring();
}

std::string getLogPath() {
    return logfile;
}
