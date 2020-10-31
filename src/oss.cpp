/* Author:      Zoya Samsonov
 * Created:     September 9, 2020
 * Last edit:   October 21, 2020
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

void main_loop(int conc, const char* logfile, std::string runpath) {
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
    pcbmsgbuf* msg = new pcbmsgbuf;
    pcb* proc; // object for the currently handled pcb to reduce code length
    
    while (!earlyquit) {
        if (schq.isEmpty() && max_count >= 100) {
            // if no active or blocked procs and no more procs can be spawned
            // end simulation
            earlyquit = true;
        } else {
            // otherwise, jump clock to next spawn time
            if (shclk->tofloat() < nextSpawnTime) shclk->set(nextSpawnTime);
        }

        int pcbnum = -1;
        if (!schq.blocked.empty()) {
            // check if any procs have unblocked
            // only receive messages with mtype = 2 (reserved for unblock msgs)
            msg->mtype = 2; 
            // do not wait if no messages present in queue
            if (msgreceivenw(2, msg)) {
                // increment clock to represent work done to unblock a process
                // (should be more than regular dispatch increment)
                shclk->inc(1000 + rand() % 19901);
                // a message was present, unblock the pcb
                proc = &schq.pcbtable[msg->data[PCBNUM]];
                writeline(logid, shclk->tostring() + ": Unblocking PID " +
                    std::to_string(proc->PID));
                schq.moveToNextPriority(proc);
            }
        }

        // schedule new proc if correct time and available slot
        // maximum new children to spawn is 100, after that just dispatch
        if ((max_count < 100) && (shclk->tofloat() >= nextSpawnTime) && 
               ((pcbnum = schq.addProc()) != -1)) {
            // slot was available, fork and exec process
            proc = &schq.pcbtable[pcbnum];
            forkexec(
                runpath + "user " + std::to_string(pcbnum), conc_count); 
            writeline(logid, shclk->tostring() + ": Spawning PID " +
                std::to_string(proc->PID) + " (" + std::to_string(++max_count)
                + "/100)");
            // generate appropriate message and send
            msg->mtype = pcbnum + 3;
            msg->data[PCBNUM] = pcbnum;
            msg->data[TIMESLICE] = schq.quantuums[proc->PRIORITY];
            msgsend(2, msg);
            // update time to attempt to spawn next proc
            nextSpawnTime = shclk->nextrand(maxBTWs * 1e9 + maxBTWns);
            writeline(logid, "\tNext spawn at " + 
                std::to_string(nextSpawnTime));
        } else {
            // try to dispatch the first active (not blocked) process
            if ((proc = schq.getFirstProc()) != NULL) {
                // incrememnt the clock by a random amount between 100 and 
                // 10000ns to indicate work done to schedule the process
                shclk->inc(100 + rand() % 9901);
                pcbnum = proc->PCBTABLEPOS;
                // queues are not empty, dispatch a process
                msg->mtype = pcbnum + 3;
                msg->data[PCBNUM] = pcbnum;
                if (proc->TIMESLICENS == 0) {
                    // allowed to use entire quantuum
                    msg->data[TIMESLICE] = schq.quantuums[proc->PRIORITY];
                } else {
                    // some quantuum used before being blocked, only
                    // schedule for remainder of time
                    msg->data[TIMESLICE] = proc->TIMESLICENS;
                }
                writeline(logid, shclk->tostring() + ": Scheduling PID " +
                    std::to_string(proc->PID) + " with quantuum " +
                    std::to_string(msg->data[TIMESLICE]) + "ns");
                msgsend(2, msg);
            }
        }

        // if proc(s) are scheduled, wait for response 
        if (pcbnum != -1) {
            proc = &schq.pcbtable[pcbnum];
            // only receive messages intended for oss
            msg->mtype = 1;
            // wait for the scheduled process if no message
            msgreceive(2, msg);
            shclk->inc(msg->data[TIMESLICE]);
            writeline(logid, shclk->tostring() + ": Message received after " +
                std::to_string(msg->data[TIMESLICE]) + "ns");
            // process received information
            if (msg->data[STATUS] == TERM) {
                writeline(logid, shclk->tostring() + ": PID " + 
                    std::to_string(proc->PID) + " terminating");
                // process is terminating, move to expired queue and collect
                schq.moveToExpired(proc);
                waitforanychild(conc_count);
            } else if (msg->data[STATUS] == RUN) {
                writeline(logid, shclk->tostring() + ": Moving PID " +
                    std::to_string(proc->PID) + " to priority queue " +
                    std::to_string(proc->PRIORITY + 1));
                // used entire quantuum or remaining timeslice,
                // move to next priority queue
                schq.moveToNextPriority(proc);
                // reset to allow entire use of next quantuum
                proc->TIMESLICENS = 0;
            } else if (msg->data[STATUS] == 2) {
                // blocked after using some time, move to blocked queue
                // and set timeslice variable for when it unblocks

                /*
                 * ASK MARK ABOUT HOW TIMESLICE INFO SHOULD BE SAVED
                 */
                
                if (proc->TIMESLICENS == 0) {
                    proc->TIMESLICENS = schq.quantuums[proc->PRIORITY] -
                        msg->data[TIMESLICE];
                } else {
                    proc->TIMESLICENS -= msg->data[TIMESLICE];
                }
                writeline(logid, shclk->tostring() + ": Moving PID " +
                    std::to_string(proc->PID) + " to blocked queue, with " +
                    "remaining quantuum " + std::to_string(proc->TIMESLICENS)
                    + "ns");
                schq.moveToBlocked(proc);
            }
        }
        shclk->inc(rand() % (long)1e9);
    }

    // log reason for termination
    if (quittype == SIGINT) {
        writeline(logid, shclk->tostring() + ": Simulation terminated due to "
            + "SIGINT received");
    } else if (quittype == SIGALRM) {
        writeline(logid, shclk->tostring() + ": Simulation terminated due to "
            + "reaching end of allotted time");
    }
    
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

int main(int argc, char** argv) {
    // register SIGINT (^C) and SIGALRM with signal handler
    signal(SIGINT, signalhandler);
    signal(SIGALRM, signalhandler);
    // set perror to display the correct program name
    std::string runpath, pref;
    parserunpath(argv, runpath, pref);
    setupprefix(pref.c_str());
    if (!pathdepcheck(runpath, "user")) pathdeperror();

    int conc = 18;
    const char* logfile = "output.log"; 
    int max_time = 3;
    // set up kill timer
    alarm(max_time);
    main_loop(conc, logfile, runpath);
    // done
    return 0;
}
