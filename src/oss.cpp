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
#include <bitset>                //std::bitset
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

void cleanup(clk* shclk, int& conc_count) {
    // remove all child processes
    while (conc_count > 0) {
        killallchildren();
        updatechildcount(conc_count);
    }
    // release all shared memory created
    shmdetach(shclk);
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
    int spawnConst = 500000; //the maximum time between new fork calls
    srand(getpid());      //set random seed;
    // create shared clock
    clk* shclk = (clk*)shmcreate(sizeof(clk), currID);
    float nextSpawnTime = shclk->nextrand(spawnConst);
    msgcreate(currID);
    // instantiate resource manager
    resman r(currID);
    int claim[20];
    for (int i : drange) claim[i] = 0;
    claim[0] = std::min(2, r.sysmax[0]);
    r.stateclaim(0, claim);
    for (int i = 0; i < claim[0]; i++) std::cout << r.allocate(0, 0) << "\n";
    std::cout << "\n\n";
    r.printAlloc();

    // while (!earlyquit) {

    // }
    cleanup(shclk, conc_count);
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
