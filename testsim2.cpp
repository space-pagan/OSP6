#include <iostream>
#include <stdexcept>
#include <unistd.h>

using namespace std;

int main(int argc, char *argv[]) {
	if (argc != 3) {
		cerr << "testsim started with an invalid (" << argc << ") number of arguments\n";
		return 0;
	}
	int loops, sleeptime;
	try {
		loops = stoi(argv[1]);
		sleeptime = stoi(argv[2]);
	} catch (const std::invalid_argument& ia) {
		cerr << "testsim started with non-integer argument(s)\n";
		return 0;
	}

	while (loops--) {
		sleep(sleeptime);
		cerr << getpid() << " (slept " << sleeptime << "s)\n";
	}
}
