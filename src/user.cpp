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
    int* sysmax = (int*)shmlookup(3);

    int claim[20];
    int* alloc = new int[20];

    pcbmsgbuf* buf = new pcbmsgbuf;
    buf->mtype = 1;
    buf->data.pid = pid;
    buf->data.status = CLAIM;
    for (int i = 0; i < 20; i++) {
        claim[i] = rand() % sysmax[i];
        buf->data.resarray[i] = claim[i];
    }
    msgsend(1, buf);
    shmdetach(sysmax);

    float nextActionTime = shclk->nextrand(10000000);
    float startTime = shclk->tofloat();
    float termActionTime;
    bool maybeTerm = false;
    int reqChance = 51;
    while(!0) {
        if (shclk->tonano() >= startTime + 1 && !maybeTerm) {
            maybeTerm = true;
            termActionTime = shclk->nextrand(250000000);
        }
        if (shclk->tofloat() >= termActionTime) {
            if (rand() % 4 == 0) {
                buf->data.status = TERM;
                shmdetach(shclk);
                exit(0);
            }
            termActionTime = shclk->nextrand(250000000);
        }
        if (shclk->tofloat() >= nextActionTime) {
            if (rand() % 100 < reqChance) {
                // request a new resource
                int reqi = -1;
                bool requestable = false;
                for (int i = 0; i < 20; i++)
                    if (claim[i] - alloc[i] > 0) requestable = true;
                if (!requestable) {
                    nextActionTime = shclk->nextrand(10000000);
                    continue;
                }
                while (requestable && reqi < 0) {
                    int resi = rand() % 20;
                    if (claim[resi] - alloc[resi] > 0) {
                        reqi = resi;
                    }
                }
                int reqamount = 1 + rand() % (claim[reqi] - alloc[reqi]);
                buf->data.status = REQ;
                for (int i = 0; i < 20; i++) {
                    buf->data.resarray[i] = (i == reqi) ? reqamount : 0;
                }
                msgsend(1, buf);
                msgreceive(1, 2+pid); // blocks until acknowledged
                alloc[reqi] += reqamount;
            } else {
                int reli = -1;
                bool releaseable = false;
                for (int i = 0; i < 20; i++)
                    if (alloc[i]) releaseable = true;
                if (!releaseable) {
                    nextActionTime = shclk->nextrand(10000000);
                    continue;
                }
                while (releaseable && reli < 0) {
                    int resi = rand() % 20;
                    if (alloc[reli]) reli = resi;
                }
                int relamount = 1 + rand() % (alloc[reli]);
                buf->data.status = REL;
                for (int i = 0; i < 20; i++) {
                    buf->data.resarray[i] = (i == reli) ? relamount : 0;
                }
                msgsend(1, buf);
                alloc[reli] -= relamount;
            }
            nextActionTime = shclk->nextrand(10000000);
        }
    }
}
