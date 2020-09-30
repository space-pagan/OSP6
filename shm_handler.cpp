/* Author: Zoya Samsonov
 * Date: September 30, 2020
 */

#include <iostream>
#include "shm_handler.h"

void* shmcreate(size_t size, int id) {
	key_t key = ftok(".", id);
	if (key == -1) perrandquit();
	int shmid = shmget(key, size, IPC_CREAT|IPC_EXCL|0660);
	if (shmid == -1) perrandquit();
	void* ataddr = shmat(shmid, NULL, 0);
	if (ataddr == (void*)-1) perrandquit();
	return ataddr;
}

int shmlookupid(int id) {
	key_t key = ftok(".", id);
	if (key == -1) perrandquit();
	int shmid = shmget(key, 0, 0);
	if (shmid == -1) perrandquit();
	return shmid;
}

void* shmlookup(int id) {
	void* ataddr = shmat(shmlookupid(id), NULL, 0);
	if (ataddr == (void*)-1) perrandquit();
	return ataddr;
}

void shmdetach(const void* shmaddr) {
	int dt = shmdt(shmaddr);
	if (dt == -1) perrandquit();
}

void shmdestroy(int id) {
	int ctl = shmctl(shmlookupid(id), IPC_RMID, NULL);
	if (ctl == -1) perrandquit();
}
