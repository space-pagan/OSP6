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
#include "sched_handler.h"   //struct pcb

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
    clk* shclk = (clk*)shmlookup(0);
    //pcb* pcbtable = (pcb*)shmlookup(1);
    int pcbnum = std::stoi(argv[1]);

    while(1) {
        pcbmsgbuf* msg = msgreceivewithdata(2, pcbnum);
        // decide if terminate
        if (rand() % 10 < 1) {
            msgsendwithdata(2, 1, pcbnum, rand() % msg->data[1], 0);
            //shmdetach
            return 0;
        } else {
            if (rand() % 2) { // use entire quantuum
                msgsendwithdata(2, 1, pcbnum, msg->data[1], 1);
            } else { // blocked after some time
                msgsendwithdata(2, 1, pcbnum, rand() % msg->data[1], 2);
                float wakeuptime = shclk->nextrand((int)3e9);
                while (shclk->tofloat() < wakeuptime);
                msgsendwithdata(2, 1, pcbnum, 0, 3);
            }
        }
    }
}
