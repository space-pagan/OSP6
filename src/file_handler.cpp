/* Author:      Zoya Samsonov
 * Created:     October 6, 2020
 * Last edit:   October 31, 2020
 */

#include <fstream>           //ofstream, ifstream
#include <string>            //string
#include "error_handler.h"   //customerrorquit()
#include "file_handler.h"    //Self func decs

int File::readline(std::string& outstr) {
    // attempt to read a line from input file # filenum
    // if successful, external outstr is set to the line as read
    if (std::getline(*this->stream, outstr))    return 1;
    // if unsuccessful read, throw error
    if (this->stream->bad()) customerrorquit("Unable to read from file");
    return 0;
}

void File::writeline(std::string line) {
    this->write(line + "\n");
}

void File::write(std::string msg) {
    // attempt to write a line to output file # filenum
    (*this->stream) << msg;
    // if unsuccessful write, throw error
    if (this->stream->bad()) customerrorquit("Unable to write to file");
    // flush line to file. Without this, line is only flushed when the file
    // is closed
    this->stream->flush();
}
