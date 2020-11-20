/* Author:      Zoya Samsonov
 * Created:     October 15, 2020
 * Last edit:   November 17, 2020
 */

#include <iostream>          //cout, cerr
#include <unistd.h>          //getpid()
#include <signal.h>          //signal()
#include "shm_handler.h"     //shmlookup(), shmdetach()
#include "error_handler.h"   //setupprefix()
#include "file_handler.h"    //add_outfile_append(), writeline()
#include "sys_clk.h"         //struct clk
#include "res_handler.h"     //Descriptor
#include "util.h"            //range()

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
    // save the time the process started
    float startTime = shclk->tofloat();
    // constant parameters
    const long actConstant = 10e6;
    const int reqChance = 55;
    const int termChance = 4;
    // calculate the time of the next random action
    float nextActionTime = shclk->nextrand(actConstant);
    float termActionTime; // holds the time of next random termination check
    bool maybeTerm = false; // status of whether termination checks can occur
    
    // create a message buffer, and send max-claim to oss
    pcbmsgbuf* buf = new pcbmsgbuf;
    buf->mtype = 1;
    buf->data.pid = pid;
    buf->data.status = CLAIM;
    for (int i : range(20)) {
        buf->data.resarray[i] = rand() % sysmax[i] + 1;
    }
    msgsend(1, buf);
    // confirm receipt and sync up
    msgreceive(1, pid+2);
    // system resource no longer needed, detach from memory
    shmdetach(sysmax);

    while(!0) {
        // random termination checks should only occur after 1 second from
        // process startup
        if (shclk->tofloat() >= startTime + 1 && !maybeTerm) {
            // change termination check flag and calculate next check time
            maybeTerm = true;
            termActionTime = shclk->nextrand(250e6);
        }
        if (maybeTerm && shclk->tofloat() >= termActionTime) {
            // if termination check time and check flag is set, randomly 
            // determine if the process will terminate
            if (rand() % termChance == 0) {
                // if yes, send a termination message to oss, detach all memory
                // and exit
                buf->data.status = TERM;
                buf->data.realpid = getpid();
                shmdetach(shclk);
                msgsend(1, buf);
                shmdetach(desc);
                exit(0);
            }
            // otherwise, calculate time for next termination check
            termActionTime = shclk->nextrand(250e6);
        }
        // if random action time
        if (shclk->tofloat() >= nextActionTime) {
            // decide if requesting or releasing resource
            if (rand() % 100 < reqChance) {
                // request a new resource
                int reqi = -1;
                bool requestable = false;
                // check that we have not already filled max-claim
                for (int i : range(20))
                    if (desc[i].claim[pid] - desc[i].alloc[pid] > 0)
                        requestable = true;
                if (!requestable) {
                    // we have, so generate a new time for next random action
                    nextActionTime = shclk->nextrand(actConstant);
                    continue;
                }
                while (requestable && reqi < 0) {
                    // keep randomly selecting a resource until one is found
                    // where max-claim > allocated to this user
                    int resi = rand() % 20;
                    if (desc[resi].claim[pid] - desc[resi].alloc[pid] > 0) {
                        reqi = resi;
                    }
                }
                // randomly generate an amount to request, so that
                // allocated + requested <= max-claim
                int reqamount = 1 + rand() % 
                    (desc[reqi].claim[pid] - desc[reqi].alloc[pid]);
                // prepare request message to oss and wait for an
                // acknowledgement, indicating that the request has been
                // granted
                buf->data.status = REQ;
                buf->data.resi = reqi;
                buf->data.resamount = reqamount;
                msgsend(1, buf);
                msgreceive(1, pid+2); // blocks until acknowledged
            } else {
                // release a resource
                int reli = -1;
                bool releaseable = false;
                // check that there are resources to release
                for (int i : range(20)) {
                    if (desc[i].alloc[pid]) {
                        releaseable = true;
                    }
                }
                if (!releaseable) {
                    // there are not, calculate the next action time
                    nextActionTime = shclk->nextrand(actConstant);
                    continue;
                }
                while (releaseable && reli < 0) {
                    // keep randomly selecting resources until one is found
                    // that has been allocated (so that it can be released)
                    int resi = rand() % 20;
                    if (desc[resi].alloc[pid]) reli = resi;
                }
                // calculate a random amount to release, not to exceed alloc
                int relamount = 1 + rand() % (desc[reli].alloc[pid]);
                // prepare and send release message to oss, and wait
                // for an acknowledgement, as state of alloc table is
                // undefined while oss processes resource release
                buf->data.status = REL;
                buf->data.resi = reli;
                buf->data.resamount = relamount;
                msgsend(1, buf);
                msgreceive(1, pid+2); // sync with oss
            }
            // an action was taken, calculate the time for the next one
            nextActionTime = shclk->nextrand(actConstant);
        }
    }
}
