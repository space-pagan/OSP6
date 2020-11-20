/*  Author:     Zoya Samsonov
 *  Created:    November 17, 2020
 *  Last Edit:  November 17, 2020
 */

#include <csignal>          //SIGINT, SIGALRM
#include "file_handler.h"   //writeline()
#include "res_handler.h"    //Descriptor struct
#include "util.h"           //range()
#include "log_handler.h"    //self func declarations

Log::Log(std::string logpath, int max, bool verb) { 
    // Constructor for Log object, sets the internal File object, maximum 
    // number of printed lines, and verbose mode
    logfile = File(logpath, APPEND);
    maxlinecount = max;
    verbose = verb;
}

Log::Log(const Log& old) {
    // Copy-constructor for Log
    logfile = old.logfile;
    linecount = old.linecount;
    maxlinecount = old.maxlinecount;
    verbose = old.verbose;
}

void Log::logline(std::string msg, bool force) {
    // writes a line to the log file if the maximum linecount has not been
    // exceeded or if the force flag is set. No parsing is done to confirm
    // that msg is actually only one line
    if (force || this->linecount < this->maxlinecount) {
        this->logfile.writeline(msg);
        this->linecount++;
    }
}

void Log::logNewPID(clk* shclk, int pid, int max_count) {
    // logs the creation of a PID and reports the number created out of 
    // maximum
    if (!this->verbose) return;
    this->logline(shclk->tostring() + ": Created PID " + 
            std::to_string(pid) + " (" + std::to_string(max_count) + "/40)");
}

void Log::logMaxClaim(clk* shclk, Data d) {
    // logs a process' max-claim statement
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
    // helper for logReqGranted, logReqDenied, logReqDeadlock, with added 
    // output if the resource is shareable. Printing of the clock can be 
    // disabled with the printclk flag set to false
    std::string out = printclk ? shclk->tostring() + ": ":ClockPadding(shclk);
    return out + "PID " + std::to_string(d.pid) +
        " requested " + std::to_string(d.resamount) + " of " +
        (shareable ? "shareable resource " : "") +
        "R" + std::to_string(d.resi) + " and was ";
}

void Log::logDeadlockTest(clk* shclk, bool issafe, std::vector<int>& blocked) {
    // logs that the deadlock avoidance algorithm was executed, and the
    // result of said execution. If processes could deadlock (pids listed in
    // vector blocked), log that as well
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
    // logs that a process resource request was granted
    if (!this->verbose) return;
    this->logDeadlockTest(shclk, true, blocked);
    this->logline(logReqBaseMsg(shclk, d, shareable, false) + "granted");
}

void Log::logReqDenied(clk* shclk, Data d, bool shareable) {
    // log that a process resource request was denied due to lack of
    // availability of the resource. The deadlock algorithm was not executed
    this->logline(logReqBaseMsg(shclk, d, shareable, true) + 
        "denied due to lack of availability");
}

void Log::logReqDeadlock(clk* shclk, Data d, bool shareable, std::vector<int>& blocked) {
    // log that a process resource request could result in a deadlock and 
    // that the request was denied
    this->logDeadlockTest(shclk, false, blocked);
    this->logline(logReqBaseMsg(shclk, d, shareable, false) + 
        "denied due to possible deadlock");
}

void Log::logUnblockCheck(clk* shclk, Data d, int blockSize) {
    // log that oss is checking if any processes can be granted resources
    // after a resource release, which would cause them to be unblocked
    // also log the total number of processes in the blocked queue
    if (blockSize) {
        this->logline(shclk->tostring() + ": Checking if any processes can be " +
            "unblocked (Currently " + std::to_string(blockSize) + " blocked)");
    }
}

void Log::logRel(clk* shclk, Data d, int blockSize) {
    // log that a process has releasead a resource
    if (!this->verbose) return;
    this->logline(shclk->tostring() + ": PID " + std::to_string(d.pid) +
        " released " + std::to_string(d.resamount) + " of R" +
        std::to_string(d.resi));
    this->logUnblockCheck(shclk, d, blockSize);
}

void Log::logUnblock(clk* shclk, Data d) {
    // log that a process has become unblocked and its resource request was
    // granted
    this->logline(shclk->tostring() + ": PID " + std::to_string(d.pid) +
        " unblocked and request for " + std::to_string(d.resamount) + " of R" +
        std::to_string(d.resi) + " was granted");
}

void Log::logTerm(clk* shclk, Data d, int blockSize) {
    // log that a process is terminating and releasing all resources
    if (!this->verbose) return;
    this->logline(shclk->tostring() + ": PID " + std::to_string(d.pid) +
        " terminated and released all allocated resources");
    this->logUnblockCheck(shclk, d, blockSize);
}

std::string Log::logExit(clk* shclk, int quittype, int max_count) {
    // log that oss has terminated and the reason why, along with a summary
    // also returns the same message as a string if oss desires to print it
    // to stdout
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
    // log the current resource allocation table
    if (!this->verbose) return;
    char buf[256]; 
    char* pos = buf;
    this->logline("Current System Resources:");
    this->logline(std::string("        ") + "RX* indicates a shared/infinite" +
        " allocation resource, and will not cause deadlock");
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
