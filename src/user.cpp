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
    srand(getpid());
    // attach shared memory
    clk* shclk = (clk*)shmlookup(0);
    pcb* pcbtable = (pcb*)shmlookup(1);
    int pcbnum = std::stoi(argv[1]);
    pcbmsgbuf* msg = new pcbmsgbuf;

    while(1) {
        // mtype = 1 reserved for messages back to oss
        // mtype = 2 reserved for unblock status messages
        msg->mtype = pcbnum + 3; 
        msgreceive(2, msg); // overwrites previous msg
        if (msg->data[PCBNUM] != pcbnum)
            customerrorquit("pcb#" + std::to_string(pcbnum) +
                "received message intended for pcb#" + 
                std::to_string(msg->data[PCBNUM]));
      
        if (pcbtable[pcbnum].BLOCK_START != 0) {
            pcbtable[pcbnum].BLOCK_TIME += 
                shclk->clk_s * 1e9 + shclk->clk_n -
                pcbtable[pcbnum].BLOCK_START;
            pcbtable[pcbnum].BLOCK_START = 0;
        }
        // prepare to return message
        msg->mtype = 1;
        if (rand() % 10 < 1) { // 1/10 chance to terminate
            // terminating after using a random amount of time
            // generate and send appropriate message
            pcbtable[pcbnum].BURST_TIME = rand() % msg->data[TIMESLICE];
            msg->data[STATUS] = TERM;
            msgsend(2, msg);
            pcbtable[pcbnum].SYS_TIME = 
                pcbtable[pcbnum].BURST_TIME +
                shclk->clk_s * 1e9 + shclk->clk_n -
                pcbtable[pcbnum].INCEPTIME;
            // cleanup and terminate
            shmdetach(shclk);
            shmdetach(pcbtable);
            exit(0);
        } else {
            // not terminating
            int useraction = rand() % 3;
            if (useraction == 0) {
                // use entire quantuum (no need to change data[TIMESLICE])
                // generate and send approprate message
                pcbtable[pcbnum].BURST_TIME = msg->data[TIMESLICE];
                msg->data[STATUS] = RUN;
                msgsend(2, msg);
            } else if (useraction == 1) {
                // use some quantuum and get blocked
                // generate and send appropriate message
                pcbtable[pcbnum].BURST_TIME = rand() % msg->data[TIMESLICE];
                msg->data[STATUS] = BLOCK;
                msgsend(2, msg);
                pcbtable[pcbnum].BLOCK_START = shclk->clk_s * 1e9 + shclk->clk_n;
                // calculate how long to block and then wait
                float wakeuptime = shclk->nextrand(3e9);
                while (shclk->tofloat() < wakeuptime);
                // generate and send unblock message on reserved mtype = 2
                msg->mtype = 2;
                msg->data[STATUS] = UBLOCK;
                msgsend(2, msg);
                // reset mtype
                msg->mtype = pcbnum+3;
            } else if (useraction == 2) {
                // use some quantuum and get pre-empted to the next queue
                // generate and send appropriate message
                pcbtable[pcbnum].BURST_TIME = msg->data[TIMESLICE] / 100 * 
                    (1 + rand() % 99);
                msg->data[STATUS] = PREEMPT;
                msgsend(2, msg);
            }
        }
    }
}
