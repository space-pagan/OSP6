/* Author:      Zoya Samsonov
 * Created:     November 16, 2020
 * Last Edit:   November 16, 2020
 */

#include "util.h"
#include "res_handler.h"

void resman::stateclaim(int PID, int resclaim[20]) {
    for (int i : range(20)) {
        this->desc[i].claim[PID] = resclaim[i];
    }
    started[PID] = true;
}

bool resman::isSafe() {
    this->lastBlockTest.clear();
    int currentavail[20];
    for (int i : range(20)) {
        currentavail[i] = this->desc[i].avail;
    }
    bool running[18];
    for (int PID : range(18)) {
        running[PID] = this->started[PID];
    }
    while(!0) {
        bool claimsatisfied = false;
        for (int PID : range(18)) {
            if (running[PID]) {
                claimsatisfied = true;
                for (int descID : range(20)) {
                    if (this->desc[descID].claim[PID] -
                        this->desc[descID].alloc[PID] > currentavail[descID])
                        claimsatisfied = false;
                }
                if (claimsatisfied) {
                    for (int descID : range(20))
                        currentavail[descID] += this->desc[descID].alloc[PID];
                    running[PID] = false;
                    break;
                }
            }
        }
        if (!claimsatisfied) {
            for (int PID : range(18)) {
                if (running[PID] && this->started[PID]) {
                    // std::cout << PID << " might deadlock\n";
                    this->lastBlockTest.push_back(PID);
                }
            }
            return false;
        }
        bool nonerunning = true;
        for (int PID : range(18)) nonerunning &= !running[PID];
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
    if (this->desc[descID].shareable) {
        this->desc[descID].alloc[PID] += instances;
        return 0;
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
    if (!this->desc[descID].shareable) {
        this->desc[descID].avail += instances;
    }
}

int resman::findfirstunset() {
    for (int i : range(18)) if (!this->bitmap[i]) return i;
    return -1;
}

void resman::findandsetpid(int& pid) {
    if ((pid = findfirstunset()) != -1) {
        this->bitmap.set(pid);
    }
}

void resman::unsetpid(int pid) {
    this->bitmap.reset(pid);
    this->started[pid] = false;
}

