 /* Author: Zoya Samsonov
  * Date: October 6, 2020
  */

#include <iostream>              //cout
#include <string>                //string
#include <unistd.h>              //alarm()
#include <csignal>               //signal()
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
void testopts(int argc, char** argv, int optind, int max, int& conc,\
        int max_time, bool* flags) {
    // print help message and quit
    if (flags[0]) {
        printhelp(argv[0]);
        exit(0);
    }

    // not enough arguments, FILE is missing
    if (argc <= optind) custerrhelpprompt("FILE is required!");
    // too many arguments
    if (argc > optind+1) custerrhelpprompt(
            "Unknown argument '" + std::string(argv[optind]) + "'");
    // inappropriate values for -n, -s, -t
    if (max < 1) custerrhelpprompt(
            "option -n must be an integer greater than 0");
    if (conc < 1) custerrhelpprompt(
            "option -s must be an integer greater than 0");
    if (conc > 20) {
        std::cout << "This system does not allow more than 20 concurrent";
        std::cout << " processes. Option -s has been set to 20.\n";
        conc = 20;
    }
    if (max_time < 1) custerrhelpprompt(
            "option -t must be an integer greater than 0");
}

void main_loop(int max, int conc, char* infile) {
    int max_count = 0;    //count of children created
    int conc_count = 0;   //count of currently running children
    int startid = 0;      //key_id of the first shared memory segment
    int currid = 0;       //key_id of the next available key_id
    int semid;            //semid for ipc mutex locks

    // create shared memory segments for each non-blank line of the file
    // up to max lines
    shmfromfile(infile, currid, max);
    // change max to be the number of lines read, if less than -n argument
    // this is only relevant if -n argument is greater than the number
    // of lines in the file
    max = currid - startid;
    // create semaphores for mutex control of files being written
    semid = semcreate(3, currid);
    // initiate all semaphores to value 1, to allow the first child
    // to lock them
    semunlockall(semid, 3);

    while (max_count++ < max) {
        // check if interrupt happened
        if (earlyquit) {
            earlyquithandler();
            break; // stop spawning children
        }
        while (conc_count >= conc) {
            // don't create any more children if at concurrency limit
            waitforanychild(conc_count);
        }
        // "palin id semid"
        forkexec("palin " + std::to_string(startid+max_count-1) +\
                 " " + std::to_string(semid), conc_count);
        // check if any children have exited
        updatechildcount(conc_count);
    }
    // reached maximum total children. Wait for all remaining to quit
    while (conc_count > 0) {
        // check if interrupt happened
        if (earlyquit) {
            earlyquithandler();
        }
        // wait for all children to terminate
        waitforanychild(conc_count);
    }

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
    int opts[3] = {4, 2, 100}; // defaults for max, conc, max_time
    bool flags[1] = {false};   // only -h flag for this program
    int optind = getcliarg(argc, argv, "nst", "h", opts, flags);

    // local variables
    // parsed variables and flags are stored in opts and flags arrays
    int max = opts[0];
    int conc = opts[1];
    int max_time = opts[2];
    // FILE is the last argument, but testopts() will check that this is
    // actually the case
    char* infile = argv[argc-1];
    // this line will terminate the program if any options are mis-set
    testopts(argc, argv, optind, max, conc, max_time, flags);
    // set up kill timer
    alarm(max_time);
    // internal file id for output.log
    int logid = add_outfile_append("output.log");
    // output.log << "Started job 'FILE' (n=N, s=S, t=T)"
    writeline(logid, std::string("Started job '") + std::string(infile) +\
            std::string("' (n=") + std::to_string(max) +\
            std::string(", s=") + std::to_string(conc) +\
            std::string(", t=") + std::to_string(max_time) +\
            std::string(")"));
    // spawn children, do work
    main_loop(max, conc, infile);
    // output appropriate termination message to log
    if (earlyquit) {
        writeline(logid, "Process terminated before completion");
    } else {
        writeline(logid, "Complete!");
    }

    return 0;
}
