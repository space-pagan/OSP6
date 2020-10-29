#ifndef CLI_HANDLER_H
#define CLI_HANDLER_H

#include <vector>
#include <string>

int getcliarg(int argc, char** argv, const char* options, \
        const char* flags, std::vector<std::string> &optout,\
        bool* flagout);

void parserunpath(char** argv, std::string& runpath, std::string& pref);

#endif
