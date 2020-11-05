#ifndef SCHED_HANDLER_H
#define SCHED_HANDLER_H

#include <list>
#include <bitset>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include "error_handler.h"
#include "sys_clk.h"

struct pcb{
    /* struct representing a virtual process control block*/
    long CPU_TIME;
    long INCEPTIME;
    long SYS_TIME;
    long BURST_TIME;
    long BLOCK_START;
    long BLOCK_TIME;
    int PID;
    int PRIORITY;
    int PCBTABLEPOS;

    pcb(int& extPID, int POS) {
        /* constructor: default values and set PID */
        CPU_TIME = 0;
        INCEPTIME = 0;
        SYS_TIME = 0;
        BURST_TIME = 0;
        BLOCK_START = 0;
        BLOCK_TIME = 0;
        PID = extPID++; // PID must be unique, increment external value
        PRIORITY = 0;
        PCBTABLEPOS = POS;
    }

    pcb(const pcb &old) {
        CPU_TIME = old.CPU_TIME;
        INCEPTIME = old.INCEPTIME;
        SYS_TIME = old.SYS_TIME;
        BURST_TIME = old.BURST_TIME;
        BLOCK_START = old.BLOCK_START;
        BLOCK_TIME = old.BLOCK_TIME;
        PID = old.PID;
        PRIORITY = old.PRIORITY;
        PCBTABLEPOS = old.PCBTABLEPOS;
    }
};

struct mlfq{
    /* struct representing a multilevel feedback queue*/
    pcb* pcbtable; // shm table of current pcbs
    std::bitset<18> bitmap; // bitmap for pcbtable
    std::list<pcb*> blocked; // blocked Q
    std::list<pcb*> queues[4]; // Q1-Q4
    std::list<pcb*> expired; // expired list, not really used
    int quantuums[4] = {10000, 20000, 40000, 80000};
    int PID = 0; // value of next unused PID
    long IDLE_TIME = 0; // time CPU spends with no active proccesses

    int addProc();
    bool isEmpty();
    bool anyActive();
    pcb* getFirstProc();
    std::list<pcb*>::iterator findProcInQueue(pcb* proc);
    void moveToNextPriority(pcb* proc);
    void moveToBlocked(pcb* proc);
    void moveToExpired(pcb* proc);
    void printQueues();
};

void unblockreadyproc(mlfq& schq, clk* shclk, int logid);
void genproc(mlfq& schq, clk* shclk, std::string userargstr, int pcbnum, 
    int& conc_count, int& max_count, int logid);
void scheduleproc(mlfq& schq, clk* shclk, pcb* proc, int logid, int& conc_count);
void printsummary(mlfq& schq, clk* shclk, int quittype, 
    std::string logfile, int logid);

#endif
