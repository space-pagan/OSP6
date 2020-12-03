#ifndef SHM_HANDLER_H
#define SHM_HANDLER_H

#include <sys/types.h>

enum Status { REQ_READ, REQ_WRITE, TERM };

typedef struct {
    int pid;
    Status status;
    int resarray[20];
    int resi;
    int resamount;
    pid_t realpid;
}Data;

struct pcbmsgbuf {
    long mtype;
    Data data;
};

void* shmcreate(size_t bytes, int& key_id);
void* shmlookup(int key_id);
void shmdetach(const void* shmptr);
void shmdestroy(int key_id);
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
