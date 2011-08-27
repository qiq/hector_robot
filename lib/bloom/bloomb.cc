#include <stdio.h>
#include <vector>
#define DEBUG
#include "NgramBloomFilter.h"

using namespace std;

int main(int argc, char **argv) {
	NgramBloomFilter bloom(10, 0.5, 17430483, 0.01);
	vector<uint32_t> words;
	uint32_t data;
	bool finish;
	int doc = 1;
	do {
		finish = fread((void*)&data, 4, 1, stdin) != 1;
		if (finish || data == 0) {
			if (words.size() > 0) {
				// proces
				//bloom.TestDuplicateSlow(words);
				if (bloom.TestDuplicate(words)) {
					printf("DUP %d\n", doc);
					for (int i = 0; i < (int)words.size(); i++) {
						if (i > 0)
							putchar(' ');
						printf("%d", words[i]);
					}
					putchar('\n');
				}
			}
			doc++;
			words.clear();
		} else {
			words.push_back(data);
		}
	} while (!finish);

	return 0;
}
