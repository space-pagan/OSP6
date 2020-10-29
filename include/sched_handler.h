#ifndef SCHED_HANDLER_H
#define SCHED_HANDLER_H

#include <list>
#include <bitset>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include "error_handler.h"

struct pcb{
    /* struct representing a virtual process control block*/
    float CPU_TIME;
    float SYS_TIME;
    float BURST_TIME;
    int PID;
    int PRIORITY;
    int PCBTABLEPOS;

    pcb(int& extPID, int POS) {
        /* constructor: default values and set PID */
        CPU_TIME = 0;
        SYS_TIME = 0;
        BURST_TIME = 0;
        PID = extPID++; // PID must be unique, increment external value
        PRIORITY = 0;
        PCBTABLEPOS = POS;
    }

    pcb(const pcb &old) {
        CPU_TIME = old.CPU_TIME;
        SYS_TIME = old.SYS_TIME;
        BURST_TIME = old.BURST_TIME;
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
    int T1 = 10000; // timeslice for Q1
    int T2 = 20000; // timeslice for Q2
    int T3 = 40000; // timeslice for Q3
    int T4 = 80000; // timeslice for Q4
    int PID = 0; // value of next unused PID

    int addProc();
    bool isEmpty();
    pcb* getFirstProc();
    std::list<pcb*>::iterator findProcInQueue(pcb* proc);
    void moveToNextPriority(pcb* proc);
    void moveToBlocked(pcb* proc);
    void moveToExpired(pcb* proc);
    void printQueues();
};

#endif
