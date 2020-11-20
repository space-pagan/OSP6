/* Author:      Zoya Samsonov
 * Created:     October 6, 2020
 * Last edit:   October 31, 2020
 */

#include <fstream>           //ofstream, ifstream
#include <string>            //string
#include "error_handler.h"   //customerrorquit()
#include "file_handler.h"    //Self func decs

File::File(std::string filepath, IOMODE open_mode) {
    // File constructor, creates a File object for a given path and 
    // input mode (READ, WRITE, APPEND)
    name = filepath;
    mode = (std::ios_base::openmode)open_mode;
}

File::File(const File& old) {
    // File copy-constructor
    name = old.name;
    mode = old.mode;
}

int File::readline(std::string& outstr) {
    // attempt to read a line from a file and save it to outstr
    // open the file for use in the correct mode
    std::ifstream stream(this->name.c_str(), this->mode);
    // if successful, external outstr is set to the line as read
    if (std::getline(stream, outstr)) return 1;
    // if unsuccessful read, throw error
    if (stream.bad()) customerrorquit("Unable to read from file");
    return 0;
}

void File::writeline(std::string line) {
    // alias for write with an appended newline
    this->write(line + "\n");
}

void File::write(std::string msg) {
    // attempt to write 
    // open the file for use in the correct mode
    std::ofstream stream(this->name.c_str(), this->mode);
    // attempt to write a line to output file # filenum
    stream << msg;
    if (stream.bad()) customerrorquit("Unable to write to file");
    // flush line to file. Without this, line is only flushed when the file
    // is closed
    stream.flush();
}
