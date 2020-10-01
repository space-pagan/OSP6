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

void shmfromfile(const char* filename, int& id, int& maxlines) {
	std::ifstream file(filename);
	std::string line;
	int linecount = 0;
	while ((std::getline(file, line)) && (linecount++ < maxlines)) {
		if (line.size() == 0) {
			linecount--;
			continue;
		}
		char* shmstr = (char*)shmcreate(line.size(), id++);
		strcpy(shmstr, line.c_str());
		shmdetach(shmstr);
	}
	// if the number of lines in the file < -n argument, reset to prevent
	// attempting to access shared memory that doesn't exist
	if (line.size() == 0) linecount++;
	maxlines = --linecount;
}
