/* Author: Zoya Samsonov
 * Date: October 6, 2020
 */

#include <iostream>			//cout, cerr
#include <sstream>			//ostringstream
#include <string>			//string
#include <cstring>			//strlen(), strcmp()
#include <unistd.h>			//getpid()
#include <stdlib.h>			//srand()
#include <signal.h>			//signal()
#include <chrono>			//system_clock::
#include <ctime>			//ctime()
#include "shm_handler.h"	//semlock(), semunlock(), shmlookup()
#include "error_handler.h"	//
#include "file_handler.h"	//add_outfile_append(), writeline()

using namespace std::chrono;

static std::string prefix;

void signalhandler(int signum) {
	if (signum == SIGINT) exit(-1);
}

void getCurrentCTime(char* outchr) {
	auto timenow = system_clock::to_time_t(system_clock::now());
	const char* timechr = std::ctime(&timenow);
	memset(outchr, '\0', 9);
	strncpy(outchr, timechr+11, 8);
}

void printclockmsg(std::ostream& out, std::string msg) {
	std::ostringstream termout;
	char timechr[9];
	
	getCurrentCTime(timechr);
	termout << "[" << timechr << "]  ";
	termout << prefix << msg;
	out << termout.str();
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
			sanitary[j++] = std::tolower(c);
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

void critical_section(int semid, char* testStr) {
	int semnum = 1;
	std::string word = "not ";
	std::string outfile = "nopalin.out";

	if (testPalindrome(sanitizeStr(testStr))) {
		semnum = 0;
		word = "";
		outfile = "palin.out";
	}
	printclockmsg(std::cout, ": " + std::string(testStr) + " is " + word +\
			"a palindrome!\n");

	// attempt to get lock on semaphore (process will sleep until unblocked)
	semlock(semid, semnum);
	printclockmsg(std::cerr, ": Beginning critical section\n");
	sleep(rand() % 3);
	printclockmsg(std::cerr, ": In critical section\n");
	
	// write to file
	int fileid = add_outfile_append(outfile.c_str());
	writeline(fileid, testStr);
	close_outfile(fileid);

	// exit critical section
	printclockmsg(std::cerr, ": Left critical section\n");
	semunlock(semid, semnum);
}

void remainder_section(int semid, int id, char* testStr) {
	semlock(semid, 2);
	int logid = add_outfile_append("output.log");
	writeline(logid, std::to_string((int)getpid()) + std::string("\t") +\
			std::to_string(id) + std::string("\t") + testStr);
	close_outfile(logid);
	semunlock(semid, 2);
}

int main(int argc, char **argv) {
	signal(SIGINT, signalhandler);

	prefix = std::string(argv[0]) + std::string(argv[1]);
	setupprefix(prefix.c_str());
	srand(getpid());

	int id = std::stoi(argv[1]);
	char* testStr = (char*)shmlookup(id);
	int semid = std::stoi(argv[2]);

	critical_section(semid, testStr);
	remainder_section(semid, id, testStr);

	// cleanup
	shmdetach(testStr);
	return 0;
}
