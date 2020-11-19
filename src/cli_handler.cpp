/* Author:      Zoya Samsonov
 * Created:     September 9, 2020
 * Last edit:   October 28, 2020
 */

#include <unistd.h>             //getopt()
#include <string>               //string
#include <cstring>              //strlen()
#include <sstream>              //ostringstream
#include <vector>               //std::vector
#include <sys/stat.h>           //stat()
#include "error_handler.h"      //custerrhelpprompt()
#include "util.h"
#include "cli_handler.h"        //Self func defs

char* getoptstr(const char* options, const char* flags) {
    // creates a string for getopt format given options and flags
    // ex: options = "abc" flags = "def" -> optstr = ":a:b:c:def"
    int optlen = strlen(options);
    int flglen = strlen(flags);
    char* optstr = new char[optlen * 2 + flglen + 1];
    // leading : means that getopt returns ':' if required argument is missing
    optstr[0] = ':';
    int i = 1;
    if (optlen) {
        for (int j : range(optlen)) {
            optstr[i++] = options[j];
            optstr[i++] = ':';
        }
    }
    if (flglen) {
        for (int j : range(flglen)) {
            optstr[i++] = flags[j];
        }
    }
    return optstr;
}

int getcliarg(int argc, char** argv, const char* options, \
              const char* flags, std::vector<std::string> &optout, \
              bool* flagout) {
    // Searches argv for options and flags, stores values in optout, flagout
    // if any arguments are not present or unknown flags are used,
    // returns -1 to tell main() to quit. Otherwise returns 0;
    char* optstr = getoptstr(options, flags);
    int c;
    opterr = 0;  // disable getopt printing error messages

    while ((c = getopt(argc, argv, optstr)) != -1) {
        if (c == '?') {
            custerrhelpprompt("Unknown argument '-" +\
                    std::string(1, (char)optopt) +    "'");
        }
        if (c == ':') {
            custerrhelpprompt("Option '-" +\
                    std::string(1, (char)optopt) + "' requires an argument!");
        }
        // check if the matched item is an option
        int optindex = -1;
        int optlen = strlen(options);
        int flglen = strlen(flags);
        if (optlen) {
            for (int j : range(optlen)) {
                if (c == options[j]) {
                    optindex = j;
                    // yes, set optout to argument value
                    optout[optindex] = std::string(optarg);
                    break;
                }
            }
        }
        if (optindex == -1 && flglen) {
            // optindex not set, match must be a flag
            for (int j : range((int)strlen(flags))) {
                if (c == flags[j]) {
                    // set corresponding flagout
                    flagout[j] = true;
                    break;
                }
            }
        }
    }
    // tell main how many arguments in argv have been parsed
    return optind;
}

void parserunpath(char** argv, std::string& runpath, std::string& pref) {
    // This function parses argv[0] and puts the PWD and name of the binary
    // into runpath and pref respectively
    std::string rawpath = argv[0];
    size_t split = rawpath.rfind('/')+1;
    if (split != 0) {
        runpath = rawpath.substr(0, split);
        pref = rawpath.substr(split);
    } else {
        runpath = std::string("./");
        pref = std::string(argv[0]);
    }
}

bool pathdepcheck(std::string runpath, std::string depname) {
    // This function confirms the existence of a file named depname in the path
    // runpath. Used to ensure that dependant binaries are present
    struct stat buffer;
    std::string depcheck = runpath + depname;
    if (stat(depcheck.c_str(), &buffer) == 0) return true;
    return false;
}
