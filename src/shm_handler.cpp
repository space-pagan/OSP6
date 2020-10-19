/* Author: Zoya Samsonov
 * Date: October 6, 2020
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
#include "shm_handler.h"        //Self func defs
#include "file_handler.h"       //add_infile(), readline()

// store references to all created shared memory objects for easier cleanup
std::set<int> shmsegments;
std::set<int> semaphores;
std::set<int> msgqueues;

struct msgbuffer {
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

void shmfromfile(const char* filename, int& id, int maxlines) {
    // read up to maxlines non-blank lines from file "filename" and
    // create a shm segment storing each line
    // increment external id to match the next available id to use for
    // shared segment creation
    int fileid = add_infile(filename);
    int linecount = 0;
    std::string line;
    while (readline(fileid, line) && (linecount++ < maxlines)) {
        // shmcreate fails if size=0, skip blank lines
        if (line.size() == 0) {
            linecount--; 
            continue;
        }
        char* shmstr = (char*)shmcreate(line.size(), id);
        // copy contents of the read line to the shared memory segment
        strcpy(shmstr, line.c_str());
        // detach shared memory segment. any further operations will need
        // to call shmlookup
        shmdetach(shmstr);
    }
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
    for (int i = 0; i < semsize; i++) {
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
    int msqid = msgget(getkeyfromid(key_id++), IPC_CREAT|IPC_EXCL|0660);
    // desired msg queue could not be created, throw error
    if (msqid == -1) perrandquit();
    // save shmid for easier cleanup
    msgqueues.insert(msqid);
}

int msglookupid(int key_id) {
    int msqid = msgget(getkeyfromid(key_id), 0660);
    if (msqid == -1) perrandquit();
    return msqid;
}

void msgsend(int key_id) {
    struct msgbuffer buf;
    buf.mtype = 1; //this really doesn't matter
    if (msgsnd(msglookupid(key_id), &buf, 0, 0) == -1) perrandquit();
}

void msgreceive(int key_id) {
    struct msgbuffer buf;
    if (msgrcv(msglookupid(key_id), &buf, 0, 0, 0) == -1) perrandquit();
}

void msgdestroy(int key_id) {
    if (msgctl(msglookupid(key_id), IPC_RMID, NULL) == -1) perrandquit();
}

void ipc_cleanup() {
    // destroys all ipc objects created by this process (that were saved)
    // and clears shmsegments and semaphores sets
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
