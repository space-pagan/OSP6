/* Author: Zoya Samsonov
 * Date: October 6, 2020
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
    if (signum == SIGINT) {
        earlyquit = true;
    }
}

int main(int argc, char **argv) {
    // register SIGINT (^C)
    signal(SIGINT, signalhandler);
    // set up perror prefix
    setupprefix(argv[0]);
    // set seed for rand()
    srand(getpid());

    clk* shclk = (clk*)shmlookup(0);
    int* shmPID = (int*)shmlookup(1);

    float stop = shclk->nextrand(1e6);

    while (shclk->tofloat() < stop && !earlyquit);
    msgreceive(2);
    *shmPID = getpid();

    // cleanup
    shmdetach(shclk);
    shmdetach(shmPID);
    return 0;
}
