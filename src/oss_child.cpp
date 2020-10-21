/* Author:      Zoya Samsonov
 * Created:     October 15, 2020
 * Last edit:   October 21, 2020
 */

#include <iostream>          //cout, cerr
#include <unistd.h>          //getpid()
#include <signal.h>          //signal()
#include "shm_handler.h"     //shmlookup(), shmdetach()
#include "error_handler.h"   //setupprefix()
#include "file_handler.h"    //add_outfile_append(), writeline()
#include "sys_clk.h"         //struct clk

volatile bool earlyquit = false;

void signalhandler(int signum) {
    exit(-1); // child does not create any ipc, safe to just kill
}

int main(int argc, char **argv) {
    // register SIGINT (^C)
    signal(SIGINT, signalhandler);
    // set up perror prefix
    setupprefix(argv[0]);
    // set seed for rand()
    srand(getpid());

    // attach to shared memory
    clk* shclk = (clk*)shmlookup(0);
    int* shmPID = (int*)shmlookup(1);
    // calculate a random future time to terminate
    float stop = shclk->nextrand(1e6);
    // busy-wait until stop time, or if SIGINT
    while (shclk->tofloat() < stop);
    // critical section
    msgreceive(2);
    *shmPID = getpid();

    // cleanup
    shmdetach(shclk);
    shmdetach(shmPID);
    return 0;
}
