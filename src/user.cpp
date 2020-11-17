/* Author:      Zoya Samsonov
 * Created:     October 15, 2020
 * Last edit:   November 5, 2020
 */

#include <iostream>          //cout, cerr
#include <unistd.h>          //getpid()
#include <signal.h>          //signal()
#include "shm_handler.h"     //shmlookup(), shmdetach()
#include "error_handler.h"   //setupprefix()
#include "file_handler.h"    //add_outfile_append(), writeline()
#include "sys_clk.h"         //struct clk
#include "res_handler.h"     //Descriptor

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
    int pid = std::stoi(argv[1]);
    // attach shared memory
    clk* shclk = (clk*)shmlookup(0);
    Descriptor* desc = (Descriptor*)shmlookup(2);
    int* sysmax = (int*)shmlookup(3);
    float startTime = shclk->tofloat();
    float nextActionTime = shclk->nextrand(10e6);
    float termActionTime;
    bool maybeTerm = false;
    int reqChance = 60;
    int termChance = 20;

    pcbmsgbuf* buf = new pcbmsgbuf;
    buf->mtype = 1;
    buf->data.pid = pid;
    buf->data.status = CLAIM;
    for (int i = 0; i < 20; i++) {
        buf->data.resarray[i] = rand() % sysmax[i];
    }
    msgsend(1, buf);
    msgreceive(1, pid+2); // sync with oss to make sure that claim is set properly
    shmdetach(sysmax);

    while(!0) {
        if (shclk->tofloat() >= startTime + 1 && !maybeTerm) {
            maybeTerm = true;
            termActionTime = shclk->nextrand(250e6);
        }
        if (maybeTerm && shclk->tofloat() >= termActionTime) {
            if (rand() % termChance == 0) {
                buf->data.status = TERM;
                buf->data.realpid = getpid();
                shmdetach(shclk);
                msgsend(1, buf);
                shmdetach(desc);
                exit(0);
            }
            termActionTime = shclk->nextrand(250e6);
        }
        if (shclk->tofloat() >= nextActionTime) {
            if (rand() % 100 < reqChance) {
                // request a new resource
                int reqi = -1;
                bool requestable = false;
                for (int i = 0; i < 20; i++)
                    if (desc[i].claim[pid] - desc[i].alloc[pid] > 0) requestable = true;
                if (!requestable) {
                    nextActionTime = shclk->nextrand(10e6);
                    continue;
                }
                while (requestable && reqi < 0) {
                    int resi = rand() % 20;
                    if (desc[resi].claim[pid] - desc[resi].alloc[pid] > 0) {
                        reqi = resi;
                    }
                }
                int reqamount = 1 + rand() % (desc[reqi].claim[pid] - desc[reqi].alloc[pid]);
                buf->data.status = REQ;
                buf->data.resi = reqi;
                buf->data.resamount = reqamount;
                msgsend(1, buf);
                msgreceive(1, pid+2); // blocks until acknowledged
            } else {
                int reli = -1;
                bool releaseable = false;
                for (int i = 0; i < 20; i++) {
                    if (desc[i].alloc[pid]) {
                        releaseable = true;
                    }
                }
                if (!releaseable) {
                    nextActionTime = shclk->nextrand(10e6);
                    continue;
                }
                while (releaseable && reli < 0) {
                    int resi = rand() % 20;
                    if (desc[resi].alloc[pid]) reli = resi;
                }
                int relamount = 1 + rand() % (desc[reli].alloc[pid]);
                buf->data.status = REL;
                buf->data.resi = reli;
                buf->data.resamount = relamount;
                msgsend(1, buf);
                msgreceive(1, pid+2); // sync with oss
            }
            nextActionTime = shclk->nextrand(10e6);
        }
    }
}
