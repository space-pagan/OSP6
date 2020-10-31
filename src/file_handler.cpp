/* Author:      Zoya Samsonov
 * Created:     October 6, 2020
 * Last edit:   October 10, 2020
 */

#include <fstream>           //ofstream, ifstream
#include <string>            //string
#include <vector>            //vector
#include <memory>            //shared_ptr
#include "error_handler.h"   //customerrorquit()
#include "file_handler.h"    //Self func decs

std::vector<std::shared_ptr<std::ofstream>> outfiles;
std::vector<std::shared_ptr<std::ifstream>> infiles;

int add_outfile(const char* name) {
    // open a file for writing in replace mode, and add to list of currently
    // open output files

    // requires shared_ptr<> due to gcc 4.8.5 in deployment env not complying
    // with movable streams
    std::shared_ptr<std::ofstream> out(new(std::ofstream));
    out->open(name);
    // file did not actually open, throw error
    if (!out->is_open()) customerrorquit("Unable to open file '" +\
            std::string(name) + "' for output!");
    outfiles.push_back(out);
    return outfiles.size() - 1; // count from 0
}

int add_outfile_append(std::string name) {
    return add_outfile_append(name.c_str());
}

int add_outfile_append(const char* name) {
    // open a file for writing in append mode, and add to list of currently
    // open output files
    std::shared_ptr<std::ofstream> out(new(std::ofstream));
    out->open(name, std::ios_base::app);
    // file dod not actually open, throw error
    if (!out->is_open()) customerrorquit("Unable to open file '" +\
            std::string(name) + "' for output!");
    outfiles.push_back(out);
    return outfiles.size() - 1; // count from 0
}

int add_infile(const char* name) {
    // open a file for reading, and add to list of currently open input files
    std::shared_ptr<std::ifstream> in(new(std::ifstream));
    in->open(name);
    // file did not actually open, throw error
    if (!in->is_open()) customerrorquit("Unable to open file '" +\
            std::string(name) + "' for input!");
    infiles.push_back(in);
    return infiles.size() - 1; // count from 0
}

int readline(int filenum, std::string& outstr) {
    // attempt to read a line from input file # filenum
    // if successful, external outstr is set to the line as read
    if (std::getline(*infiles[filenum], outstr))    return 1;
    // if unsuccessful read, throw error
    if (infiles[filenum]->bad()) customerrorquit("Unable to read from file");
    return 0;
}

void writeline(int filenum, std::string line) {
    // attempt to write a line to output file # filenum
    (*outfiles[filenum]) << line << "\n";
    // if unsuccessful write, throw error
    if (outfiles[filenum]->bad()) customerrorquit("Unable to write to file");
    // flush line to file. Without this, line is only flushed when the file
    // is closed
    outfiles[filenum]->flush();
}

void close_outfile(int filenum) {
    // manually close output file # filenum
    outfiles[filenum]->close();
}

void close_infile(int filenum) {
    // manually close input file # filenum
    infiles[filenum]->close();
}

void close_all() {
    // manually close all input and output files and clear the stream vectors
    for (auto f : outfiles) f->close();
    for (auto f : infiles) f->close();
    outfiles.clear();
    infiles.clear();
    // all previous file ids are invalid after this call. Files must be 
    // reopened with add_outfile/_append and add_infile
}
