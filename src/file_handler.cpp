/* Author: Zoya Samsonov
 * Date: October 6, 2020
 */

#include <fstream>			//ofstream, ifstream
#include <string>			//string
#include <vector>			//vector
#include <memory>			//shared_ptr
#include "error_handler.h"	//customerrorquit()
#include "file_handler.h"	//Self func decs

std::vector<std::shared_ptr<std::ofstream>> outfiles;
std::vector<std::shared_ptr<std::ifstream>> infiles;

int add_outfile(const char* name) {
	std::shared_ptr<std::ofstream> out(new(std::ofstream));
	out->open(name);
	if (!out->is_open()) customerrorquit("Unable to open file '" +\
			std::string(name) + "' for output!");
	outfiles.push_back(out);
	return outfiles.size() - 1;
}

int add_outfile_append(const char* name) {
	std::shared_ptr<std::ofstream> out(new(std::ofstream));
	out->open(name, std::ios_base::app);
	if (!out->is_open()) customerrorquit("Unable to open file '" +\
			std::string(name) + "' for output!");
	outfiles.push_back(out);
	return outfiles.size() - 1;
}

int add_infile(const char* name) {
	std::shared_ptr<std::ifstream> in(new(std::ifstream));
	in->open(name);
	if (!in->is_open()) customerrorquit("Unable to open file '" +\
			std::string(name) + "' for input!");
	infiles.push_back(in);
	return infiles.size() - 1;
}

int readline(int filenum, std::string& outstr) {
	if (std::getline(*infiles[filenum], outstr))	return 1;
	if (infiles[filenum]->bad()) customerrorquit("Unable to read from file");
	return 0;
}

void writeline(int filenum, std::string line) {
	(*outfiles[filenum]) << line << "\n";
	if (outfiles[filenum]->bad()) customerrorquit("Unable to write to file");
	outfiles[filenum]->flush();
}

void close_outfile(int filenum) {
	outfiles[filenum]->close();
}

void close_infile(int filenum) {
	infiles[filenum]->close();
}

void close_all() {
	for (auto f : outfiles) f->close();
	for (auto f : infiles) f->close();
}
