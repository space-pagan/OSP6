#ifndef RES_HANDLER_H
#define RES_HANDLER_H

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

    Descriptor(int instances, bool share=false);
    Descriptor(const Descriptor& old);
}; 

struct resman {
    Descriptor* desc;
    bool started[18];
    int* sysmax;
    std::bitset<18> bitmap;
    std::vector<int> lastBlockTest;

    resman(int& currID);
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
