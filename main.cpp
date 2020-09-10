#include <iostream>
#include <string>
#include <stdio.h>
#include "cli_handler.h"
#include "child_handler.h"

using namespace std;

void freestrarray(char**, int);

int main(int argc, char **argv) {
	int pr_limit;
	int pr_count = 0;
	int pr_id = 1;

	if (getcliarg(argc, argv, 'n', pr_limit)) {
		return -1;
	}

	std::string line;
	char** child_args;
	while (std::getline(std::cin, line)) {
		int size;
		line += " " + std::to_string(pr_id);
		pr_id++;
		child_args = makeargv(line, size);
		while (pr_count >= pr_limit) {
			waitforanychild(pr_count);
		}
		forkexec(child_args[0], child_args, pr_count);
		updatechildcount(pr_count);
		freestrarray(child_args, size);
	}

	while (pr_count > 0) {
		waitforanychild(pr_count);
	}
}

void freestrarray(char** array, int size) {
	for (int x = 0; x < size; x++) {
		delete[] array[x];
	}
	delete[] array;
}
