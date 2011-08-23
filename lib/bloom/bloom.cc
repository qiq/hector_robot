#include <stdio.h>
#include <vector>
#include "BloomFilter.h"

using namespace std;

int main(int argc, char **argv) {
	vector<uint32_t> words;
	uint32_t data;
	while (fread((void*)data, 4, 1, stdin) == 1) {
		if (data == 0) {
			// process
			words.clear();
		} else {
			words.push_back(data);
		}
	}
	return 0;
}
