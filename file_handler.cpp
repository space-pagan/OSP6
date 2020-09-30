/* Author: Zoya Samsonov
 * Date: September 30, 2020
 */

#include "file_handler.h"

char** getfilelines(const char* filename, int& size, int& lines) {
	std::ifstream file(filename);
	std::string line;
	std::vector<std::string> filelines;

	// import lines into string vector
	while (std::getline(file, line)) {
		filelines.push_back(line);
	}

	size = 0;
	lines = filelines.size();
	char** out = new char*[lines];

	// if file is not empty, save to out
	if (lines) {
		for (int i = 0; i < filelines.size() - 1; i++) {
			out[i] = new char[filelines[i].size()];
			size += filelines[i].size();
			strcpy(out[i], filelines[i].c_str());
		}
	}

	return out;
}

