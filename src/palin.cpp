/* Author: Zoya Samsonov
 * Date: October 6, 2020
 */

#include <iostream>          //cout, cerr
#include <sstream>           //ostringstream
#include <string>            //string
#include <cstring>           //strlen(), strcmp()
#include <unistd.h>          //getpid()
#include <stdlib.h>          //srand()
#include <signal.h>          //signal()
#include <chrono>            //system_clock::
#include <ctime>             //ctime()
#include "shm_handler.h"     //semlock(), semunlock(), shmlookup()
#include "error_handler.h"   //setupprefix()
#include "file_handler.h"    //add_outfile_append(), writeline()

static std::string prefix;

void signalhandler(int signum) {
    // child does not allocate ipc, can exit safely
    if (signum == SIGINT) exit(-1);
}

void getCurrentCTime(char* outchr) {
    // saves the current time to outchr in HH:MM:SS format
    using namespace std::chrono; // makes the next line much shorter

    auto timenow = system_clock::to_time_t(system_clock::now());
    const char* timechr = std::ctime(&timenow);
    // clear any data in outchr
    memset(outchr, '\0', 9);
    // only copy HH:MM:SS
    strncpy(outchr, timechr+11, 8);
}

void printclockmsg(std::ostream& out, std::string msg) {
    // out << "[HH:MM:SS]  prefix: msg"
    std::ostringstream termout;
    char timechr[9];
    
    getCurrentCTime(timechr);
    termout << "[" << timechr << "]  ";
    termout << prefix << msg;
    // print entire message at once to prevent multiple proccesses writing to
    // out at the same time and possibly merging messages
    out << termout.str();
}

char* sanitizeStr(char* testString) {
    // creates a new string from testString that only contains 0-9A-Za-z
    // sanitary is a memory leak, but palin is a short-lived process so
    // it's okay
    int len = strlen(testString);
    char* sanitary = new char[len];
    int j = 0;
    for (int i = 0; i < len; i++) {
        char c = testString[i];
        if ((('0' <= c) && (c <= '9')) || (('A' <= c) && (c <= 'Z')) 
                || (('a' <= c) && (c <= 'z'))) {
            // convert to lowercase so that mixed-case strings are still
            // recognized as palindromes
            sanitary[j++] = std::tolower(c);
        }
    }
    sanitary[j] = '\0';
    return sanitary;
}

bool testPalindrome(char* testString) {
    // tests if a string is a palindrome
    // in general, only call this after sanitizing the string
    int len = strlen(testString);
    char* reversed = new char[len];
    // build the reversed string
    for (int i = 0; i < len; i++) {
        reversed[i] = testString[len-i-1];
    }
    // test if the two strings are equal
    if (strcmp(testString, reversed) == 0) {
        delete reversed;
        return true;
    }
    delete reversed;
    return false;
}

void critical_section(int semid, char* testStr) {
    // tests if testStr is a palindrome and writes appropriate output to
    // appropriate file

    // defaults
    int semnum = 1;
    std::string word = "not ";
    std::string outfile = "nopalin.out";

    // testStr is a palindrome, change defaults
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
    // sleep for a random amount of time between 0 and 2 seconds inclusive
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
    // writes to logfile
    // log << "PID    ID    testStr"
    semlock(semid, 2);
    int logid = add_outfile_append("output.log");
    writeline(logid, std::to_string((int)getpid()) + std::string("\t") +\
            std::to_string(id) + std::string("\t") + testStr);
    close_outfile(logid);
    semunlock(semid, 2);
}

int main(int argc, char **argv) {
    // set up signal handling
    signal(SIGINT, signalhandler);

    // set up perror prefix
    prefix = std::string(argv[0]) + std::string(argv[1]);
    setupprefix(prefix.c_str());
    // set seed for rand()
    srand(getpid());

    // parse arguments
    int id = std::stoi(argv[1]);
    char* testStr = (char*)shmlookup(id);
    int semid = std::stoi(argv[2]);

    // test string, write to file, write to log
    critical_section(semid, testStr);
    remainder_section(semid, id, testStr);

    // cleanup
    shmdetach(testStr);
    return 0;
}
