#ifndef RES_HANDLER_H
#define RES_HANDLER_H

#define drange { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 }
#define prange { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 }

#include <iostream>
#include <cstring>
#include <stdlib.h>
#include "shm_handler.h"
#include "error_handler.h"

struct Descriptor {
    int avail = 0;
    int claim[18];
    int alloc[18];
    bool shareable = false;

    Descriptor(int instances, bool share) {
        avail = instances;
        shareable = share;
        for (int i : prange) {
            claim[i] = 0;
            alloc[i] = 0;
        }
    }

    Descriptor(const Descriptor& old) {
        avail = old.avail;
        shareable = old.shareable;
        for (int i : prange) {
            claim[i] = old.claim[i];
            alloc[i] = old.alloc[i];
        }
    }
}; 

struct resman {
    Descriptor* desk;
    bool started[18];
    int sysmax[20];

    resman(int& currID) {
        desk = (Descriptor*)shmcreate(sizeof(Descriptor)*20, currID);
        for (int i : drange) {
            desk[i] = Descriptor(1 + rand() % 10, false);
            sysmax[i] = desk[i].avail;
        }
        for (int i : prange) started[i] = false;
    }

    void stateclaim(int PID, int resclaim[20]);
    bool isSafe();
    int allocate(int PID, int deskID);
    int allocate(int PID, int deskID, int instances);
    void release(int PID, int deskID);
    void release(int PID, int deskID, int instances);

    void printAlloc();
};

void resman::stateclaim(int PID, int resclaim[20]) {
    for (int i : drange) {
        this->desk[i].claim[PID] = resclaim[i];
    }
    started[PID] = true;
}

bool resman::isSafe() {
    int currentavail[20];
    for (int i : drange) {
        currentavail[i] = this->desk[i].avail;
    }
    bool running[18];
    for (int PID : prange) {
        running[PID] = this->started[PID];
    }
    while(!0) {
        bool claimsatisfied = false;
        for (int PID : prange) {
            if (running[PID]) {
                claimsatisfied = true;
                for (int deskID : drange) {
                    if (this->desk[deskID].claim[PID] -
                        this->desk[deskID].alloc[PID] > currentavail[deskID])
                        claimsatisfied = false;
                }
                if (claimsatisfied) {
                    for (int deskID : drange)
                        currentavail[deskID] += this->desk[deskID].alloc[PID];
                    running[PID] = false;
                    break;
                }
            }
        }
        if (!claimsatisfied) return false;
        bool nonerunning = true;
        for (int PID : prange) nonerunning &= !running[PID];
        if (nonerunning) return true;
    }
}

int resman::allocate(int PID, int deskID) {
    return this->allocate(PID, deskID, 1);
}

int resman::allocate(int PID, int deskID, int instances) {
    if (this->desk[deskID].alloc[PID] + instances >
        this->desk[deskID].claim[PID]) {
        customerrorquit("PID " + std::to_string(PID) + " requested more than"
            + " initial claim of R" + std::to_string(deskID));
    } 
    if (instances > this->desk[deskID].avail) {
        return 1;
    } else {
        this->desk[deskID].alloc[PID] += instances;
        this->desk[deskID].avail -= instances;
        if (this->isSafe()) {
            return 0;
        } else {
            this->desk[deskID].alloc[PID] -= instances;
            this->desk[deskID].avail += instances;
            return 2;
        }
    }
}

void resman::release(int PID, int deskID) {
    this->release(PID, deskID, 1);
}

void resman::release(int PID, int deskID, int instances) {
    this->desk[deskID].alloc[PID] -= instances;
    this->desk[deskID].avail += instances;
}

void resman::printAlloc() {
    // print header:
    printf("    ");
    for (int j : drange) {
        printf("R%-2d ", j);
    }
    printf("\n");
    printf("A   ");
    for (int j : drange)
        printf("%2d  ", this->desk[j].avail);
    printf("\n");
    for (int PID : prange) {
        printf("P%-2d ", PID);
        for (int deskID : drange)
            printf("%2d  ", this->desk[deskID].alloc[PID]); 
        printf("\n");
    }
}

#endif
