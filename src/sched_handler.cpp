/* Author:      Zoya Samsonov
 * Created:     October 29, 2020
 * Last edit:   October 29, 2020 
 */

#include <csignal>
#include "child_handler.h"
#include "file_handler.h"
#include "sys_clk.h"
#include "shm_handler.h"
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

bool mlfq::anyActive() {
    /* This function returns whether there are any processes in the active
     * priority queues at this time */
    for (auto q : this->queues) if (!q.empty()) return true;
    return false;
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
    for (auto proc : this->blocked) std::cout << proc->PID << ", ";
    for (int i = 0; i < 4; i++){
        std::cout << "\nQueue " << i << ": ";
        for (auto proc : this->queues[i]) std::cout << proc->PID << ", ";
    }
    std::cout << "\nExpired: ";
    for (auto proc : this->expired) std::cout << proc->PID << ", ";
    std::cout << "\n";
}

void unblockreadyproc(mlfq& schq, clk* shclk, int logid) {
    // check if any procs have unblocked
    // only receive messages with mtype = 2 (reserved for unblock msgs)
    pcbmsgbuf* msg = new pcbmsgbuf;
    msg->mtype = 2; 
    // do not wait if no messages present in queue
    if (msgreceivenw(2, msg)) {
        // increment clock to represent work done to unblock a process
        // (should be more than regular dispatch increment)
        shclk->inc(1000 + rand() % 19901);
        // a message was present, unblock the pcb
        pcb* proc = &schq.pcbtable[msg->data[PCBNUM]];
        writeline(logid, shclk->tostring() + ": Unblocking PID " +
            std::to_string(proc->PID));
        schq.moveToNextPriority(proc);
    }
    delete msg;
}

void scheduleproc(mlfq& schq, clk* shclk, pcb* proc, int logid, int& conc_count) {
    // incrememnt the clock by a random amount between 100 and 
    // 10000ns to indicate work done to schedule the process
    shclk->inc(100 + rand() % 9901);
    int pcbnum = proc->PCBTABLEPOS;
    // queues are not empty, dispatch a process
    pcbmsgbuf* msg = new pcbmsgbuf;
    msg->mtype = pcbnum + 3;
    msg->data[PCBNUM] = pcbnum;
    msg->data[TIMESLICE] = schq.quantuums[proc->PRIORITY];
    writeline(logid, shclk->tostring() + ": Scheduling PID " +
        std::to_string(proc->PID) + " with quantuum " +
        std::to_string(msg->data[TIMESLICE]) + "ns");
    msgsend(2, msg);

    // wait for response from scheduled proc
    // only receive messages intended for oss
    msg->mtype = 1;
    // block until response received
    msgreceive(2, msg);
    shclk->inc(proc->BURST_TIME);
    proc->CPU_TIME += proc->BURST_TIME;
    writeline(logid, shclk->tostring() + ": Message received after " +
        std::to_string(proc->BURST_TIME) + "ns");
    // process received information
    if (msg->data[STATUS] == TERM) {
        writeline(logid, shclk->tostring() + ": PID " + 
            std::to_string(proc->PID) + " terminating");
        // process is terminating, move to expired queue and collect
        schq.moveToExpired(proc);
        waitforanychild(conc_count);
    } else if (msg->data[STATUS] == RUN) {
        writeline(logid, shclk->tostring() + ": Moving PID " +
            std::to_string(proc->PID) + " to priority queue " +
            std::to_string(proc->PRIORITY + 1));
        schq.moveToNextPriority(proc);
    } else if (msg->data[STATUS] == BLOCK) {
        writeline(logid, shclk->tostring() + ": Moving PID " +
            std::to_string(proc->PID) + " to blocked queue");
        schq.moveToBlocked(proc);
    } else if (msg->data[STATUS] == PREEMPT) {
        writeline(logid, shclk->tostring() + ": PID " +
            std::to_string(proc->PID) + " preempted and moved to priority" +
            " queue " + std::to_string(proc->PRIORITY + 1));
        schq.moveToNextPriority(proc);
    }
    delete msg;
}

void printsummary(mlfq& schq, clk* shclk, int quittype,
        std::string logfile, int logid) {
    // PRINT SUMMARY
    // log reason for termination
    std::string summary;
    if (quittype == SIGINT) {
        summary = shclk->tostring() + ": Simulation terminated due to "
            + "SIGINT received";
    } else if (quittype == SIGALRM) {
        summary = shclk->tostring() + ": Simulation terminated due to "
            + "reaching end of allotted time";
    } else if (quittype == 0) {
        summary = shclk->tostring() + ": Simulation terminated after "
            + "100 processes were created and naturally terminated";
    }
    std::cout << summary << "\n";
    writeline(logid, summary);

    // Average CPU Times
    long long AVGCPU = 0;
    for (auto proc : schq.expired) {
        AVGCPU += proc->CPU_TIME;
    }
    AVGCPU /= schq.expired.size();
    summary = "Average CPU Utilization per PID (ns): " + 
        std::to_string(AVGCPU);
    std::cout << summary << "\n";
    writeline(logid, summary);

    // Average Blocked Queue Times
    long long AVGBLK = 0;
    for (auto proc : schq.expired) {
        AVGBLK  += proc->BLOCK_TIME;
    }
    AVGBLK /= schq.expired.size();
    summary = "Average time in Blocked Queue per PID (ns): " +
        std::to_string(AVGBLK);
    std::cout << summary << "\n";
    writeline(logid, summary);

    // Average Idle CPU
    summary = "Total CPU Idle Time (ns): " + std::to_string(schq.IDLE_TIME);
    std::cout << summary << "\n";
    writeline(logid, summary);

    std::cout << "For a complete simulation log, please see " << logfile << "\n";
}
