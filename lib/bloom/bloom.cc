#include <stdio.h>
#include <vector>
#define DEBUG
#include "NgramBloomFilter.h"

using namespace std;

int main(int argc, char **argv) {
	NgramBloomFilter bloom(3, 20, 100, 0.01);
	vector<uint32_t> words;
	uint32_t data;
	bool finish;
	do {
		finish = fread((void*)&data, 4, 1, stdin) != 1;
		if (finish || data == 0) {
			// proces
//bloom.TestDuplicateSlow(words);
			if (bloom.TestDuplicate(words)) {
				fprintf(stderr, "dup\n");
			} else {
				fprintf(stderr, "!dup\n");
			}
			words.clear();
		} else {
			words.push_back(data);
		}
	} while (!finish);
	return 0;
}
