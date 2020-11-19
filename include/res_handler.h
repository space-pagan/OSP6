#ifndef RES_HANDLER_H
#define RES_HANDLER_H

#define drange { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 }
#define prange { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 }

#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <bitset>
#include <vector>
#include "shm_handler.h"
#include "error_handler.h"

struct Descriptor {
    int avail = 0;
    int claim[18];
    int alloc[18];
    bool shareable = false;

    Descriptor(int instances, bool share=false) {
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
    std::vector<int> lastBlockTest;

    resman(int& currID) {
        desc = (Descriptor*)shmcreate(sizeof(Descriptor)*20, currID);
        sysmax = (int*)shmcreate(sizeof(int)*20, currID);
        for (int i : drange) {
            desc[i] = Descriptor(1 + rand() % 10);
            sysmax[i] = desc[i].avail;
        }
        int numshare = 0;
        int maxshare = 3 + rand() % 3;
        while (numshare < maxshare) {
            desc[rand() % 20].shareable = true;
            numshare++;
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
};

#endif
