 /* Author: Zoya Samsonov
  * Date: October 6, 2020
  */

#include <iostream>              //cout
#include <string>                //string
#include <cstring>               //strcmp()
#include <unistd.h>              //alarm()
#include <csignal>               //signal()
#include <vector>                //vector
#include "cli_handler.h"         //getcliarg()
#include "child_handler.h"       //forkexec(), updatechildcount()
                                 //    waitforanychild(), killallchildren()
#include "error_handler.h"       //setupprefix(), perrquit(), 
                                 //    custerrhelpprompt()
#include "shm_handler.h"         //shmfromfile(), semcreate(), semunlockall()
                                 //    ipc_cleanup()
#include "help_handler.h"        //printhelp()
#include "file_handler.h"        //add_outfile_append(), writeline()

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

// ignore nested interrupts, only need to quit once :)
void earlyquithandler() {
    if (!handlinginterrupt) {
        handlinginterrupt = true;
        killallchildren(); //child_handler.h
        // print message to indicate reason for termination
        if (quittype == SIGINT) {
            std::cerr << "SIGINT received! Terminating...\n";
        } else if (quittype == SIGALRM) {
            std::cerr << "Timeout limit exceeded! Terminating...\n";
        }
    }
}

// tests and modifies parsed cli arguments and flags. If all correct, 
// returns to main(), otherwise exit().
void testopts(int argc, char** argv, int optind, int& conc, \
              const char* logfile, int max_time, bool* flags) {
    // print help message and quit
    if (flags[0]) {
        printhelp(argv[0]);
        exit(0);
    }

    if (!strcmp(logfile, "")) custerrhelpprompt(
            "-l FILE is a required argument!");
    if (argc > optind+1) custerrhelpprompt(
            "Unknown argument '" + std::string(argv[optind]) + "'");
    // inappropriate values for -c, -t
    if (conc < 1) custerrhelpprompt(
            "option -s must be an integer greater than 0");
    if (conc > 20) {
        std::cout << "This system does not allow more than 20 concurrent";
        std::cout << " processes. Option -c has been set to 20.\n";
        conc = 20;
    }
    if (max_time < 1) custerrhelpprompt(
            "option -t must be an integer greater than 0");
}

void main_loop(int conc, const char* logfile) {
    int max_count = 0;    //count of children created
    int conc_count = 0;   //count of currently running children
    int currID = 0;
    int* clk_s = (int*)shmcreate(sizeof(int), currID);
    int* clk_n = (int*)shmcreate(sizeof(int), currID);
    int* shmPID = (int*)shmcreate(sizeof(int), currID);
    int semid = semcreate(1, currID);

    *clk_s = 0;
    *clk_n = 0;
    *shmPID = 0;
    semunlock(semid, 0);

    while (max_count < 100 && *clk_s < 2) {
        // check if interrupt happened
        if (earlyquit) {
            earlyquithandler();
            break; // stop spawning children
        }
        if (conc_count < conc) {
            // "oss_child"
            int zeropad = 9 - std::to_string(*clk_n).size();
            int pid = forkexec("oss_child " + std::to_string(semid), \
                               conc_count);
            std::cout << "Master: Creating new child pid " << pid;
            std::cout << " at my time " << *clk_s << ".";
            while (zeropad--) std::cout << "0";
            std::cout << *clk_n << "\n";
            max_count++;
        }
        // check if any children have exited
        if (*shmPID != 0) {
            int zeropad = 9 - std::to_string(*clk_n).size();
            std::cout << "Master: Child pid " << *shmPID << " is terminating";
            std::cout << " at system clock time " << *clk_s << ".";
            while (zeropad--) std::cout << "0";
            std::cout << *clk_n << "\n";
            waitforanychild(conc_count);
            *shmPID = 0;
            semunlock(semid, 0);
        }
        *clk_n += 300;
        while (*clk_n > 1e9) {
            *clk_n -= 1e9;
            *clk_s += 1;
        }
        //std::cout << "Time: " << *clk_s << "." << *clk_n << "\n";
    }
    // reached maximum total children. Wait for all remaining to quit
    killallchildren();
    while (conc_count > 0) {
        waitforanychild(conc_count);
    }

    std::cout << "Terminated after running " << max_count << " threads...\n";

    // release all shared memory created
    ipc_cleanup();
}

int main(int argc, char **argv) {
    // register SIGINT (^C) and SIGALRM with signal handler
    signal(SIGINT, signalhandler);
    signal(SIGALRM, signalhandler);
    // set perror to display the correct program name
    setupprefix(argv[0]);

    // parse runtime arguments
    std::vector<std::string> opts{"5", "", "20"};
    bool flags[1] = {false};   // only -h flag for this program
    int optind = getcliarg(argc, argv, "clt", "h", opts, flags);

    // local variables
    // parsed variables and flags are stored in opts and flags arrays
    int conc = std::stoi(opts[0]);
    const char* logfile = opts[1].c_str(); // needs to be -l FILE
    int max_time = std::stoi(opts[2]);
    // this line will terminate the program if any options are mis-set
    testopts(argc, argv, optind, conc, logfile,  max_time, flags);
    // set up kill timer
    //alarm(max_time);
    main_loop(conc, logfile);

    return 0;
}
