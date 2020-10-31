#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <string>

int add_outfile(const char* name);
int add_outfile_append(std::string name);
int add_outfile_append(const char* name);
int add_infile(const char* name);
int readline(int filenum, std::string& outstr);
void writeline(int filenum, std::string line);
void close_outfile(int filenum);
void close_infile(int filenum);
void close_all();

#endif
