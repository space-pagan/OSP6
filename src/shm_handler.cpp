/* Author:      Zoya Samsonov
 * Created:     October 6, 2020
 * Last edit:   November 19, 2020
 */

#include <sys/types.h>          //key_t
#include <sys/ipc.h>            //ftok(), IPC_CREAT, IPC_EXCL
#include <sys/shm.h>            //shmget(), shmat(), shmdt(), shmctl()
#include <sys/sem.h>            //semget(), semop(), semctl()
#include <sys/msg.h>            //msgget(), msgsnd(), msgrcv(), msgctl()
#include <string>               //string
#include <cstring>              //strcpy()
#include <set>                  //set
#include "error_handler.h"      //perrandquit()
#include "file_handler.h"       //add_infile(), readline()
#include "util.h"
#include "shm_handler.h"        //Self func defs

// store references to all created shared memory objects for easier cleanup
static std::set<int> shmsegments;
static std::set<int> semaphores;
static std::set<int> msgqueues;

struct msgbuffer {
    // used for 0-length messages
    long mtype;
};

key_t getkeyfromid(int key_id) {
    // returns a valid key for allocating shared objects
    // provides no guarantee that the key is unused
    key_t key = ftok(".", key_id);
    if (key == -1) perrandquit();
    return key;
}

int shmlookupid(int key_id) {
    // retrieves an existing shared memory id, given a key_id
    // if no shared memory segment has been created with key_id, this will
    // fail and terminate.
    int shmid = shmget(getkeyfromid(key_id), 0, 0);
    if (shmid == -1) perrandquit();
    return shmid;
}

void* shmcreate(size_t bytes, int& key_id) {
    // attempts to create a new shared memory segment of size bytes
    // associated to key_id. external key_id is incremented, so that one
    // variable can be used for all create(key_id) calls without risking
    // selecting a key that is already in use

    // segment is created with RW for user and group (all children)
    int shmid = shmget(getkeyfromid(key_id++), bytes, IPC_CREAT|IPC_EXCL|0660);
    // desired segment could not be created, throw error
    if (shmid == -1) perrandquit();
    // save shmid for easier cleanup
    shmsegments.insert(shmid);
    // attach the segment to a memory address in this process's heap.
    // shmaddr=NULL allows the system to choose a suitable page-aligned addr
    void* ataddr = shmat(shmid, NULL, 0);
    // the segment could not be attached, throw error
    if (ataddr == (void*)-1) perrandquit();
    // segment created and attached, return address of attach point
    return ataddr;
}

void* shmlookup(int key_id) {
    // attempts to find an existing memory segment with associated key_id
    // and attach it to this process's heap. If the memory segment does
    // not actually exist, this will throw an error and terminate.
    void* ataddr = shmat(shmlookupid(key_id), NULL, 0);
    if (ataddr == (void*)-1) perrandquit();
    // segment found and attached, return address of attach point
    return ataddr;
}

void shmdetach(const void* shmptr) {
    // detach a shared memory segment that has been attached at address shmptr
    // if no shared memory segment is actually attached at this address,
    // this will throw an error and terminate.
    if (shmdt(shmptr) == -1) perrandquit();
}

void shmdestroy(int key_id) {
    // attempts to destroy a shared memory segment associated with key_id
    // if no key_id is not associated with any shm or the current process
    // is not the creator of the shm, this will throw an error and terminate
    if (shmctl(shmlookupid(key_id), IPC_RMID, NULL) == -1) perrandquit();
}

int semcreate(int num, int& key_id) {
    // creates a semaphore array with num semaphores at key_id
    int semid = semget(getkeyfromid(key_id++), num, IPC_CREAT|IPC_EXCL|0660);
    // unable to create, throw error
    if (semid == -1) perrandquit();
    // save semid for easier cleanup
    semaphores.insert(semid);
    // semaphore array created, return its id
    return semid;
}

int semlookup(int key_id) {
    // attempts to find a created semaphore array associated with key_id
    // if no semaphore array exists with key_id, this will throw an error and
    // terminate
    int semid = semget(getkeyfromid(key_id), 0, 0);
    if (semid == -1) perrandquit();
    // semaphore array found, return its id
    return semid;
}

void semlock(int semid, int semnum) {
    // attempt to lock the semnum'th semaphore in semarray with id semid
    // if the semaphore is already locked, this process will block until
    // the semaphore is unlocked
    struct sembuf op;
    op.sem_num = semnum;
    op.sem_op = -1;
    op.sem_flg = 0;
    if (semop(semid, &op, 1) == -1 && errno != 4) {
        //errno 4 happens when process receives a signal while blocking,
        //which should NOT result in perror
        perrandquit();
    }
}

void semunlock(int semid, int semnum) {
    // attempt to unlock the semnum'th semaphore in semarray with id semid
    // in general, this should not fail unless semid or semnum are not valid
    struct sembuf op;
    op.sem_num = semnum;
    op.sem_op = 1;
    op.sem_flg = 0;

    if (semop(semid, &op, 1) == -1) perrandquit();
}

void semunlockall(int semid, int semsize) {
    // attempt to unlock all semaphores in semarray with id semid
    // in general this should not fail unless semid or semsize are not valid
    struct sembuf op[semsize];
    for ( int i = 0; i < semsize; i++) {
        op[i].sem_num = i;
        op[i].sem_op = 1;
        op[i].sem_flg = 0;
    }
    if (semop(semid, op, semsize) == -1) perrandquit();
}

void semlockall(int semid, int semsize) {
    // attempt to lock all semaphores in semarray with id semid
    // This probably will never be used
    struct sembuf op[semsize];
    for (int i : range(semsize)) {
        op[i].sem_num = i;
        op[i].sem_op = -1;
        op[i].sem_flg = 0;
    }
    if (semop(semid, op, semsize) == -1) perrandquit();
}

void semdestroy(int semid) {
    // attempt to destroy the semarray with id semid
    // if semid is invalid or this process is not the owner, this will throw
    // an error and terminate
    if (semctl(semid, 0, IPC_RMID) == -1) perrandquit();
}

void msgcreate(int& key_id) {
    // attempt to create a message queue at key_id
    int msqid = msgget(getkeyfromid(key_id++), IPC_CREAT|IPC_EXCL|0660);
    // desired msg queue could not be created, throw error
    if (msqid == -1) perrandquit();
    // save msqid for easier cleanup
    msgqueues.insert(msqid);
}

int msglookupid(int key_id) {
    // attempt to find an existing message queue at key_id and return its
    // internal id if found
    int msqid = msgget(getkeyfromid(key_id), 0660);
    if (msqid == -1) perrandquit();
    return msqid;
}

void msgsend(int key_id) {
    // send a zero-length message to a message queue at key_id
    // this should generally succeed so long as a message queue exists
    struct msgbuffer buf; // sacrificial buffer struct
    buf.mtype = 1; // required to be > 0, any value works here
    if (msgsnd(msglookupid(key_id), &buf, 0, 0) == -1) perrandquit();
}

void msgsend(int key_id, int mtype) {
    // send a zero-length message to a message queue at key_id
    // with a specific mtype. This should generally succeed so long
    // as a message queue exists
    struct msgbuffer buf;
    buf.mtype = mtype;
    if (msgsnd(msglookupid(key_id), &buf, 0, 0) == -1) perrandquit();
}

void msgsend(int key_id, pcbmsgbuf* buf) {
    // send a message containing the structure buf to a message queue
    // at key_id with a specific mtype. This should generally succeed so long
    // as a message queue exists.
    if (msgsnd(msglookupid(key_id), buf, sizeof(buf->data), 0) == -1)
        perrandquit();
}

void msgreceive(int key_id) {
    // reads first available zero-length message from the msg queue at key_id
    // if no message is present in the queue, the calling process will block
    // until a message is received.
    // In general, this call should succeed so long as a msg queue exists
    struct msgbuffer buf; // sacrificial buffer struct
    // do not acknowledge error due to interrupt, forces using interrupt
    // handler as defined in oss.cpp
    if (msgrcv(msglookupid(key_id), &buf, 0, 0, 0) == -1 && errno != EINTR) 
        perrandquit();
}

void msgreceive(int key_id, int mtype) {
    // reads the first message with mtype from the message queue at key_id
    // if no message with specified mtype is present in the queue, the calling 
    // process will block until a message is received.
    // In general, this call should succeed so long as a msg queue exists
    struct msgbuffer buf; // sacrificial buffer struct
    // do not acknowledge error due to interrupt, forces using interrupt
    // handler as defined in oss.cpp
    if (msgrcv(msglookupid(key_id), &buf, 0, mtype, 0) == -1 && errno != EINTR) 
        perrandquit();
}

void msgreceive(int key_id, pcbmsgbuf* buf) {
    // reads the first message with mtype specified by buf->mtype from the
    // message queue at key_id. If no message with specified mtype is present
    // in the queue, the calling proccess will block until a message is
    // received.
    // In general, this call should succeed so long as a msg queue exists
    // do not acknowledge error due to interrupt, forces using interrupt
    // handler as defined in oss.cpp
    if (msgrcv(
        msglookupid(key_id), buf, sizeof(buf->data), buf->mtype, 0) == -1 &&
            errno != EINTR)
                perrandquit();
}

bool msgreceivenw(int key_id) {
    // reads first available zero-length message from the msg queue at key_id
    // The calling process will not wait for a message if none is present
    // In general, this call should succeed so long as a msg queue exists
    struct msgbuffer buf; // sacrificial buffer struct
    if (msgrcv(msglookupid(key_id), &buf, 0, 0, IPC_NOWAIT) == -1) {
        if (errno != ENOMSG) {
            perrandquit();
        } else {
            return false;
        }
    }
    return true;
}

bool msgreceivenw(int key_id, int mtype) {
    // reads the first message with mtype from the message queue at key_id
    // The calling process will not wait for a message if none is present
    // In general, this call should succeed so long as a msg queue exists
    struct msgbuffer buf; // sacrificial buffer struct
    if (msgrcv(msglookupid(key_id), &buf, 0, mtype, IPC_NOWAIT) == -1) {
        if (errno != ENOMSG) {
            perrandquit();
        } else {
            return false;
        }
    }
    return true;
}

bool msgreceivenw(int key_id, pcbmsgbuf* buf) {
    // reads the first message with mtype specified by buf->mtype from the
    // message queue at key_id. The calling process will not wait for a 
    // message if none is present
    // In general, this call should succeed so long as a msg queue exists
    if (msgrcv(msglookupid(key_id), buf, sizeof(buf->data), 
               buf->mtype, IPC_NOWAIT) == -1) {
        if (errno != ENOMSG) {
            perrandquit();
        } else {
            return false;
        }
    }
    return true;
}

void msgdestroy(int key_id) {
    // attempt to destroy the message queue at key_id.
    // If key_id is invalid or the calling process is not the owner of
    // the message queue, this call will fail and the process will terminate
    if (msgctl(msglookupid(key_id), IPC_RMID, NULL) == -1) perrandquit();
}

void ipc_cleanup() {
    // destroys all ipc objects created by this process (that were saved)
    // and clears shmsegments, semaphores, and msgqueues sets
    // NOTE: This will not remove any ipc objects created manually by the
    // process outside of this library
    
    // ipc cleanup generally should not fail, but can. This function
    // provides no guarantees that all ipc objects will be freed prior
    // to the termination of this process
    for (int shmid : shmsegments) 
        if (shmctl(shmid, IPC_RMID, NULL) == -1) perrandquit();
    for (int semid : semaphores)
        if (semctl(semid, 0, IPC_RMID) == -1) perrandquit();
    for (int msqid : msgqueues)
        if (msgctl(msqid, IPC_RMID, NULL) == -1) perrandquit();

    shmsegments.clear();
    semaphores.clear();
    msgqueues.clear();
}
