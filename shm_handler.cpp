/* Author: Zoya Samsonov
 * Date: September 30, 2020
 */

#include <sys/types.h>			//key_t
#include <sys/ipc.h>			//ftok(), IPC_CREAT, IPC_EXCL
#include <sys/shm.h>			//shmget(), shmat(), shmdt(), shmctl()
#include <sys/sem.h>			//semget(), semop(), semctl()
#include <string>				//string
#include <cstring>				//strcpy()
#include "error_handler.h"		//perrandquit()
#include "shm_handler.h"		//Self func defs
#include "file_handler.h"		//add_infile(), readline()

key_t getkeyfromid(int key_id) {
	key_t key = ftok(".", key_id);
	if (key == -1) perrandquit();
	return key;
}

int shmlookupid(int key_id) {
	int shmid = shmget(getkeyfromid(key_id), 0, 0);
	if (shmid == -1) perrandquit();
	return shmid;
}

void* shmcreate(size_t bytes, int& key_id) {
	int shmid = shmget(getkeyfromid(key_id++), bytes, IPC_CREAT|IPC_EXCL|0660);
	if (shmid == -1) perrandquit();
	void* ataddr = shmat(shmid, NULL, 0);
	if (ataddr == (void*)-1) perrandquit();
	return ataddr;
}

void* shmlookup(int key_id) {
	void* ataddr = shmat(shmlookupid(key_id), NULL, 0);
	if (ataddr == (void*)-1) perrandquit();
	return ataddr;
}

void shmdetach(const void* shmptr) {
	if (shmdt(shmptr) == -1) perrandquit();
}

void shmdestroy(int key_id) {
	if (shmctl(shmlookupid(key_id), IPC_RMID, NULL) == -1) perrandquit();
}

void shmfromfile(const char* filename, int& id, int maxlines) {
	int fileid = add_infile(filename);
	int linecount = 0;
	std::string line;
	while ((readline(fileid, line)) && (linecount++ < maxlines)) {
		// shmcreate fails if size=0, skip blank lines
		if (line.size() == 0) {
			linecount--; 
			continue;
		}
		char* shmstr = (char*)shmcreate(line.size(), id);
		strcpy(shmstr, line.c_str());
		shmdetach(shmstr);
	}
}

int semcreate(int num, int& key_id) {
	int semid = semget(getkeyfromid(key_id++), num, IPC_CREAT|IPC_EXCL|0660);
	if (semid == -1) perrandquit();
	return semid;
}

int semlookup(int key_id) {
	int semid = semget(getkeyfromid(key_id), 0, 0);
	if (semid == -1) perrandquit();
	return semid;
}

void semlock(int semid, int semnum) {
	struct sembuf op;
	op.sem_num = semnum;
	op.sem_op = -1;
	op.sem_flg = 0;
	if (semop(semid, &op, 1) == -1 && errno != 4) {
		perrandquit();
	}
}

void semunlock(int semid, int semnum) {
	struct sembuf op;
	op.sem_num = semnum;
	op.sem_op = 1;
	op.sem_flg = 0;

	if (semop(semid, &op, 1) == -1) perrandquit();
}

void semdestroy(int semid) {
	if (semctl(semid, 0, IPC_RMID) == -1) perrandquit();
}
