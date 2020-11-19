/* Author:      Zoya Samsonov
 * Created:     October 6, 2020
 * Last edit:   October 31, 2020
 */

#include <fstream>           //ofstream, ifstream
#include <string>            //string
#include "error_handler.h"   //customerrorquit()
#include "file_handler.h"    //Self func decs

int File::readline(std::string& outstr) {
    // open the file for use in the correct mode
    this->stream = new std::fstream(this->name.c_str(), this->mode);
    // attempt to read a line from input file # filenum
    // if successful, external outstr is set to the line as read
    if (std::getline(*this->stream, outstr)) {
        // gcc 4.9.5 disallows copying streams, might as well delete it
        delete this->stream;
        return 1;
    }
    delete this->stream;
    // if unsuccessful read, throw error
    if (this->stream->bad()) customerrorquit("Unable to read from file");
    return 0;
}

void File::writeline(std::string line) {
    this->write(line + "\n");
}

void File::write(std::string msg) {
    // open the file for use in the correct mode
    this->stream = new std::fstream(this->name.c_str(), this->mode);
    // attempt to write a line to output file # filenum
    (*this->stream) << msg;
    // flush line to file. Without this, line is only flushed when the file
    // is closed
    this->stream->flush();
    // gcc 4.9.5 disallows copying streams, might as well delete it
    delete this->stream;
}
