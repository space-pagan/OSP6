#ifndef SCHEDULING_H
#define SCHEDULING_H

#include <list>
#include <iostream>
#include <stdlib.h>

struct pcb{
    float CPU_TIME;
    float SYS_TIME;
    float BURST_TIME;
    int PID;
    int PRIORITY;
    bool REALTIME;
    int realtimeChance = 1;

    pcb(int& extPID) {
        CPU_TIME = 0;
        SYS_TIME = 0;
        BURST_TIME = 0;
        PID = extPID;
        PRIORITY = 0;
        REALTIME = (rand() % 100 < realtimeChance);
        extPID++;
    }
};

struct mlfq{
    std::list<pcb*> blocked;
    std::list<pcb*> queues[4];
    std::list<pcb*> expired;
    int T1 = 10000;
    int T2 = 20000;
    int T3 = 40000;
    int T4 = 80000;

    bool isEmpty();
    pcb* getFirstProc();
    void moveToNextPriority(pcb* proc);
};

bool mlfq::isEmpty() {
    for (auto q : this->queues)
        if (!q.empty()) return false;
    return true;
}

pcb* mlfq::getFirstProc() {
    for (auto q : this->queues) {
        if (!q.empty()) {
            return q.front();
        }
    }
    return NULL;
}

void mlfq::moveToNextPriority(pcb* proc) {
    int PRI = proc->PRIORITY;
    if (PRI < 3) {
        auto i = this->queues[PRI].begin();
        for (; i != this->queues[PRI].end(); ++i) {
            if ((*i)->PID == proc->PID) break;
        }
        if (i == this->queues[PRI].end() && (*i)->PID != proc->PID) {
            std::cerr << "Cannot find PCB for PID " << proc->PID;
            std::cerr << " in priority queue " << PRI << "\n";
            exit(-1);
        }
        this->queues[PRI+1].push_back(proc);
        this->queues[PRI].erase(i);
    }   
}
#endif
