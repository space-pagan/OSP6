#include "error_handler.h"

std::string prefix;

void setupprefix(char* arg0) {
	prefix = arg0;
	prefix += ": Error";
}

void perrandquit() {
	perror(prefix.c_str());
	exit(0);
}
