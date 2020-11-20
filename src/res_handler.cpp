/* Author:      Zoya Samsonov
 * Created:     November 16, 2020
 * Last Edit:   November 19, 2020
 */

#include "util.h"           //range()
#include "res_handler.h"    //self func declarations

Descriptor::Descriptor(int instances, bool share) {
    // constructor for resource Descriptor, sets the initial number of
    // available instances, and whether the resource is shareable
    avail = instances;
    shareable = share;
    for (int i : range(18)) {
        claim[i] = 0;
        alloc[i] = 0;
    }
}

Descriptor::Descriptor(const Descriptor& old) {
    // copy-constructor for Descriptor
    avail = old.avail;
    shareable = old.shareable;
    for (int i : range(18)) {
        claim[i] = old.claim[i];
        alloc[i] = old.alloc[i];
    }
}

resman::resman(int& currID) {
    // constructor for resource manager, currID is updated to the next
    // free key_id after resman makes shared memory allocations

    // resource descriptors are in shared memory
    desc = (Descriptor*)shmcreate(sizeof(Descriptor)*20, currID);
    // system resource maximum is in shared memory
    sysmax = (int*)shmcreate(sizeof(int)*20, currID);
    // generate resource availability randomly
    for (int i : range(20)) {
        desc[i] = Descriptor(1 + rand() % 10);
        sysmax[i] = desc[i].avail;
    }
    // ensure that between 3 and 5 resources are always shareable
    int numshare = 0;
    int maxshare = 3 + rand() % 3;
    while (numshare < maxshare) {
        desc[rand() % 20].shareable = true;
        numshare++;
    }
    // set all processes to not started for deadlock algorithm
    for (int i : range(18)) started[i] = false;
}


void resman::stateclaim(int PID, int resclaim[20]) {
    // updates the claim matrix for a specific PID and sets that process to
    // started for the deadlock algorithm
    for (int i : range(20)) {
        this->desc[i].claim[PID] = resclaim[i];
    }
    started[PID] = true;
}

bool resman::isSafe() {
    // deadlock avoidance algorithm, returns true if the current state is 
    // considered safe, otherwise false

    // clear list of possibly blocked PIDs
    this->lastBlockTest.clear();
    // create a local copy of the current system availability for each resource
    int currentavail[20];
    for (int i : range(20)) {
        currentavail[i] = this->desc[i].avail;
    }
    // create a local copy of the current running processes
    bool running[18];
    for (int PID : range(18)) {
        running[PID] = this->started[PID];
    }
    // simulate processes being run until an unsafe state is found or all
    // processes in the simulation have terminated
    while(!0) {
        bool claimsatisfied = false;
        for (int PID : range(18)) {
            // only simulate those processes that have been marked started
            if (running[PID]) {
                claimsatisfied = true;
                for (int descID : range(20)) {
                    // check if any resources would not be able to be allocated
                    // to meet the process' max claim, given the current
                    // system allocation
                    if (this->desc[descID].claim[PID] -
                        this->desc[descID].alloc[PID] > currentavail[descID])
                        claimsatisfied = false;
                }
                if (claimsatisfied) {
                    // PID's max claim is satisfiable. update the local
                    // availability table, set PID to not running
                    // and restart the simulation on all remaining PIDs
                    for (int descID : range(20))
                        currentavail[descID] += this->desc[descID].alloc[PID];
                    running[PID] = false;
                    break;
                }
            }
        }
        // PID's claim could not be satisified. The state is Unsafe
        if (!claimsatisfied) {
            // generate a list of all PIDs that could become deadlocked
            for (int PID : range(18)) {
                if (running[PID] && this->started[PID]) {
                    this->lastBlockTest.push_back(PID);
                }
            }
            return false;
        }
        // state is only safe if no processes remaining running in the sim
        // check and return if so, or continue simulating remaining procs
        bool nonerunning = true;
        for (int PID : range(18)) nonerunning &= !running[PID];
        if (nonerunning) return true;
    }
}

int resman::allocate(int PID, int descID) {
    // alias to allocate only one resource
    return this->allocate(PID, descID, 1);
}

int resman::allocate(int PID, int descID, int instances) {
    // attempt to allocate instances amount of resource descID for PID
    // if the request exceeds PID's max-claim, throw an error
    if (this->desc[descID].alloc[PID] + instances >
        this->desc[descID].claim[PID]) {
        customerrorquit("PID " + std::to_string(PID) + " requested " + std::to_string(instances) + " of R" + std::to_string(descID) + " but has a maximum claim of " + std::to_string(this->desc[descID].claim[PID]));
    } 
    // if the resource is shareable, allocation is automatically granted
    if (this->desc[descID].shareable) {
        this->desc[descID].alloc[PID] += instances;
        return 0;
    }
    // if instances exceeds availability, deny the request, but do not run
    // deadlock avoidance test
    if (instances > this->desc[descID].avail) {
        return 1;
    } else {
        // otherwise, run deadlock avoidance to see if the proceeding state
        // will be safe. If so, allocate the resource and return 0
        this->desc[descID].alloc[PID] += instances;
        this->desc[descID].avail -= instances;
        if (this->isSafe()) {
            return 0;
        } else {
            // otherwise, remove the temporary allocation and return 2
            this->desc[descID].alloc[PID] -= instances;
            this->desc[descID].avail += instances;
            return 2;
        }
    }
}

void resman::release(int PID, int descID) {
    // alias to release one instance of a resource for PID
    this->release(PID, descID, 1);
}

void resman::release(int PID, int descID, int instances) {
    // release instances amount of a resource descID for PID
    this->desc[descID].alloc[PID] -= instances;
    if (!this->desc[descID].shareable) {
        // shareable resource availability is not modified, only update
        // availability for non-shareable resources
        this->desc[descID].avail += instances;
    }
}

int resman::findfirstunset() {
    // returns the first unset bit in a bitmap (used for PID)
    for (int i : range(18)) if (!this->bitmap[i]) return i;
    return -1;
}

void resman::findandsetpid(int& pid) {
    // finds the first unset bit in a bitmap, sets it, and returns the index
    if ((pid = findfirstunset()) != -1) {
        this->bitmap.set(pid);
    }
}

void resman::unsetpid(int pid) {
    // unsets a bit in a bitmap (releases PID back into the pool)
    this->bitmap.reset(pid);
    this->started[pid] = false;
}

