/* Author:      Zoya Samsonov
 * Created:     October 6, 2020
 * Last edit:   October 31, 2020
 */

#include <fstream>           //ofstream, ifstream
#include <string>            //string
#include "error_handler.h"   //customerrorquit()
#include "file_handler.h"    //Self func decs

File::File(std::string filepath, IOMODE open_mode) {
    name = filepath;
    mode = (std::ios_base::openmode)open_mode;
    // if (!stream->is_open()) 
        // customerrorquit("Unable to open file '" + name + "'!");
}

File::File(const File& old) {
    name = old.name;
    // stream = old.stream;
    mode = old.mode;
}

// int File::readline(std::string& outstr) {
    // if (std::getline(*this->stream, outstr)) return 1;
    // if (this->stream->bad()) customerrorquit("Unable to read from file");
    // return 0;
// }

int File::readline(std::string& outstr) {
    // open the file for use in the correct mode
    std::ifstream stream(this->name.c_str(), this->mode);
    // attempt to read a line from input file # filenum
    // if successful, external outstr is set to the line as read
    if (std::getline(stream, outstr)) return 1;
    // if unsuccessful read, throw error
    if (stream.bad()) customerrorquit("Unable to read from file");
    return 0;
}

void File::writeline(std::string line) {
    this->write(line + "\n");
}

// void File::write(std::string msg) {
    // (*this->stream) << msg;
    // if (this->stream->bad()) customerrorquit("Unable to write to file");
    // this->stream->flush();
// }

void File::write(std::string msg) {
    // this could be written as OO but gcc 4.9.5 breaks it
    // open the file for use in the correct mode
    std::ofstream stream(this->name.c_str(), this->mode);
    // attempt to write a line to output file # filenum
    stream << msg;
    if (stream.bad()) customerrorquit("Unable to write to file");
    // flush line to file. Without this, line is only flushed when the file
    // is closed
    stream.flush();
}
