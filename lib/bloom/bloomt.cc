#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
//#define DEBUG
#include "NgramBloomFilter.h"

using namespace std;

int main(int argc, char **argv) {
	NgramBloomFilter bloom(10, 0.5, 17430483, 0.01);
	vector<string> words;
	bool finish;
	string data;
	int doc = 1;
	do {
		finish = getline(cin, data).eof();
		if (finish || data.length() == 0) {
			if (words.size() > 0) {
				// process
				if (bloom.TestDuplicate(words)) {
					printf("DUP %d\n", doc);
					for (int i = 0; i < (int)words.size(); i++) {
						if (i > 0)
							putchar(' ');
						fputs(words[i].c_str(), stdout);
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
