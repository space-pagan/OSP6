/* Author:      Zoya Samsonov
 * Created:     September 9, 2020
 * Last edit:   November 05, 2020
 */

#include <iostream>              //cout
#include <string>                //std::string
#include <cstring>               //strcmp()
#include <unistd.h>              //alarm()
#include <csignal>               //signal()
#include <vector>                //std::vector
#include <list>                  //std::list
#include "cli_handler.h"         //parserunpath()
#include "child_handler.h"       //forkexec(), updatechildcount()
                                 //    waitforchildpid(), killallchildren()
                                 //    getdeppath()
#include "error_handler.h"       //setupprefix(), perrquit(), 
                                 //    custerrhelpprompt()
#include "shm_handler.h"         //shmcreate(), shmdetach(), msgcreate()
                                 //    msgsend()
#include "help_handler.h"        //printhelp()
#include "file_handler.h"        //add_outfile_append(), writeline()
#include "sys_clk.h"             //struct clk
#include "res_handler.h"         //struct Descriptor, struct resman

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

void cleanup(clk* shclk, resman r, int& conc_count) {
    // remove all child processes
    while (conc_count > 0) {
        killallchildren();
        updatechildcount(conc_count);
    }
    // release all shared memory created
    shmdetach(shclk);
    shmdetach(r.desc);
    shmdetach(r.sysmax);
    ipc_cleanup();
}

void testopts(int argc, char** argv, std::string pref, int optind, bool* flags) {
    if (flags[0]) {
        printhelp(pref);
        exit(0);
    }

    if (argc > optind) custerrhelpprompt(
        "Unknown argument '" + std::string(argv[optind]) + "'");
}

void main_loop(std::string logfile, std::string runpath) {
    int max_count = 0;    //count of children created
    int conc_count = 0;   //count of currently running children
    int currID = 0;       //value of next unused ftok id
    int logid = add_outfile_append(runpath + logfile); //log stream id
    int spawnConst = 500000000; //the maximum time between new fork calls
    int pid;              // pid for new child
    srand(getpid());      //set random seed;
    // create shared clock
    clk* shclk = (clk*)shmcreate(sizeof(clk), currID);
    float nextSpawnTime = shclk->nextrand(spawnConst);
    msgcreate(currID);
    // instantiate resource manager
    resman r(currID);
    pcbmsgbuf* buf = new pcbmsgbuf;

    std::list<Data> blockedRequests;

    while (!earlyquit) {
        if (max_count >= 40 && conc_count == 0) earlyquit = true;
        if (shclk->tofloat() >= nextSpawnTime && max_count < 40) {
            r.findandsetpid(pid);
            if (pid >= 0) {
                std::cout << shclk->tostring() << ": Created PID " << pid << " (" << ++max_count << "/40)\n";
                forkexec(runpath + "user " + std::to_string(pid), conc_count);
                nextSpawnTime = shclk->nextrand(spawnConst);
            }
        }
        if (msgreceivenw(1, buf)) {
            if (buf->data.status == CLAIM) {
                // std::cout << shclk->tostring() << ": Claim stated by PID " << buf->data.pid << "\n";
                r.stateclaim(buf->data.pid, buf->data.resarray);
            } else if (buf->data.status == REQ) {
                int allocated = r.allocate(buf->data.pid, buf->data.resi, buf->data.resamount);
                if (allocated == 0) {
                    msgsend(1, buf->data.pid+2);
                    std::cout << shclk->tostring() << ": PID " << buf->data.pid << " requested " << buf->data.resamount << " of R" << buf->data.resi << " and was granted\n";
                } else if (allocated == 1) {
                    blockedRequests.push_back(buf->data);
                    std::cout << shclk->tostring() << ": PID " << buf->data.pid << " requested " << buf->data.resamount << " of R" << buf->data.resi << " and was denied due to lack of resource availability\n";
                } else if (allocated == 2) {
                    blockedRequests.push_back(buf->data);
                    std::cout << shclk->tostring() << ": PID " << buf->data.pid << " requested " << buf->data.resamount << " of R" << buf->data.resi << " and was denied due to possible deadlock\n";
                }
            } else if (buf->data.status == REL) {
                r.release(buf->data.pid, buf->data.resi, buf->data.resamount);
                std::cout << shclk->tostring() << ": PID " << buf->data.pid << " released " << buf->data.resamount << " of R" << buf->data.resi << "\n";
                if (blockedRequests.size()) {
                    std::cout << shclk->tostring() << ": Checking if any processes can be unblocked (Currently " << blockedRequests.size() << " blocked)\n";
                    auto i = blockedRequests.begin();
                    while (i != blockedRequests.end()) {
                        if (r.allocate((*i).pid, (*i).resi, (*i).resamount) == 0) {
                            msgsend(1, (*i).pid+2);
                            std::cout << shclk->tostring() << ": Unblocking PID " << (*i).pid << " and granting request for " << (*i).resamount << " of R" << (*i).resi << "\n";
                            blockedRequests.erase(i++);
                        } else {
                            i++;
                        }
                    }
                }
            } else if (buf->data.status == TERM) {
                std::cout << shclk->tostring() << ": PID " << buf->data.pid << " terminating\n";
                for (int i : drange) {
                    r.release(buf->data.pid, i, r.desc[i].alloc[buf->data.pid]);
                }
                std::cout << shclk->tostring() << ": Released all resources for PID " << buf->data.pid << "\n";
                waitforchildpid(buf->data.realpid, conc_count);
                r.unsetpid(buf->data.pid);
                if (blockedRequests.size()) {
                    std::cout << shclk->tostring() << ": Checking if any processes can be unblocked (Currently " << blockedRequests.size() << " blocked)\n";
                    auto i = blockedRequests.begin();
                    while (i != blockedRequests.end()) {
                        if (r.allocate((*i).pid, (*i).resi, (*i).resamount) == 0) {
                            std::cout << shclk->tostring() << ": Unblocking PID " << (*i).pid << " and granting request for " << (*i).resamount << " of R" << (*i).resi << "\n";
                            msgsend(1, (*i).pid+2);
                            blockedRequests.erase(i++);
                        } else {
                            i++;
                        }
                    }
                }
            }
        } else {
            // std::cout << shclk->tostring() << ": Waiting\n";
        }
        shclk->inc(1e6);
    }
    std::cout << shclk->tostring() << ": Terminated with " <<  blockedRequests.size() << " blocked processes\n";
    cleanup(shclk, r, conc_count);
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

    std::vector<std::string> opts{};    // sacrificial vector
    bool flags[2] = {false, false};    // -h, -v
    int optind = getcliarg(argc, argv, "", "hv", opts, flags);
    std::string logfile = "output-" + epochstr() + ".log";
    int max_time = 5;
    // this line will terminate the program if any options are mis-set
    testopts(argc, argv, pref, optind, flags);
    // set up kill timer
    alarm(max_time);
    main_loop(logfile, runpath);
    // done
    return 0;
}
