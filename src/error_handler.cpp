/* Author:      Zoya Samsonov
 * Created:     September 10, 2020
 * Last edit:   October 10, 2020
 */

#include <string>               //string
#include <iostream>             //cerr
#include "shm_handler.h"        //ipc_cleanup()
#include "error_handler.h"      //Self func defs

std::string rawprefix; // stores argv[0] ONLY
std::string prefix;    // stores argv[0]: Error

void setupprefix(const char* arg0) {
    // sets the value of prefix. Run this at the very start of the program 
    // to ensure that perror always displays the correct error message
    prefix = arg0;
    rawprefix = arg0;
    prefix += ": Error";
}

void perrandquit() {
    // print a detailed error message in the format 
    // argv[0]: Error: detailed error message, then terminate
    perror(prefix.c_str());
    ipc_cleanup();
    exit(-1);
}

void customerrorquit(const char* error) {
    // print a custom error message in the format 
    // argv[0]: Error: custom error message, then terminate
    std::cerr << prefix << ": " << error << "\n";
    ipc_cleanup();
    exit(-1);
}

void customerrorquit(std::string error) {
    // alias in case error is of type std::string
    customerrorquit(error.c_str());
}

void custerrhelpprompt(const char* error) {
    // print a custom error message followed by a line instructing how
    // to display program help, then terminate
    std::cerr << prefix << ": " << error << "\n";
    std::cerr << "Please run '" << rawprefix;
    std::cerr << " -h' for more assistance!\n\n";
    ipc_cleanup();
    exit(-1);
}

void custerrhelpprompt(std::string error) {
    // alias in case error is of type std::string
    custerrhelpprompt(error.c_str());
}
