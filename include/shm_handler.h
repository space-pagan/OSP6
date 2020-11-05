#ifndef SHM_HANDLER_H
#define SHM_HANDLER_H

#include <sys/types.h>

enum Data { PCBNUM, TIMESLICE, STATUS };
enum Status { TERM, RUN, BLOCK, UBLOCK, PREEMPT };

struct pcbmsgbuf {
    long mtype;
    int data[3];
};

void* shmcreate(size_t bytes, int& key_id);
void* shmlookup(int key_id);
void shmdetach(const void* shmptr);
void shmdestroy(int key_id);
void shmfromfile(const char* filename, int& first_id, int maxlines);
int semcreate(int num, int& key_id);
int semlookup(int key_id);
void semlock(int semid, int semnum);
void semunlock(int semid, int semnum);
void semunlockall(int semid, int semsize);
void semlockall(int semid, int semsize);
void semdestroy(int semid);
void msgcreate(int& key_id);
int msglookupid(int key_id);
void msgsend(int key_id);
void msgsend(int key_id, int mtype);
void msgsend(int key_id, pcbmsgbuf* buf);
void msgreceive(int key_id);
void msgreceive(int key_id, int mtype);
void msgreceive(int key_id, pcbmsgbuf* buf);
bool msgreceivenw(int key_id);
bool msgreceivenw(int key_id, int mtype);
bool msgreceivenw(int key_id, pcbmsgbuf* buf);
void ipc_cleanup();

#endif
