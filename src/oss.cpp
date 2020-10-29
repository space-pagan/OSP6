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

// ignore nested interrupts, only need to quit once :)
void earlyquithandler() {
    if (!handlinginterrupt) {
        handlinginterrupt = true;
        killallchildren(); //child_handler.h
        // print message to indicate reason for termination
        if (quittype == SIGINT) {
            std::cerr << "SIGINT received! Terminating...\n";
        } else if (quittype == SIGALRM) {
            std::cerr << "Timeout limit exceeded! Terminating...\n";
        }
    }
}

void main_loop(int conc, const char* logfile, const char* runpath) {
    int max_count = 0;    //count of children created
    int conc_count = 0;   //count of currently running children
    int currID = 0;       //value of next unused ftok id
    int currPID = 0;      //value of next unused PID
    int logid = add_outfile_append(logfile); //log stream id
    int maxBTWs = 2;      //maximum seconds between new procs
    int maxBTWns = 0;     //maximum nanosec between new procs
    float nextSpawnTime;
    // create shared clock
    clk* shclk = (clk*)shmcreate(sizeof(clk), currID);
    // create MLFQ object
    mlfq schedqueue;
    schedqueue.pcbtable = (pcb*)shmcreate(sizeof(pcb)*18, currID);

    // placeholder test of mlfq
    schedqueue.addProc();
    schedqueue.addProc();
    schedqueue.addProc();
    schedqueue.addProc();
    schedqueue.moveToNextPriority(&schedqueue.pcbtable[0]);
    schedqueue.moveToNextPriority(&schedqueue.pcbtable[0]);
    schedqueue.moveToNextPriority(schedqueue.getFirstProc());
    schedqueue.moveToBlocked(&schedqueue.pcbtable[3]);
    schedqueue.moveToNextPriority(&schedqueue.pcbtable[2]);
    schedqueue.moveToNextPriority(&schedqueue.pcbtable[2]);
    schedqueue.moveToNextPriority(&schedqueue.pcbtable[2]);
    schedqueue.moveToExpired(&schedqueue.pcbtable[2]);
    schedqueue.printQueues();

    // release all shared memory created
    shmdetach(shclk);
    shmdetach(schedqueue.pcbtable);
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
    main_loop(conc, logfile, runpath.c_str());
    // done
    return 0;
}
