/* Author: Zoya Samsonov
 * Date: September 28, 2020
 */

#include <iostream>
#include <string.h>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <chrono>
#include <ctime>

bool testPalindrome(char* testString);
char* sanitizeStr(char* testString);
char* getCurrentCTime();

int main(int argc, char **argv) {
	// int id = std::stoi(argv[2]);
	srand(getpid());

	bool isPalindrome = testPalindrome(sanitizeStr(argv[1]));
	std::cout << argv[1] << " is ";
	if (!isPalindrome) {
		std::cout << "not ";
	}
	std::cout << "a palindrome!\n";
	// attempt to enter critical section
	std::cerr << "Beginning critical section at: ";
	std::cerr << getCurrentCTime() << "\n";
	// sleep for a random amount of time [0-2] seconds.
	sleep(rand() % 3);

	// Critical Section
	std::cerr << "In critical section at:        ";
	std::cerr << getCurrentCTime() << "\n";

	// exit critical section
	std::cerr << "Left critical section at:      ";
	std::cerr << getCurrentCTime() << "\n";
	return 0;
}

char* getCurrentCTime() {
	auto time_point = std::chrono::system_clock::now();
	std::time_t time_c = std::chrono::system_clock::to_time_t(time_point);
	return std::ctime(&time_c);
}

char* sanitizeStr(char* testString) {
	int len = strlen(testString);
	char* sanitary = new char[len];
	int j = 0;
	for (int i = 0; i < len; i++) {
		char c = testString[i];
		if (((48 <= c) && (c <= 57)) || ((65 <= c) && (c <= 90)) 
				|| ((97 <= c) && (c <= 122))) {
			// 0-9A-Za-z
			sanitary[j++] = c;
		}
	}
	sanitary[j] = '\0';
	return sanitary;
}

bool testPalindrome(char* testString) {
	int len = strlen(testString);
	char* reversed = new char[len];
	for (int i = 0; i < len; i++) {
		reversed[i] = testString[len-i-1];
	}
	if (strcmp(testString, reversed) == 0) {
		return true;
	}
	return false;
}
