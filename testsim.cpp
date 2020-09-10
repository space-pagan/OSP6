#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string>

using namespace std;

int main(int argc, char *argv[]) {
	srand(time(NULL) * std::stoi(argv[argc-1]));
	int i = 0;
	for (; i < argc-1; i++) {
		cout << argv[i] << " ";
	}
	int sleepdur = rand() % 9 + 1;
	sleep(sleepdur);
	cout << "#" << argv[i] << "\t(Slept for " << sleepdur << "s)\n";
	return 0;
}
