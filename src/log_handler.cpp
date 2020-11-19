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
    if (!this->verbose) return;
    this->logline(shclk->tostring() + ": Created PID " + 
            std::to_string(pid) + " (" + std::to_string(max_count) + "/40)");
}

void Log::logMaxClaim(clk* shclk, Data d) {
    if (!this->verbose) return;
    std::string out = shclk->tostring() + ": PID " + std::to_string(d.pid) +
        " Max Claim: [";
    for (int i : range(19)) {
        out += std::to_string(d.resarray[i]) + ", ";
    }
    out += std::to_string(d.resarray[19]) + "]";
    this->logline(out);
}

std::string logReqBaseMsg(clk* shclk, Data d, bool shareable, bool printclk) {
    std::string out = printclk ? shclk->tostring() + ": ":ClockPadding(shclk);
    return out + "PID " + std::to_string(d.pid) +
        " requested " + std::to_string(d.resamount) + " of " +
        (shareable ? "shareable resource " : "") +
        "R" + std::to_string(d.resi) + " and was ";
}

void Log::logDeadlockTest(clk* shclk, bool issafe, std::vector<int>& blocked) {
    std::string out = ClockPadding(shclk);
    this->logline(shclk->tostring() + ": Deadlock avoidance algorithm ran");
    this->logline(out + (issafe ? "Safe":"Unsafe") + 
            " state after granting request");
    if (!issafe) {
        if (blocked.size()-1 > 0) {
            for (int PID : range(blocked.size()-1)) {
                out += "P" + std::to_string(PID) + ", ";
            }
        }
        out += "P" + std::to_string(blocked.back()) + " could deadlock";
        this->logline(out);
    }
}

void Log::logReqGranted(clk* shclk, Data d, bool shareable, std::vector<int>& blocked) {
    if (!this->verbose) return;
    this->logDeadlockTest(shclk, true, blocked);
    this->logline(logReqBaseMsg(shclk, d, shareable, false) + "granted");
}

void Log::logReqDenied(clk* shclk, Data d, bool shareable) {
    this->logline(logReqBaseMsg(shclk, d, shareable, true) + 
        "denied due to lack of availability");
}

void Log::logReqDeadlock(clk* shclk, Data d, bool shareable, std::vector<int>& blocked) {
    this->logDeadlockTest(shclk, false, blocked);
    this->logline(logReqBaseMsg(shclk, d, shareable, false) + 
        "denied due to possible deadlock");
}

void Log::logUnblockCheck(clk* shclk, Data d, int blockSize) {
    if (blockSize) {
        this->logline(shclk->tostring() + ": Checking if any processes can be " +
            "unblocked (Currently " + std::to_string(blockSize) + " blocked)");
    }
}

void Log::logRel(clk* shclk, Data d, int blockSize) {
    if (!this->verbose) return;
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
    if (!this->verbose) return;
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

void Log::logAlloc(Descriptor* desc, int* sysmax) {
    if (!this->verbose) return;
    char buf[256]; 
    char* pos = buf;
    this->logline("Current System Resources:");

    //print Resource column headers
    pos += sprintf(pos, "        ");
    for (int j : range(20)) {
        pos += sprintf(pos, "R%d%c%s ", j, 
                desc[j].shareable ? '*' : ' ', 
                j < 10 ? " " : "");
    }
    this->logline(buf);
    pos = buf;
    memset(buf, 0, 256);

    //print system maximum
    pos += sprintf(pos, "Max     ");
    for (int j : range(20))
        pos += sprintf(pos, "%-2d   ", sysmax[j]);
    this->logline(buf);
    pos = buf;
    memset(buf, 0, 256);

    //print currently available
    pos += sprintf(pos, "Avail   ");
    for (int j : range(20))
        pos += sprintf(pos, "%-2d   ", desc[j].avail);
    this->logline(buf);
    pos = buf;
    memset(buf, 0, 256);

    //print allocated to each process
    for (int PID : range(18)) {
        pos += sprintf(pos, "P%-2d     ", PID);
        for (int descID : range(20))
            pos += sprintf(pos, "%-2d   ", desc[descID].alloc[PID]);
        this->logline(buf);
        pos = buf;
        memset(buf, 0, 256);
    }
}
