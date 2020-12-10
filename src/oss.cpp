/* Author:      Zoya Samsonov
 * Created:     September 9, 2020
 * Last edit:   November 19, 2020
 */

#include <iostream>              //cout
#include <string>                //std::string
#include <cstring>               //strcmp()
#include <unistd.h>              //alarm()
#include <csignal>               //signal()
#include <vector>                //std::vector
#include <list>                  //std::list
#include "cli_handler.h"         //cli arguments and parsing runpath
#include "child_handler.h"       //forking and handling child processes
#include "error_handler.h"       //error reporting
#include "shm_handler.h"         //shared memory and message queues
#include "help_handler.h"        //printhelp()
#include "sys_clk.h"             //struct clk
#include "res_handler.h"         //struct Descriptor, struct resman
#include "mem_handler.h"         //struct memman
#include "log_handler.h"         //logging utilities
#include "util.h"

// variables used in interrupt handling
volatile bool earlyquit = false;
volatile bool handlinginterrupt = false;
volatile int quittype = 0;

// set flags so program can quit gracefully
void signalhandler(int signum) {
    if (signum == SIGALRM || signum == SIGINT) {
        earlyquit = true;
        quittype = signum;
    }
}

void cleanup(clk* shclk, childman &cm, memman &mm) {
    // remove all child processes
    cm.cleanup();
    // release all shared memory created
    shmdetach(shclk);
    ipc_cleanup();
}

void testopts(int argc, char** argv, std::string pref, int optind,
        std::vector<std::string> opts, bool* flags) {
    // test options set by cli handler. Prints help message if help flag is set
    // if any options provided not defined by cli handler, print an error here
    // when this function returns, it is safe to proceed to the main loop
    if (flags[0]) {
        printhelp(pref);
        exit(0);
    }

    if (argc > optind) custerrhelpprompt(
        "Unknown argument '" + std::string(argv[optind]) + "'");

    try {
        int m = std::stoi(opts[0]);
        if (m != 0 && m != 1) custerrhelpprompt(
            "Unknown option for -m '" + opts[0] + "'");
    } catch (const std::invalid_argument& e) {
        custerrhelpprompt("Argument to -m must be an integer. Received '" + 
                opts[0] + "'");
    }
}

void handleReq(clk* shclk, memman& mm, Log& log, Data data, 
        list<request>& io_req, int& pfault_delta, int& pfault_count,
        long& speed_delta, long& speed_count) {
    int page = data.address >> 10;
    log.logline(shclk->tostring() + ": P" + std::to_string(data.pid) + 
            " requested " + (data.status == REQ_READ ? 
                "read of " : "write to ") + "address " +
            std::to_string(data.address));
    int framenum;
    if (mm.check_frame(data.pid, page, framenum)) {
        // page is already loaded into frame, set the dirty bit as needed
        mm.set_dirty(framenum, data.status);
        shclk->inc(10);
        // send ACK
        msgsend(1, data.pid + 2);
        // log action
        log.logline(shclk->tostring() + ": Address " + 
                std::to_string(data.address) + " is in frame " +
                std::to_string(framenum) + (data.status == REQ_READ ? 
                    ", giving data to P" + std::to_string(data.pid) :
                    ", writing data for P" + std::to_string(data.pid)));
    } else {
        // page is not loaded, append request to wait queue
        request r = request(
            data.pid, page, framenum, data.status, shclk->tonano(), 14e6
        );
        // increment page fault counters
        pfault_count++;
        pfault_delta++;
        speed_delta += 14e6;
        speed_count += 14e6;
        log.logline(shclk->tostring() + ": Address " +
                std::to_string(data.address) + " is not in a frame, " +
                "pagefault");
        log.logline(shclk->tostring() + ": Clearing frame " + 
                std::to_string(r.frame) + " and swapping in P" +
                std::to_string(r.proc) + " Page " +
                std::to_string(r.page));
        if (mm.is_dirty(framenum)) {
            r.req_wait += 14e6;
            speed_delta += 14e6;
            speed_count += 14e6;
            log.logline(shclk->tostring() + ": Dirty bit of frame " +
                    std::to_string(r.frame) + " set, adding additional " +
                    "wait time to request");
        }
        io_req.push_back(r);
    }
}

void handleTerm(clk* shclk, childman& cm, memman& mm, Log& log, Data data) {
    // processes a message from a child that it is terminating by releasing
    // all pages loaded by the child, waiting on the child process
    // to terminate, and releasing the PID form the bitmap
    mm.flush_all(data.pid);
    cm.waitforchildpid(data.realpid);
    log.logTerm(shclk, data, cm.PIDS.size());
}

void handleWaiting(clk* shclk, memman& mm, Log& log, list<request>& io_req) {
    auto ptr = io_req.front();
    while(ptr) {
        // iterate over request queue
        auto tmp = ptr;
        ptr = ptr->next;
        request r = tmp->val;
        if (r.req_time + r.req_wait < shclk->tonano()) {
            // if an appropriate amount of time has passed for the request
            // then load the frame
            mm.load_frame(r.frame, r.proc, r.page, r.rw);
            shclk->inc(10);
            // send ACK
            msgsend(1, r.proc + 2);
            // log action
            log.logline(shclk->tostring() + ": Disk I/O for frame " + 
                    std::to_string(r.frame) + " completed," + 
                    (r.rw == REQ_READ ? 
                        " giving data to P" + std::to_string(r.proc) :
                        " writing data for P" + std::to_string(r.proc)));
            io_req.erase(tmp);
        } 
    }
}

void main_loop(std::string runpath, Log& log, std::string  m) {
    int max_count = 0;              //count of children created
    int currID = 0;                 //value of next unused ftok id
    int spawnConst = 500000000;     //the maximum time between new fork calls
    //int reqs = 0;                 // track number of requests for logging
    srand(getpid());                //set random seed;
    // create shared clock
    clk* shclk = (clk*)shmcreate(sizeof(clk), currID);
    float nextSpawnTime = shclk->nextrand(spawnConst);
    shclk->set(nextSpawnTime);
    msgcreate(currID);
    memman mm;
    childman cm;
    pcbmsgbuf* buf = new pcbmsgbuf;

    // statistics variables
    long last_draw = 0;
    int req_count = 0;
    int req_delta = 0;
    int pfault_count = 0;
    int pfault_delta = 0;
    long speed_delta = 0;
    long speed_count = 0;

    list<request> io_requests;

    while (!earlyquit) {
        // if 100 processes have been started and all have terminated, quit
        if (max_count >= 100 && cm.PIDS.size() == 0) earlyquit = true;
        // if it is time to create a new process and 100 have not been
        // created yet, attempt to create a new process
        if (shclk->tofloat() >= nextSpawnTime && max_count < 100) {
            // attempt to fork a child, if concurrency limit allows it
            // forkexec returns -1 on error and virt_pid on success
            int pid = cm.forkexec(runpath + "user " + m );
            if (pid >= 0) {
                // fork succeeded, log
                log.logNewPID(shclk, pid, ++max_count);
                nextSpawnTime = shclk->nextrand(spawnConst);
            }
        }
        buf->mtype = 1; // messages to oss will only be on mtype=1
        // check if message in queue but do not block
        if (msgreceivenw(1, buf)) {
            // process message accordingly
            if (buf->data.status == REQ_READ || buf->data.status == REQ_WRITE){
                req_delta++;
                req_count++;
                handleReq(shclk, mm, log, buf->data, io_requests, 
                        pfault_delta, pfault_count, speed_delta, speed_count);
            } else if (buf->data.status == TERM) {
                handleTerm(shclk, cm, mm, log, buf->data);
                // draw memory map every time a process terminates
                mm.log_mmap(shclk, log, cm.PIDS.size(), max_count, 
                        (float)req_delta / ((shclk->tonano() - last_draw)/1e9),
                        (float)pfault_delta / req_delta,
                        speed_delta / req_delta);
                last_draw = shclk->tonano();
                req_delta = 0;
                pfault_delta = 0;
                speed_delta = 0;
            } else {
                customerrorquit("P" + std::to_string(buf->data.pid) + 
                        " provided unknown status code '" + 
                        std::to_string(buf->data.status) + "'");
            }
        }
        // process requests waiting on I/O
        if (io_requests.size())
            handleWaiting(shclk, mm, log, io_requests);

        // draw memoy map every ~1 seconds
        if (shclk->tonano() % (long)1e9 == 1e9-1 && req_delta) {
            mm.log_mmap(shclk, log, cm.PIDS.size(), max_count,
                    (float)req_delta / ((shclk->tonano() - last_draw)/1e9),
                    (float)pfault_delta / req_delta,
                    speed_delta / req_delta);
            last_draw = shclk->tonano();
            req_delta = 0;
            pfault_delta = 0;
            speed_delta = 0;
        }

        // increment the clock a set amount each loop
        shclk->inc(4e5);
    }
    // print termination summary to stdout, as well as the logfile name where
    // a detailed output of the run can be found
    std::cout << log.logExit(shclk, quittype, max_count - cm.PIDS.size()) << "\n";
    std::string out = "Average Memory IO / sec: " + std::to_string(
            (float)req_count / shclk->tofloat());
    log.logline(out);
    std::cout << out << "\n";
    out = "Average pagefaults / Memory IO: " + std::to_string(
            (float)pfault_count / req_count);
    log.logline(out);
    std::cout << out << "\n";
    out = "Average memory access time: " + std::to_string(
            speed_count / req_count) + "ns";
    log.logline(out);
    std::cout << out << "\n";

    std::cout << "\nFor simulation details, please see " << log.logfile.name;
    std::cout << "\n";
    // close and remove ipc
    cleanup(shclk, cm, mm);
}

int main(int argc, char** argv) {
    // register SIGINT (^C) and SIGALRM with signal handler
    signal(SIGINT, signalhandler);
    signal(SIGALRM, signalhandler);
    // set perror to display the correct program name
    std::string runpath, pref;
    parserunpath(argv, runpath, pref);
    setupprefix(pref);
    if (!pathdepcheck(runpath, "user")) pathdeperror();

    std::vector<std::string> opts{"0"};// -m
    bool flags[2] = {false, false};    // -h
    int optind = getcliarg(argc, argv, "m", "h", opts, flags);
    // create Log object limited to 100k lines
    Log log = Log(runpath + "output-" + epochstr() + ".log", -1, true);
    int max_time = 2;
    // this line will terminate the program if any options are mis-set
    testopts(argc, argv, pref, optind, opts, flags);
    // set up kill timer
    alarm(max_time);
    main_loop(runpath, log, opts[0]);
    // done
    return 0;
}
