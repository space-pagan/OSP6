/* Author: Zoya Samsonov
 * Date: September 11, 2020
 */

#include <iostream>
#include <stdexcept>
#include <unistd.h>

int main(int argc, char *argv[]) {
	// simple program for testing proc_fan, which echoes
	// all arguments it was called with to stderr
	int i = 0;
	for (; i < argc-1; i++) {
		std::cerr << argv[i] << " ";
	}
	std::cerr << argv[i] << "\n";
	return 0;
}
