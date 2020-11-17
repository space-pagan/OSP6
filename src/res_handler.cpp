/* Author:      Zoya Samsonov
 * Created:     November 16, 2020
 * Last Edit:   November 16, 2020
 */

#include "res_handler.h"

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
        customerrorquit("PID " + std::to_string(PID) + " requested " + std::to_string(instances) + " of R" + std::to_string(descID) + " but has a maximum claim of " + std::to_string(this->desc[descID].claim[PID]));
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

