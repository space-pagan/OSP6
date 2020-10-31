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

void main_loop(int conc, const char* logfile, std::string runpath) {
    int max_count = 0;    //count of children created
    int conc_count = 0;   //count of currently running children
    int currID = 0;       //value of next unused ftok id
    int logid = add_outfile_append((runpath + logfile).c_str()); //log stream id
    int maxBTWs = 2;      //maximum seconds between new procs
    int maxBTWns = 0;     //maximum nanosec between new procs
    float nextSpawnTime = 0;
    // create shared clock
    clk* shclk = (clk*)shmcreate(sizeof(clk), currID);
    // create MLFQ object
    mlfq schedqueue;
    schedqueue.pcbtable = (pcb*)shmcreate(sizeof(pcb)*18, currID);
    msgcreate(currID);
    
    while (max_count < 100 && !earlyquit) {
        // check if any procs have unblocked
        std::cout << "Checking blocked\n";
        pcbmsgbuf* msg = msgrecwithdatanw(2, -1); 
        if (msg->data[2] == 3) {
            std::cout << "Unblocking pcb#" << msg->data[0] << "\n";
            schedqueue.moveToNextPriority(&schedqueue.pcbtable[msg->data[0]]);
        }

        // schedule new proc if correct time
        if (shclk->tofloat() >= nextSpawnTime) {
            std::cout << "Spawning at time " << shclk->tostring() << "\n";
            int pcbnum = schedqueue.addProc();
            if (pcbnum != -1) {
                forkexec(runpath + "user " + std::to_string(pcbnum), conc_count); 
                std::cout << "pcb #" << pcbnum << "\n";
                max_count++;
                msgsendwithdata(2, pcbnum+2, pcbnum, schedqueue.quantuums[schedqueue.pcbtable[pcbnum].PRIORITY], 0); // qid, pcbnum, timeslicens, status
                nextSpawnTime = shclk->nextrand(maxBTWs * 1e9 + maxBTWns);
                std::cout << "Next spawn at " << nextSpawnTime << "\n";
            }
        } else { // schedule top priority proc
            std::cout << "Scheduling first proc\n";
            pcb* proc;
            if ((proc = schedqueue.getFirstProc()) != NULL) {
                if (proc->TIMESLICENS == 0) {
                    msgsendwithdata(2, proc->PCBTABLEPOS+2, proc->PCBTABLEPOS, schedqueue.quantuums[proc->PRIORITY], 0);
                } else {
                    msgsendwithdata(2, proc->PCBTABLEPOS+2, proc->PCBTABLEPOS, proc->TIMESLICENS, 0);
                }
            } else {
                std::cout << "No procs to schedule\n";
            }
        }

        // if proc(s) are scheduled, wait for response 
        if (schedqueue.bitmap != 0) {
            std::cout << "Waiting on message\n";
            msg = msgreceivewithdata(2, -1);
            if (msg->data[2] == 0) {// is terminating
                std::cout << "Child " << msg->data[0] << " terminating\n";
                schedqueue.moveToExpired(&schedqueue.pcbtable[msg->data[0]]);
                schedqueue.printQueues();
                waitforanychild(conc_count);
                shclk->inc(msg->data[1]);
            } else if (msg->data[2] == 1) { // used entire quantuum, running
                std::cout << "Child " << msg->data[0] << " moved to next Q\n";
                schedqueue.moveToNextPriority(&schedqueue.pcbtable[msg->data[0]]);
                schedqueue.printQueues();
                shclk->inc(msg->data[1]);
            } else if (msg->data[2] == 2) { // blocked after using some time
                std::cout << "Child " << msg->data[0] << " blocked\n";
                schedqueue.pcbtable[msg->data[0]].TIMESLICENS = 
                    schedqueue.quantuums[schedqueue.pcbtable[msg->data[0]].PRIORITY] -
                    msg->data[1];
                schedqueue.moveToBlocked(&schedqueue.pcbtable[msg->data[0]]);
                shclk->inc(msg->data[1]);
            }
        }
        shclk->inc(1e9 + (long)rand() % (long)1e9);
    }
    
    while (conc_count > 0) {
        killallchildren();
        updatechildcount(conc_count);
    }

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
    main_loop(conc, logfile, runpath);
    // done
    return 0;
}
