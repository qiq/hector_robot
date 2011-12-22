/**
	Do a random permutation on the sentences in the file.
	It reads entire file into the memory (using mmap), so it may fail if
	file is too large.

	g++ -Wall -o shuffle_sentences shuffle_sentences.cc
 */
#include <assert.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string.h>

using namespace std;

unsigned rand8() {
	static unsigned const limit = RAND_MAX - RAND_MAX % 256;
	unsigned result = rand();
	while (result >= limit) {
		result = rand();
	}
	return result % 256;
}

uint64_t rand64() {
	uint64_t result = 0ULL;
	for (int count = 0; count < 8; count++)
		result = 256U*result + rand8();
	return result;
}

void perm(vector<uint64_t> &v) {
	uint64_t len = v.size();
	for (int64_t i = len-1; i >= 0; i--) {
		uint64_t j = rand64() % len;
		uint64_t tmp = v[i];
		v[i] = v[j];
		v[j] = tmp;
	}
}

vector<uint64_t> *sentence_boundaries(void *mm, uint64_t len) {
	vector<uint64_t> *s = new vector<uint64_t>();
	s->reserve(1024*1024);
	char *data = (char*)mm;
	uint64_t index = 0;
	bool first = true;
	uint64_t last_word = 0;
	while (index < len) {
		char *nl = (char*)memchr(data+index, '\n', len-index);
		char *start = data+index;
		if (start[0] == '<') {
			if (start[1] == 's') {
				// sentence start
				if (!first)
					s->push_back(last_word);
				else
					first = false;
				s->push_back(index);
				last_word = (nl-data)+1;
			} else if (start[1] != 'd' && start[1] != 'p') {
				last_word = (nl-data)+1;
			}
		} else {
			last_word = (nl-data)+1;
		}
		index = nl-data+1;
	}
	if (!first)
		s->push_back(last_word);
	return s;
}

int main(int argc, char **argv) {
	srand(3434);

	if (argc != 3) {
		cerr << "usage: shuffle_sentences file_in.vert perm_file >file_out.vert\n";
		exit(1);
	}

	int fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		cerr << "Cannot open file: " << argv[1] << "\n";
		exit(1);
	}

	off_t len = lseek(fd, 0, SEEK_END);
	if (len == -1) {
		cerr << "Cannot seek\n";
		exit(1);
	}

	void *mm = mmap(NULL, len, PROT_READ, MAP_SHARED, fd, 0);
	if (mm == (void*)-1) {
		cerr << "Cannot mmap\n";
		exit(1);
	}

	// get boundaries
	vector<uint64_t> *s = sentence_boundaries(mm, len);

	// create random permutation
	uint64_t n = s->size()/2;
	assert(s->size() % 2 == 0);
	vector<uint64_t> p(n);
	for (uint64_t i = 0; i < n; i++)
		p[i] = i;
	perm(p);

	// write permutation
	FILE *f = fopen(argv[2], "w");
	if (!f) {
		cerr << "Cannot open file: " << argv[2] << "\n";
		exit(1);
	}
	for (uint64_t i = 0; i < n; i++)
		fprintf(f, "%lu\n", p[i]);
	fclose(f);

	// write sentences
	char *data = (char*)mm;
	for (uint64_t i = 0; i < n; i++) {
		uint64_t id = p[i];
		uint64_t offset = (*s)[id*2];
		uint64_t size = (*s)[id*2+1]-offset;
		fwrite(data+offset, size, 1, stdout);
	}

	delete s;
	munmap(mm, len);

	close(fd);
}
