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
#include "file_handler.h"

using namespace std::chrono;

void printclockmsg(std::ostream& out, std::string msg);
void getCurrentCTime(char* outchr);
char* sanitizeStr(char* testString);
bool testPalindrome(char* testString);
static std::string prefix;

void signalhandler(int signum) {
	if (signum == SIGINT) exit(-1);
}

int main(int argc, char **argv) {
	signal(SIGINT, signalhandler);

	prefix = std::string(argv[0]) + std::string(argv[1]);
	setupprefix(prefix.c_str());
	srand(getpid());

	int id = std::stoi(argv[1]);
	char* testStr = (char*)shmlookup(id);
	int semid = std::stoi(argv[2]);
	int semnum;
	int fileid;
	int logid = add_outfile_append("output.log");

	bool isPalindrome = testPalindrome(sanitizeStr(testStr));
	// writes entire message to terminal in one action to prevent
	// interference from messages from other processes
	std::string ln;
	if (isPalindrome) {
		ln = std::string(": ") + testStr +\
			 std::string(" is a palindrome!\n");
		fileid = add_outfile_append("palin.out");
		semnum = 0;
	} else {
		ln = std::string(": ") + testStr +\
			 std::string(" is not a palindrome!\n");
		fileid = add_outfile_append("nopalin.out");
		semnum = 1;
	}
	printclockmsg(std::cout, ln);

	// attempt to enter critical section
	semlock(semid, semnum);
	printclockmsg(std::cerr, ": Beginning critical section\n");

	// sleep for a random amount of time [0-2] seconds.
	sleep(rand() % 3);

	// Critical Section
	printclockmsg(std::cerr, ": In critical section\n");
	writeline(fileid, testStr);
	close_outfile(fileid);

	// exit critical section
	printclockmsg(std::cerr, ": Left critical section\n");
	semunlock(semid, semnum);
	
	// log to logfile
	semlock(semid, 2);
	printclockmsg(std::cerr, ": Writing to log\n");
	writeline(logid, std::to_string((int)getpid()) + std::string("\t") +\
			std::to_string(id) + std::string("\t") + testStr);
	close_outfile(logid);
	semunlock(semid, 2);

	// cleanup
	shmdetach(testStr);
	return 0;
}

void printclockmsg(std::ostream& out, std::string msg) {
	std::ostringstream termout;
	char timechr[9];
	
	getCurrentCTime(timechr);
	termout << "[" << timechr << "]  ";
	termout << prefix << msg;
	out << termout.str();
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
