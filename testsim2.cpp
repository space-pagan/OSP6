/* Author: Zoya Samsonov
 * Date: September 11, 2020
 */

#include <iostream>
#include <stdexcept>
#include <unistd.h>

int main(int argc, char *argv[]) {
	// simple program to test proc_fan, which when run as 'testsim2 a b'
	// will perform 'a' loops, during which it sleeps for 'b' seconds
	// and finally outputs its PID and the time it slept to stderr
	if (argc != 3) {
		std::cerr << "testsim started with an invalid (" << argc << ") number of arguments\n";
		return 0;
	}
	int loops, sleeptime;
	try {
		loops = std::stoi(argv[1]);
		sleeptime = std::stoi(argv[2]);
	} catch (const std::invalid_argument& ia) {
		std::cerr << "testsim started with non-integer argument(s)\n";
		return 0;
	}

	while (loops--) {
		sleep(sleeptime);
		std::cerr << getpid() << " (slept " << sleeptime << "s)\n";
	}
}
