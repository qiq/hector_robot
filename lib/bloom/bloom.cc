#include <stdio.h>
#include <vector>

int main() {
	vector<uint32_t> words;
	uint32_t data;
	while (fread(data, 4, 1) == 1) {
		if (data == 0) {
			// process
			words.clear();
		} else {
			word.push_back(data);
		}
	}
}
