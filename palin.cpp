/* Author: Zoya Samsonov
 * Date: September 28, 2020
 */

#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <chrono>
#include <ctime>
#include "shm_handler.h"
#include "error_handler.h"


bool testPalindrome(char* testString);
char* sanitizeStr(char* testString);
char* getCurrentCTime();

void signalhandler(int signum) {
	if (signum == SIGINT) exit(-1);
}

int main(int argc, char **argv) {
	signal(SIGINT, signalhandler);

	char prefix[strlen(argv[0]) + strlen(argv[1])];
	sprintf(prefix, "%s%s", argv[0], argv[1]);
	setupprefix(prefix);
	srand(getpid());

	int id = std::stoi(argv[1]);
	char* testStr = (char*)shmlookup(id);
	int semid = std::stoi(argv[2]);

	bool isPalindrome = testPalindrome(sanitizeStr(testStr));
	
	// attempt to enter critical section
	// semlock(semid, 0);
	std::cout << argv[0] << argv[1] << ": " << testStr << " is ";

	if (!isPalindrome) {
		std::cout << "not ";
	}
	std::cout << "a palindrome!\n";
	
	semlock(semid, 0);

	std::cerr << argv[0] << argv[1] << ": Beginning critical section at: ";
	std::cerr << getCurrentCTime() << "\n";

	// sleep for a random amount of time [0-2] seconds.
	sleep(rand() % 3);

	// Critical Section
	std::cerr << argv[0] << argv[1] << ": In critical section at:        ";
	std::cerr << getCurrentCTime() << "\n";

	// exit critical section
	std::cerr << argv[0] << argv[1] << ": Left critical section at:      ";
	std::cerr << getCurrentCTime() << "\n";
	semunlock(semid, 0);
	shmdetach(testStr);

	return 0;
}

char* getCurrentCTime() {
	auto time_point = std::chrono::system_clock::now();
	std::time_t time_c = std::chrono::system_clock::to_time_t(time_point);
	return std::ctime(&time_c);
}

char* sanitizeStr(char* testString) {
	int len = strlen(testString);
	char* sanitary = new char[len];
	int j = 0;
	for (int i = 0; i < len; i++) {
		char c = testString[i];
		if (((48 <= c) && (c <= 57)) || ((65 <= c) && (c <= 90)) 
				|| ((97 <= c) && (c <= 122))) {
			// 0-9A-Za-z
			sanitary[j++] = c;
		}
	}
	sanitary[j] = '\0';
	return sanitary;
}

bool testPalindrome(char* testString) {
	int len = strlen(testString);
	char* reversed = new char[len];
	for (int i = 0; i < len; i++) {
		reversed[i] = testString[len-i-1];
	}
	if (strcmp(testString, reversed) == 0) {
		return true;
	}
	return false;
}
