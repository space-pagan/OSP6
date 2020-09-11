#include <iostream>
#include <stdexcept>
#include <unistd.h>

using namespace std;

int main(int argc, char *argv[]) {
	int i = 0;
	for (; i < argc-1; i++) {
		cerr << argv[i] << " ";
	}
	cerr << argv[i] << "\n";
	return 0;
}
