/*  Author:     Zoya Samsonov
 *  Created:    November 17, 2020
 *  Last Edit:  November 17, 2020
 */

#include <csignal>          //SIGINT, SIGALRM
#include "file_handler.h"   //writeline()
#include "res_handler.h"    //Descriptor struct
#include "util.h"           //range()
#include "log_handler.h"    //self func declarations

Log::Log(std::string logpath, int max, bool verb) : 
    // Constructor for Log object, sets the internal File object, maximum 
    // number of printed lines, and verbose mode
    logfile(File(logpath, APPEND)),
    maxlinecount(max),
    verbose(verb)
    {}

void Log::logline(std::string msg, bool force) {
    // writes a line to the log file if the maximum linecount has not been
    // exceeded or if the force flag is set. No parsing is done to confirm
    // that msg is actually only one line
    if (force || this->linecount < this->maxlinecount || 
            this->maxlinecount == -1) {
        this->logfile.writeline(msg);
        this->linecount++;
    }
}

void Log::logNewPID(clk* shclk, int pid, int max_count) {
    // logs the creation of a PID and reports the number created out of 
    // maximum
    if (!this->verbose) return;
    this->logline(shclk->tostring() + ": Created PID " + 
            std::to_string(pid) + " (" + std::to_string(max_count) + "/100)");
}

void Log::logTerm(clk* shclk, Data d, int remain) {
    // log that a process is terminating and releasing all resources
    if (!this->verbose) return;
    this->logline(shclk->tostring() + ": PID " + std::to_string(d.pid) +
        " terminated and released all loaded pages. " + 
        std::to_string(remain) + " children remaining");
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
        out = "oss terminated normally";
    } else {
        out = "oss terminated. Unknown quittype " + std::to_string(quittype);
    }
    std::string sims = " with " + std::to_string(max_count) +
        " process simulations completed";
    this->logline(shclk->tostring() + ": " + out + sims, true);
    return out + " at system time " + shclk->tostring() + sims;
}
