#ifndef SHM_HANDLER_H
#define SHM_HANDLER_H

#include <sys/ipc.h>
#include <sys/shm.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include "error_handler.h"

void* shmcreate(size_t, int);
void* shmlookup(int);
void shmdetach(const void*);
void shmdestroy(int);
void shmfromfile(const char*, int&, int&);
#endif
