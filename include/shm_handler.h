#ifndef SHM_HANDLER_H
#define SHM_HANDLER_H

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
#endif
