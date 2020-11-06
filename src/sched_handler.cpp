/* Author:      Zoya Samsonov
 * Created:     October 29, 2020
 * Last edit:   November 05, 2020 
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
        long waittime = 1000 + rand() % 19901;
        shclk->inc(waittime);
        // a message was present, unblock the pcb
        pcb* proc = &schq.pcbtable[msg->data[PCBNUM]];
        writeline(logid, shclk->tostring() + ": Unblocking PID " +
            std::to_string(proc->PID) + " which took " +
            std::to_string(waittime) + " ns");
        schq.moveToNextPriority(proc);
    }
    delete msg;
}

void genproc(mlfq& schq, clk* shclk, std::string userargstr, 
        int pcbnum, int& conc_count, int& max_count, int logid) {
    // sets up and logs the generation of a child process
    pcb* proc = &schq.pcbtable[pcbnum];
    forkexec(userargstr + std::to_string(pcbnum), conc_count);
    proc->INCEPTIME = shclk->clk_s * 1e9 + shclk->clk_n;
    writeline(logid, shclk->tostring() + ": Generating process with PID " +
        std::to_string(proc->PID) + " (" + std::to_string(++max_count) +
        "/100)");
}

void dispatch(mlfq& schq, clk* shclk, pcb* proc, int logid) {
    // generates and sends a dispatch message to the process represented by
    // pcb* proc
    int pcbnum = proc->PCBTABLEPOS;
    pcbmsgbuf* msg = new pcbmsgbuf;
    msg->mtype = pcbnum + 3;
    msg->data[PCBNUM] = pcbnum;
    // the quantuum with which to dispatch the process
    msg->data[TIMESLICE] = schq.quantuums[proc->PRIORITY];
    // log what's happening
    writeline(logid, shclk->tostring() + ": Scheduling PID " +
        std::to_string(proc->PID) + " with quantuum " +
        std::to_string(msg->data[TIMESLICE]) + "ns");
    // actually send the dispatch message
    msgsend(2, msg);
    // incrememnt the clock by a random amount between 100 and 
    // 10000ns to indicate work done to schedule the process
    long worktime = 100 + rand() % 9901;
    shclk->inc(worktime);
    writeline(logid, shclk->tostring() + ": Dispatch took " + 
        std::to_string(worktime) + " ns");
    // prevent memory leak
    delete msg;
}

void handledispatched(mlfq& schq, clk* shclk, pcb* proc, int logid, int& conc_count) {
    // handles the dispatch response from process proc via message with
    // mtype 1
    pcbmsgbuf* msg = new pcbmsgbuf;
    msg->mtype = 1;
    // block until response received
    msgreceive(2, msg);
    // increment the clock by the amount of time used by proc
    shclk->inc(proc->BURST_TIME);
    proc->CPU_TIME += proc->BURST_TIME;
    std::string summary = shclk->tostring() + ": PID " + 
        std::to_string(proc->PID) + " ran for " + 
        std::to_string(proc->BURST_TIME) + " ns";
    // process received information
    if (msg->data[STATUS] == TERM) {
        // process is terminating, move to expired queue and collect
        schq.moveToExpired(proc);
        waitforanychild(conc_count);
        summary += " and terminated";
    } else if (msg->data[STATUS] == RUN) {
        // process used all quantuum, move to next queue
        schq.moveToNextPriority(proc);
        summary += ", using its entire quantuum, and was moved to ";
        summary += "priority queue " + std::to_string(proc->PRIORITY + 1);
    } else if (msg->data[STATUS] == BLOCK) {
        // process blocked on event, move to blocked queue
        schq.moveToBlocked(proc);
        summary += " and was moved to the blocked queue";
    } else if (msg->data[STATUS] == PREEMPT) {
        // process preempted after using some quantuum, mote to next queue
        schq.moveToNextPriority(proc);
        summary += ", was preempted, and was moved to priority queue " +
            std::to_string(proc->PRIORITY + 1);
    }
    // log what occurred
    writeline(logid, summary);
}

void scheduleproc(mlfq& schq, clk* shclk, pcb* proc, int logid, int& conc_count) {
    // publicly visible to oss
    dispatch(schq, shclk, proc, logid);
    handledispatched(schq, shclk, proc, logid, conc_count);
}

void printandlog(std::string summary, int logid) {
    // utility to print a message to stdout and to a file opened for writing
    std::cout << summary << "\n";
    writeline(logid, summary);
}

void printTermReason(mlfq& schq, clk* shclk, int quittype, int logid) {
    // log reason for termination
    std::string summary;
    if (quittype == SIGINT) {
        summary = "Simulation terminated due to SIGINT at system time " + 
            shclk->tostring() + ", having ran " + std::to_string(schq.PID) +
            " processes";
    } else if (quittype == SIGALRM) {
        summary = "Simulation terminated due to reaching end of allotted time";
        summary += " at system time " + shclk->tostring() + ", having ran ";
        summary += std::to_string(schq.PID) + " processes";
    } else if (quittype == 0) {
        summary = "Simulation terminated naturally after running 100";
        summary += " processes at system time " + shclk->tostring();
    }
    printandlog(summary, logid);
}

void printAVGCPU(mlfq& schq, int logid) {
    // Average CPU Times
    long long AVGCPU = 0;
    for (auto proc : schq.expired) {
        AVGCPU += proc->CPU_TIME;
    }
    AVGCPU /= schq.expired.size();
    std::string summary = "Average CPU Utilization per PID (ns): " + 
        std::to_string(AVGCPU);
    printandlog(summary, logid);
}

void printAVGBLK(mlfq& schq, int logid) {
    // Average Blocked Queue Times
    long long AVGBLK = 0;
    for (auto proc : schq.expired) {
        AVGBLK  += proc->BLOCK_TIME;
    }
    AVGBLK /= schq.expired.size();
    std::string summary = "Average time in Blocked Queue per PID (ns): " +
        std::to_string(AVGBLK);
    printandlog(summary, logid);
}

void printAVGSYS(mlfq& schq, int logid) {
    // Average System Times (from incep to term)
    long long AVGSYS = 0;
    for (auto proc : schq.expired) {
        AVGSYS += proc->SYS_TIME;
    }
    AVGSYS /= schq.expired.size();
    std::string summary = "Average total time in system per PID (ns): " +
        std::to_string(AVGSYS);
    printandlog(summary, logid);
}

void printAVGIDLE(mlfq& schq, int logid) {
    // Average Idle CPU
    std::string summary = 
        "Total CPU Idle Time (ns): " + std::to_string(schq.IDLE_TIME);
    printandlog(summary, logid);
}

void printsummary(mlfq& schq, clk* shclk, int quittype,
        std::string logfile, int logid) {
    // PRINT SUMMARY
    printTermReason(schq, shclk, quittype, logid);
    printAVGCPU(schq, logid);
    printAVGBLK(schq, logid);
    printAVGSYS(schq, logid);
    printAVGIDLE(schq, logid);
    std::cout << "For a complete simulation log, please see " << logfile << "\n";
}
