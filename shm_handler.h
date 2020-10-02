#ifndef SHM_HANDLER_H
#define SHM_HANDLER_H

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include "error_handler.h"

void* shmcreate(size_t bytes, int key_id);
void* shmlookup(int key_id);
void shmdetach(const void* shmptr);
void shmdestroy(int key_id);
void shmfromfile(const char* filename, int& first_id, int maxlines);
int semcreate(int num, int key_id);
int semlookup(int key_id);
void semlock(int semid, int semnum);
void semunlock(int semid, int semnum);
void semdestroy(int semid);
#endif
