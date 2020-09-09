#include <iostream>
using namespace std;

int main(int argc, char *argv[]) {
	int i = 0;
	for (; i < argc-1; i++) {
		cout << argv[i] << " ";
	}
	cout << argv[i] << "\n";
	return 0;
}
