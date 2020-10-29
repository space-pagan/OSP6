/* Author:      Zoya Samsonov
 * Created:     October 29, 2020
 * Last edit:   October 29, 2020 
 */

#include "sched_handler.h"

template<size_t T> int findfirstunset(std::bitset<T> bs) {
    /* This function returns the position of the first unset bit in
     * a std::bitset object of size T, or -1 if all bits are set */
    for (size_t i = 0; i < T; i++) if (!bs[i]) return i;
    return -1;
}

int mlfq::addProc() {
    /* This function instantiates a new pcb if there is an empty slot in
     * the pcbtable bitmap, and returns its position in pcbtable after
     * adding it to pcbtable and Q1. If no slots are available, -1 is
     * returned */
    int pos = findfirstunset(this->bitmap); // get first availabe slot
    if (pos != -1) { // if a slot is available then
        // construct new pcb and copy to pcbtable (shm)
        pcb* newproc = new pcb(this->PID, pos);
        this->pcbtable[pos] = *newproc; // copy-construct to pcbtable
        delete newproc; // garbage collection
        this->queues[0].push_back(&(this->pcbtable[pos])); // add to Q1
        this->bitmap.set(pos); // set bitmap to represent slot in use
    }
    return pos; // return position in pcbtable or -1
}

bool mlfq::isEmpty() {
    /* This function returns whether there are any non-expired pcbs in
     * the mlfq at this time */
    if (!(this->blocked).empty()) return false;
    for (auto q : this->queues) if (!q.empty()) return false;
    return true;
}

pcb* mlfq::getFirstProc() {
    /* This function returns the first non-blocked pcb in the mlfq, or
     * NULL if no such PCB exists*/
    for (auto q : this->queues) if (!q.empty()) return q.front();
    return NULL;
}

std::list<pcb*>::iterator mlfq::findProcInQueue(pcb* proc) {
    /* This function attempts to find the index of a process within
     * the queue assigned in proc->PRIORITY. If the process cannot be found
     * we can assume that memory mis-management has occured, so
     * an error message is thrown and we terminate */
    int PRI = proc->PRIORITY; // shorthand
    std::list<pcb*>::iterator i;
    if (PRI < 0) { // all blocked procs have PRI = -1
        i = this->blocked.begin(); // start search at front of queue
        // search all elements in queue until end or proc is found
        while (i != this->blocked.end() && (*i) != proc) i++;
    } else if (PRI < 4) { // active proc
        i = this->queues[PRI].begin();
        while (i != this->queues[PRI].end() && (*i) != proc) i++;
    } else { // expired proc, shouldn't need to search for this
        i = this->expired.begin();
        while (i != this->expired.end() && (*i) != proc) i++;
    }
    if ((*i) != proc) // proc was not found, throw error and terminate
        customerrorquit(
            "Cannot find PCB for PID " + std::to_string(proc->PID) +\
            " in priority queue " + std::to_string(PRI));
    return i; // return proc index
}

void mlfq::moveToNextPriority(pcb* proc) {
    if (proc->PRIORITY == 3) return; // cannot expire with this method
    auto i = this->findProcInQueue(proc); // get index in queue
    if (proc->PRIORITY < 0) { // blocked, move to Q1
        this->queues[proc->PRIORITY+1].push_back(proc);
        this->blocked.erase(i);
    } else if (proc->PRIORITY < 3) { // active, move to next unless in Q4
        this->queues[proc->PRIORITY+1].push_back(proc);
        this->queues[proc->PRIORITY].erase(i);
    }
    proc->PRIORITY++;
}

void mlfq::moveToBlocked(pcb* proc) { // move an active proc into blocked Q
    auto i = this->findProcInQueue(proc); // get index in current Q
    this->blocked.push_back(proc); // add to blocked Q
    this->queues[proc->PRIORITY].erase(i); // remove from current Q
    proc->PRIORITY = -1; // all blocked procs have PRI = -1
}

void mlfq::moveToExpired(pcb* proc) { // move an active proc to expired list
    auto i = this->findProcInQueue(proc); // get index in current Q
    this->expired.push_back(new pcb(*proc)); // copy-construct to expired Q
    if (proc->PRIORITY < 0) { // currently blocked
        this->blocked.erase(i); // remove from blocked Q
    } else { // currently active
        this->queues[proc->PRIORITY].erase(i); // remove from active Q
    }
    this->bitmap.reset(proc->PCBTABLEPOS); // unset bitmap slot
}

void mlfq::printQueues() { // for debugging
    std::cout << "Bitmap:  " << this->bitmap << "\n";
    std::cout << "Blocked: ";
    for (auto proc : this->blocked) std::cout << "P" << proc->PID << ", ";
    std::cout << "\nQueue 1: ";
    for (auto proc : this->queues[0]) std::cout << "P" << proc->PID << ", ";
    std::cout << "\nQueue 2: ";
    for (auto proc : this->queues[1]) std::cout << "P" << proc->PID << ", ";
    std::cout << "\nQueue 3: ";
    for (auto proc : this->queues[2]) std::cout << "P" << proc->PID << ", ";
    std::cout << "\nQueue 4: ";
    for (auto proc : this->queues[3]) std::cout << "P" << proc->PID << ", ";
    std::cout << "\nExpired: ";
    for (auto proc : this->expired) std::cout << "P" << proc->PID << ", ";
    std::cout << "\n";
}

