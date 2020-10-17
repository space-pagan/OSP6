/* Author: Zoya Samsonov
 * Date: October 6, 2020
 */

#include <iostream>          //cout, cerr
#include <sstream>           //ostringstream
#include <string>            //string
#include <cstring>           //strlen(), strcmp()
#include <unistd.h>          //getpid()
#include <stdlib.h>          //srand()
#include <signal.h>          //signal()
#include <chrono>            //system_clock::
#include <ctime>             //ctime()
#include "shm_handler.h"     //semlock(), semunlock(), shmlookup()
#include "error_handler.h"   //setupprefix()
#include "file_handler.h"    //add_outfile_append(), writeline()

void signalhandler(int signum) {
    // child does not allocate ipc, can exit safely
    if (signum == SIGINT) exit(-1);
}

int main(int argc, char **argv) {
    // set up signal handling
    signal(SIGINT, signalhandler);

    // set up perror prefix
    setupprefix(argv[0]);
    // set seed for rand()
    srand(getpid());

    int* clk_s = (int*)shmlookup(0);
    int* clk_n = (int*)shmlookup(1);
    int* shmPID = (int*)shmlookup(2);

    int start_s = *clk_s;
    int start_n = *clk_n;

    std::cout << "Started new child pid " << getpid() << " at time ";
    std::cout << start_s << "." << start_n << "\n";

    int stop_n = start_n + rand() % 1000000;
    int stop_s = start_s;
    while (stop_n > 1e9) {
        stop_n -= 1e9;
        stop_s += 1;
    }

    while (*clk_s < stop_s || *clk_n < stop_n);
    while (*shmPID);
    *shmPID = getpid();

    // cleanup
    shmdetach(clk_s);
    shmdetach(clk_n);
    shmdetach(shmPID);
    return 0;
}
