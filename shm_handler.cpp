/* Author: Zoya Samsonov
 * Date: September 30, 2020
 */

#include "shm_handler.h"

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

void* shmcreate(size_t bytes, int key_id) {
	int shmid = shmget(getkeyfromid(key_id), bytes, IPC_CREAT|IPC_EXCL|0660);
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
	std::ifstream file(filename);
	std::string line;
	int linecount = 0;
	while ((std::getline(file, line)) && (linecount++ < maxlines)) {
		// shmcreate fails if size=0, skip blank lines
		if (line.size() == 0) {
			linecount--; 
			continue;
		}
		char* shmstr = (char*)shmcreate(line.size(), id++);
		strcpy(shmstr, line.c_str());
		shmdetach(shmstr);
	}
}

int semcreate(int num, int key_id) {
	int semid = semget(getkeyfromid(key_id), num, IPC_CREAT|IPC_EXCL|0660);
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
