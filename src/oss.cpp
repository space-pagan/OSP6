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
#include <bitset>                //std::bitset
#include "cli_handler.h"         //parserunpath()
#include "child_handler.h"       //forkexec(), updatechildcount()
                                 //    waitforchildpid(), killallchildren()
                                 //    getdeppath()
#include "error_handler.h"       //setupprefix(), perrquit(), 
                                 //    custerrhelpprompt()
#include "shm_handler.h"         //shmcreate(), shmdetach(), msgcreate()
                                 //    msgsend()
#include "help_handler.h"        //printhelp()
#include "file_handler.h"        //add_outfile_append(), writeline()
#include "sys_clk.h"             //struct clk
#include "sched_handler.h"       //mlfq and pcb structs

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

void testexitorsettime(mlfq& schq, clk* shclk, int mc, float nst, int logid) {
    if (schq.isEmpty()) {
        if (mc >= 100) {
            // if no active or blocked procs and no more procs can be
            // spawned end simulation
            earlyquit = true;
            quittype = 0;
        } else if (shclk->tofloat() < nst) {
            long waittime = floattimetonano(nst);
            waittime -= shclk->clk_s * 1e9 + shclk->clk_n;
            shclk->set(nst);
            schq.IDLE_TIME += waittime;
            writeline(logid, shclk->tostring() + ": All queues empty, " +
                "CPU idle for " + std::to_string(waittime) + " ns");
        }
    }
}

void cleanup(mlfq& schq, clk* shclk, int& conc_count) {
    // remove all child processes
    while (conc_count > 0) {
        killallchildren();
        updatechildcount(conc_count);
    }
    // release all shared memory created
    shmdetach(shclk);
    shmdetach(schq.pcbtable);
    ipc_cleanup();
}

void main_loop(std::string logfile, std::string runpath) {
    int max_count = 0;    //count of children created
    int conc_count = 0;   //count of currently running children
    int currID = 0;       //value of next unused ftok id
    int logid = add_outfile_append(runpath + logfile); //log stream id
    int maxBTWs = 2;      //maximum seconds between new procs
    int maxBTWns = 0;     //maximum nanosec between new procs
    srand(getpid());      //set random seed;
    // create shared clock
    clk* shclk = (clk*)shmcreate(sizeof(clk), currID);
    float nextSpawnTime = shclk->nextrand(maxBTWs * 1e9 + maxBTWns);
    // create MLFQ object
    mlfq schq;
    schq.pcbtable = (pcb*)shmcreate(sizeof(pcb)*18, currID);
    msgcreate(currID);
    pcb* proc; // object for the currently handled pcb to reduce code length
    
    while (!earlyquit) {
        testexitorsettime(schq, shclk, max_count, nextSpawnTime, logid);
        // unblock a proc that has indicated that the event it blocked on has
        // occurred and it is ready to be scheduled again
        if (!schq.blocked.empty()) {
            unblockreadyproc(schq, shclk, logid);
        }
        // schedule new proc if correct time and available slot
        // maximum new children to spawn is 100, after that just dispatch
        int pcbnum = -1;
        if ((max_count < 100) && (shclk->tofloat() >= nextSpawnTime) && 
               ((pcbnum = schq.addProc()) != -1)) {
            genproc(schq, shclk, runpath + "user ", pcbnum, 
                conc_count, max_count, logid);
            // update time to attempt to gen next proc
            if (max_count < 100) {
                nextSpawnTime = shclk->nextrand(maxBTWs * 1e9 + maxBTWns);
                long nst_s = (long)nextSpawnTime;
                long nst_n = (long)((nextSpawnTime - nst_s) * 1e9);
                writeline(logid, "\tNext at " + std::to_string(nst_s) + "." +
                    std::to_string(nst_n));
            }
        }
        // try to dispatch the first active (not blocked) process
        if ((proc = schq.getFirstProc()) != NULL) {
            scheduleproc(schq, shclk, proc, logid, conc_count);
        }
        // fast forward time if no active processes. Track idle time
        if (!schq.anyActive()) {
            long waittime = rand() % (long)1e9;
            shclk->inc(waittime);
            schq.IDLE_TIME += waittime;
            writeline(logid, shclk->tostring() + ": CPU was idle for " +
                std::to_string(waittime) + " ns");
        }
    }
    printsummary(schq, shclk, quittype, logfile, logid);
    cleanup(schq, shclk, conc_count);
}

int main(int argc, char** argv) {
    // register SIGINT (^C) and SIGALRM with signal handler
    signal(SIGINT, signalhandler);
    signal(SIGALRM, signalhandler);
    // set perror to display the correct program name
    std::string runpath, pref;
    parserunpath(argv, runpath, pref);
    setupprefix(pref.c_str());
    if (!pathdepcheck(runpath, "user")) pathdeperror();

    std::string logfile = "output-" + epochstr() + ".log";
    int max_time = 3;
    // set up kill timer
    alarm(max_time);
    main_loop(logfile, runpath);
    // done
    return 0;
}
