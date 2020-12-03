/* Author:      Zoya Samsonov
 * Created:     September 9, 2020
 * Last edit:   November 19, 2020
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
#include "mem_handler.h"         //struct memman
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

void cleanup(clk* shclk, childman &cm, memman &mm) {
    // remove all child processes
    cm.cleanup();
    // release all shared memory created
    shmdetach(shclk);
    ipc_cleanup();
}

void testopts(int argc, char** argv, std::string pref, int optind, std::vector<std::string> opts, bool* flags) {
    // test options set by cli handler. Prints help message if help flag is set
    // if any options provided not defined by cli handler, print an error here
    // when this function returns, it is safe to proceed to the main loop
    if (flags[0]) {
        printhelp(pref);
        exit(0);
    }

    if (argc > optind) custerrhelpprompt(
        "Unknown argument '" + std::string(argv[optind]) + "'");

    try {
        int m = std::stoi(opts[0]);
        if (m != 0 && m != 1) custerrhelpprompt(
            "Unknown option for -m '" + opts[0] + "'");
    } catch (const std::invalid_argument& e) {
        custerrhelpprompt("Argument to -m must be an integer. Received '" + opts[0] + "'");
    }
}

// void handleReq(clk* shclk, resman& r, Log& log, Data data, 
        // std::list<Data>& blockedRequests, int& reqs) {
    // // processes a message from a child that it wants to request a resource
    // // by running the deadlock algorithm and either acknowledging the child's
    // // request with a zero-length message, or adding the request to the blocked
    // // queue. Appropriately logs the action, and prints the current resource
    // // allocation table every 20 requests
    // reqs++;
    // int allocated = r.allocate(
            // data.pid, data.resi, data.resamount);
    // if (allocated == 0) { // request granted
        // msgsend(1, data.pid+2);
        // log.logReqGranted(
            // shclk, data, 
            // r.desc[data.resi].shareable,
            // r.lastBlockTest);
    // } else if (allocated == 1) { // request denied due to availability
        // blockedRequests.push_back(data);
        // log.logReqDenied(
            // shclk, data, r.desc[data.resi].shareable);
    // } else if (allocated == 2) { // request could cause deadlock, denied
        // blockedRequests.push_back(data);
        // log.logReqDeadlock(
            // shclk, data, 
            // r.desc[data.resi].shareable,
            // r.lastBlockTest);
    // }
    // if (reqs % 20 == 19) {
        // // print the resource allocation table every 20 requests
        // log.logAlloc(r.desc, r.sysmax);
    // }
// }

void handleTerm(clk* shclk, childman &cm, memman &mm, Log& log,
        Data data, std::list<Data>& blockedRequests) {
    // processes a message from a child that it is terminating by releasing
    // all resources allocated to the child, waiting on the child process
    // to terminate, releasing the PID form the bitmap, and checking if any
    // blocked requests can get processed
    mm.flush_all(data.pid);
    cm.waitforchildpid(data.pid);
    log.logTerm(shclk, data, blockedRequests.size());
}

void main_loop(std::string runpath, Log& log, int m) {
    int max_count = 0;    //count of children created
    int currID = 0;       //value of next unused ftok id
    int spawnConst = 500000000; //the maximum time between new fork calls
    //int reqs = 0;         // track number of requests for logging
    srand(getpid());      //set random seed;
    // create shared clock
    clk* shclk = (clk*)shmcreate(sizeof(clk), currID);
    float nextSpawnTime = shclk->nextrand(spawnConst);
    msgcreate(currID);
    memman mm;
    childman cm;
    pcbmsgbuf* buf = new pcbmsgbuf;

    std::list<Data> blockedRequests;

    while (!earlyquit) {
        // if 40 processes have been started and all have terminated, quit
        if (max_count >= 100 && cm.PIDS.size() == 0) earlyquit = true;
        // if it is time to create a new process and 40 have not been
        // created yet, attempt to do so
        if (shclk->tofloat() >= nextSpawnTime && max_count < 100) {
            // attempt to fork a child, if concurrency limit allows it
            int pid = cm.forkexec(runpath + "user ");
            if (pid >= 0) {
                // fork succeeded, log
                log.logNewPID(shclk, pid, ++max_count);
                nextSpawnTime = shclk->nextrand(spawnConst);
            }
        }
        buf->mtype = 1; // messages to oss will only be on mtype=1
        if (msgreceivenw(1, buf)) {
            // process message accordingly
            if (buf->data.status == REQ_READ) {
                
            } else if (buf->data.status == REQ_WRITE) {

            } else if (buf->data.status == TERM) {

            }
        }
        // increment the clock a set amount each loop
        shclk->inc(2e5);
    }
    // print termination summary to stdout, as well as the logfile name where
    // a detailed output of the run can be found
    std::cout << log.logExit(shclk, quittype, max_count) << "\n";
    std::cout << "For simulation details, please see " << log.logfile.name;
    std::cout << "\n";
    // close and remove ipc
    cleanup(shclk, cm, mm);
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

    std::vector<std::string> opts{"0"};// -m
    bool flags[2] = {false, false};    // -h
    int optind = getcliarg(argc, argv, "m", "h", opts, flags);
    // create Log object limited to 100k lines
    Log log = Log(runpath + "output-" + epochstr() + ".log", 100000, flags[1]);
    int max_time = 2;
    // this line will terminate the program if any options are mis-set
    testopts(argc, argv, pref, optind, opts, flags);
    // set up kill timer
    alarm(max_time);
    main_loop(runpath, log, std::stoi(opts[0]));
    // done
    return 0;
}
