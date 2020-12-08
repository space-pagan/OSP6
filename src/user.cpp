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
    int mode = std::stoi(argv[1]);
    int pid = std::stoi(argv[2]);
    // attach shared memory
    // clk* shclk = (clk*)shmlookup(0);
    // constant parameters
    const int msgqid = 1;
    const int readChance = 55;
    const int termMod = 900 + rand() % 200;
    const int termChance = 100;
    
    // create a message buffer, and fill with default data
    pcbmsgbuf* buf = new pcbmsgbuf;
    buf->mtype = 1;
    buf->data.pid = pid;
    buf->data.realpid = getpid();

    int requests = 0;
    roullette r(32);

    while(!0) {
        if (requests % termMod == termMod - 1) {
            if (rand() % 100 < termChance) {
                // terminate
                buf->data.status = TERM;
                msgsend(msgqid, buf);
                exit(0);
            }
        }
        Status action = (rand() % 100 < readChance ? REQ_READ : REQ_WRITE);
        int page = 0;
        if (mode) {
            // use weighted random
            page = r.rand();
        } else {
            // use random
            page = rand() % 32;
        }
        int address = (page << 10) + rand() % 1024;
        buf->data.address = address;
        buf->data.status = action;
        // send request for read or write on address
        msgsend(msgqid, buf);
        // wait for acknowledgement
        msgreceive(msgqid, pid+2);
        requests++;
    }
}
