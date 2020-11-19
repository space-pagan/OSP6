/* Author:      Zoya Samsonov
 * Created:     September 9, 2020
 * Last edit:   November 05, 2020
 */

#include <iostream>              //cout
#include <string>                //std::string
#include <cstring>               //strcmp()
#include <unistd.h>              //alarm()
#include <csignal>               //signal()
#include <vector>                //std::vector
#include <list>                  //std::list
#include "cli_handler.h"         //cli arguments and parsing runpath
#include "child_handler.h"       //forking and handling child processes
#include "error_handler.h"       //error reporting
#include "shm_handler.h"         //shared memory and message queues
#include "help_handler.h"        //printhelp()
#include "sys_clk.h"             //struct clk
#include "res_handler.h"         //struct Descriptor, struct resman
#include "log_handler.h"         //logging utilities
#include "util.h"

// variables used in interrupt handling
volatile bool earlyquit = false;
volatile bool handlinginterrupt = false;
volatile int quittype = 0;

// set flags so program can quit gracefully
void signalhandler(int signum) {
    if (signum == SIGALRM || signum == SIGINT) {
        earlyquit = true;
        quittype = signum;
    }
}

void cleanup(clk* shclk, resman r, int& conc_count) {
    // remove all child processes
    while (conc_count > 0) {
        killallchildren();
        updatechildcount(conc_count);
    }
    // release all shared memory created
    shmdetach(shclk);
    shmdetach(r.desc);
    shmdetach(r.sysmax);
    ipc_cleanup();
}

void testopts(int argc, char** argv, std::string pref, int optind, bool* flags) {
    if (flags[0]) {
        printhelp(pref);
        exit(0);
    }

    if (argc > optind) custerrhelpprompt(
        "Unknown argument '" + std::string(argv[optind]) + "'");
}

void unblockAfterRelease(clk* shclk, resman& r, std::list<Data>& blocked, Log& log) {
    auto i = blocked.begin();
    while (i != blocked.end()) {
        if (r.allocate((*i).pid, (*i).resi, (*i).resamount) == 0) {
            msgsend(1, (*i).pid+2);
            log.logUnblock(shclk, (*i));
            blocked.erase(i++);
        } else {
            i++;
        }
    }
}

void handleClaim(clk* shclk, resman& r, Log& log, Data data) {
    log.logMaxClaim(shclk, data);
    r.stateclaim(data.pid, data.resarray);
    msgsend(1, data.pid+2);
}

void handleReq(clk* shclk, resman& r, Log& log, Data data, 
        std::list<Data>& blockedRequests, int& reqs) {
    reqs++;
    int allocated = r.allocate(
            data.pid, data.resi, data.resamount);
    if (allocated == 0) {
        msgsend(1, data.pid+2);
        log.logReqGranted(
            shclk, data, 
            r.desc[data.resi].shareable,
            r.lastBlockTest);
    } else if (allocated == 1) {
        blockedRequests.push_back(data);
        log.logReqDenied(
            shclk, data, r.desc[data.resi].shareable);
    } else if (allocated == 2) {
        blockedRequests.push_back(data);
        log.logReqDeadlock(
            shclk, data, 
            r.desc[data.resi].shareable,
            r.lastBlockTest);
    }
    if (reqs % 20 == 19) {
        log.logAlloc(r.desc, r.sysmax);
    }
}

void handleRel(clk* shclk, resman& r, Log& log, Data data,
        std::list<Data>& blockedRequests) {
    r.release(data.pid, data.resi, data.resamount);
    msgsend(1, data.pid+2);
    log.logRel(shclk, data, blockedRequests.size());
    unblockAfterRelease(shclk, r, blockedRequests, log);
}

void handleTerm(clk* shclk, resman& r, Log& log, Data data,
        std::list<Data>& blockedRequests, int& conc_count) {
    for (int i : range(20)) {
        r.release(data.pid, i, r.desc[i].alloc[data.pid]);
    }
    waitforchildpid(data.realpid, conc_count);
    r.unsetpid(data.pid);
    log.logTerm(shclk, data, blockedRequests.size());
    unblockAfterRelease(shclk, r, blockedRequests, log);
}

void main_loop(std::string runpath, Log& log) {
    int max_count = 0;    //count of children created
    int conc_count = 0;   //count of currently running children
    int currID = 0;       //value of next unused ftok id
    int spawnConst = 500000000; //the maximum time between new fork calls
    int pid;              // pid for new child
    int reqs = 0;         // track number of requests for logging
    srand(getpid());      //set random seed;
    // create shared clock
    clk* shclk = (clk*)shmcreate(sizeof(clk), currID);
    float nextSpawnTime = shclk->nextrand(spawnConst);
    msgcreate(currID);
    // instantiate resource manager
    resman r(currID);
    pcbmsgbuf* buf = new pcbmsgbuf;

    std::list<Data> blockedRequests;

    while (!earlyquit) {
        if (max_count >= 40 && conc_count == 0) earlyquit = true;
        if (shclk->tofloat() >= nextSpawnTime && max_count < 40) {
            r.findandsetpid(pid);
            if (pid >= 0) {
                log.logNewPID(shclk, pid, ++max_count);
                forkexec(runpath + "user " + std::to_string(pid), conc_count);
                nextSpawnTime = shclk->nextrand(spawnConst);
            }
        }
        buf->mtype = 1; // set explicitly
        if (msgreceivenw(1, buf)) {
            if (buf->data.status == CLAIM) {
                handleClaim(shclk, r, log, buf->data);
            } else if (buf->data.status == REQ) {
                handleReq(shclk, r, log, buf->data, blockedRequests, reqs);
            } else if (buf->data.status == REL) {
                handleRel(shclk, r, log, buf->data, blockedRequests);
            } else if (buf->data.status == TERM) {
                handleTerm(shclk, r, log, buf->data, blockedRequests, conc_count);
            }
        }
        shclk->inc(2e5);
    }

    std::cout << log.logExit(shclk, quittype, max_count) << "\n";
    std::cout << "For simulation details, please see " << log.logfile.name << "\n";
    cleanup(shclk, r, conc_count);
}

int main(int argc, char** argv) {
    // register SIGINT (^C) and SIGALRM with signal handler
    signal(SIGINT, signalhandler);
    signal(SIGALRM, signalhandler);
    // set perror to display the correct program name
    std::string runpath, pref;
    parserunpath(argv, runpath, pref);
    setupprefix(pref);
    if (!pathdepcheck(runpath, "user")) pathdeperror();

    std::vector<std::string> opts{};    // sacrificial vector
    bool flags[2] = {false, false};    // -h, -v
    int optind = getcliarg(argc, argv, "", "hv", opts, flags);
    // create Log object limited to 100k lines
    Log log = Log(runpath + "output-" + epochstr() + ".log", 100000, flags[1]);
    int max_time = 5;
    // this line will terminate the program if any options are mis-set
    testopts(argc, argv, pref, optind, flags);
    // set up kill timer
    alarm(max_time);
    main_loop(runpath, log);
    // done
    return 0;
}
