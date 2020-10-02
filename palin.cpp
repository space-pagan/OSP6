/* Author: Zoya Samsonov
 * Date: September 28, 2020
 */

#include <iostream>			//cout, cerr
#include <sstream>			//ostringstream
#include <string>			//string
#include <cstring>			//strlen(), strcmp()
#include <unistd.h>			//
#include <stdlib.h>			//
#include <signal.h>			//
#include <chrono>
#include <ctime>			//
#include "shm_handler.h"	//
#include "error_handler.h"	//

using namespace std::chrono;

bool testPalindrome(char* testString);
char* sanitizeStr(char* testString);
void getCurrentCTime(char* outchr);

void signalhandler(int signum) {
	if (signum == SIGINT) exit(-1);
}

int main(int argc, char **argv) {
	signal(SIGINT, signalhandler);

	std::ostringstream prefix;
	prefix << argv[0] << argv[1];
	setupprefix(prefix.str().c_str());
	srand(getpid());

	int id = std::stoi(argv[1]);
	char* testStr = (char*)shmlookup(id);
	int semid = std::stoi(argv[2]);
	int semnum;

	bool isPalindrome = testPalindrome(sanitizeStr(testStr));
	char timechr[9];
	// writes entire message to terminal in one action to prevent
	// interference from messages from other processes
	std::ostringstream termout;
	getCurrentCTime(timechr);
	termout << timechr << ":  ";
	termout << prefix.str() << ": " << testStr << " is ";
	if (!isPalindrome) {
		termout << "not a palindrome!\n";
		semnum = 1;
	} else {
		termout << "a palindrome!\n";
		semnum = 0;
	}
	std::cout << termout.str();

	// attempt to enter critical section
	semlock(semid, semnum);
	termout.str("");
	getCurrentCTime(timechr);
	termout << timechr << ":  ";
	termout << prefix.str() << ": Beginning critical section\n";
	std::cerr << termout.str();

	// sleep for a random amount of time [0-2] seconds.
	sleep(rand() % 3);

	// Critical Section
	termout.str("");
	getCurrentCTime(timechr);
	termout << timechr << ":  ";
	termout << prefix.str() << ": In critical section\n";
	std::cerr << termout.str();

	// exit critical section
	termout.str("");
	getCurrentCTime(timechr);
	termout << timechr << ":  ";
	termout << prefix.str() << ": Left critical section\n";
	std::cerr << termout.str();
	semunlock(semid, semnum);
	
	// log to logfile
	semlock(semid, 2);
	termout.str("");
	getCurrentCTime(timechr);
	termout << timechr << ":  ";
	termout << prefix.str() << ": Writing to log\n";
	std::cerr << termout.str();
	semunlock(semid, 2);

	// cleanup
	shmdetach(testStr);
	return 0;
}

void getCurrentCTime(char* outchr) {
	auto timenow = system_clock::to_time_t(system_clock::now());
	const char* timechr = std::ctime(&timenow);
	memset(outchr, '\0', 9);
	strncpy(outchr, timechr+11, 8);
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
