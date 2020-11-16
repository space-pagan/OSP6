#ifndef RES_HANDLER_H
#define RES_HANDLER_H

#define drange { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 }
#define prange { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 }

#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <bitset>
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
    Descriptor* desc;
    bool started[18];
    int* sysmax;
    std::bitset<18> bitmap;

    resman(int& currID) {
        desc = (Descriptor*)shmcreate(sizeof(Descriptor)*20, currID);
        sysmax = (int*)shmcreate(sizeof(int)*20, currID);
        for (int i : drange) {
            desc[i] = Descriptor(1 + rand() % 10, false);
            sysmax[i] = desc[i].avail;
        }
        for (int i : prange) started[i] = false;
    }

    void stateclaim(int PID, int resclaim[20]);
    bool isSafe();
    int allocate(int PID, int descID);
    int allocate(int PID, int descID, int instances);
    void release(int PID, int descID);
    void release(int PID, int descID, int instances);
    int findfirstunset();
    void findandsetpid(int& pid);
    void unsetpid(int pid);

    void printAlloc();
};

void resman::stateclaim(int PID, int resclaim[20]) {
    for (int i : drange) {
        this->desc[i].claim[PID] = resclaim[i];
    }
    started[PID] = true;
}

bool resman::isSafe() {
    int currentavail[20];
    for (int i : drange) {
        currentavail[i] = this->desc[i].avail;
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
                for (int descID : drange) {
                    if (this->desc[descID].claim[PID] -
                        this->desc[descID].alloc[PID] > currentavail[descID])
                        claimsatisfied = false;
                }
                if (claimsatisfied) {
                    for (int descID : drange)
                        currentavail[descID] += this->desc[descID].alloc[PID];
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

int resman::allocate(int PID, int descID) {
    return this->allocate(PID, descID, 1);
}

int resman::allocate(int PID, int descID, int instances) {
    if (this->desc[descID].alloc[PID] + instances >
        this->desc[descID].claim[PID]) {
        customerrorquit("PID " + std::to_string(PID) + " requested more than"
            + " initial claim of R" + std::to_string(descID));
    } 
    if (instances > this->desc[descID].avail) {
        return 1;
    } else {
        this->desc[descID].alloc[PID] += instances;
        this->desc[descID].avail -= instances;
        if (this->isSafe()) {
            return 0;
        } else {
            this->desc[descID].alloc[PID] -= instances;
            this->desc[descID].avail += instances;
            return 2;
        }
    }
}

void resman::release(int PID, int descID) {
    this->release(PID, descID, 1);
}

void resman::release(int PID, int descID, int instances) {
    this->desc[descID].alloc[PID] -= instances;
    this->desc[descID].avail += instances;
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
        printf("%2d  ", this->desc[j].avail);
    printf("\n");
    for (int PID : prange) {
        printf("P%-2d ", PID);
        for (int descID : drange)
            printf("%2d  ", this->desc[descID].alloc[PID]); 
        printf("\n");
    }
}

int resman::findfirstunset() {
    for ( int i : prange ) if (!this->bitmap[i]) return i;
    return -1;
}

void resman::findandsetpid(int& pid) {
    if ((pid = findfirstunset()) != -1) {
        this->bitmap.set(pid);
    }
}

void resman::unsetpid(int pid) {
    this->bitmap.reset(pid);
}

#endif
