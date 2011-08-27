/**
  BloomFilter.h
*/

#ifndef _LIB_BLOOMFILTER_H_
#define _LIB_BLOOMFILTER_H_

//#include <config.h>
#include <iostream>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>

// Copy from MurmurHash3.cc

#define	FORCE_INLINE __attribute__((always_inline))

inline uint32_t rotl32(uint32_t x, int8_t r) {
	return (x << r) | (x >> (32 - r));
}

#define	ROTL32(x,y)	rotl32(x,y)

FORCE_INLINE uint32_t fmix(uint32_t h) {
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;

	return h;
}

// only for MurmurHash3_x86_32
FORCE_INLINE uint32_t getblock(const uint32_t *p, int i) {
	return p[i];
}

// original implementation of the MurmurHash3_x86_32
void MurmurHash3_x86_32(const void *key, int len, uint32_t seed, uint32_t *out) {
	const uint8_t * data = (const uint8_t*)key;
	const int nblocks = len / 4;

	uint32_t h1 = seed;

	uint32_t c1 = 0xcc9e2d51;
	uint32_t c2 = 0x1b873593;

	//----------
	// body

	const uint32_t * blocks = (const uint32_t *)(data + nblocks*4);

	for (int i = -nblocks; i; i++) {
		uint32_t k1 = getblock(blocks,i);

		k1 *= c1;
		k1 = ROTL32(k1,15);
		k1 *= c2;

		h1 ^= k1;
		h1 = ROTL32(h1, 13); 
		h1 = h1*5+0xe6546b64;
	}

	//----------
	// tail

	const uint8_t * tail = (const uint8_t*)(data + nblocks*4);

	uint32_t k1 = 0;

	switch (len & 3) {
		case 3: k1 ^= tail[2] << 16;
		case 2: k1 ^= tail[1] << 8;
		case 1: k1 ^= tail[0];
		k1 *= c1; k1 = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
	};

	//----------
	// finalization
  	h1 ^= len;

	h1 = fmix(h1);

	*out = h1;
}

class NgramBloomFilter {
public:
	NgramBloomFilter(int ngram, double duplicateThreshold, uint64_t m, int k);
	NgramBloomFilter(int ngram, double duplicateThreshold, uint64_t n, double false_positive_probability);
	~NgramBloomFilter();

	bool TestDuplicate(std::vector<uint32_t> &values);
	bool TestDuplicateSlow(std::vector<uint32_t> &values);
	bool TestDuplicate(std::vector<std::string> &values);
	bool TestDuplicateSlow(std::vector<std::string> &values);

protected:
	bool Test(uint64_t offset);
	void Set(uint64_t offset);
	bool TestAndSet(uint64_t offset);
	void Reset(uint64_t offset);

	void InitH1(int index);
	void UpdateH1(int index, uint32_t k1);
	void FinishH1(int index);
	void PrintH1();

private:
	// how many n-grams we compute
	int ngram;
	// percent of duplicate n-grams when we consider docs to be duplicates
	double duplicateThreshold;
	// number of bytes to allocate for the data
	uint64_t m;
	// number of hash functions
	int k;

	// bit array of size m
	unsigned char *data;
	// h1 coeficients for computing Murmur hash 3
	uint32_t *h1;
	// keys that were 0 and should be 1 now
	std::vector<uint64_t> set;

	uint32_t *ng;
};

inline NgramBloomFilter::NgramBloomFilter(int ngram, double duplicateThreshold, uint64_t m, int k): ngram(ngram), duplicateThreshold(duplicateThreshold), m(m), k(k) {
	data = new unsigned char[m/8+1];
	memset(data, 0, m/8+1);
	h1 = new uint32_t[(ngram-1)*k*2];
	ng = new uint32_t[ngram];
}

inline NgramBloomFilter::NgramBloomFilter(int ngram, double duplicateThreshold, uint64_t n, double false_positive_probability): ngram(ngram), duplicateThreshold(duplicateThreshold) {
	m = ceil((double)-1*n*log(false_positive_probability)/(log(2)*log(2)));
	k = ((double)m/n)*log(2);
#ifdef DEBUG
	std::cout << "n: " << n << ", m: " << m << ", k: " << k << "\n";
#endif
	data = new unsigned char[m/8+1];
	memset(data, 0, m/8+1);
	h1 = new uint32_t[(ngram-1)*k*2];
	ng = new uint32_t[ngram];
}

inline NgramBloomFilter::~NgramBloomFilter() {
	delete[] data;
	delete[] h1;
	delete[] ng;
}

FORCE_INLINE bool NgramBloomFilter::Test(uint64_t offset) {
	offset %= m;
	uint64_t index = offset >> 3;
	unsigned char mask = 1 << (offset & 0x7);
#ifdef DEBUG
	if (data[index] & mask)
		std::cout << offset << " = 1\n";
	else
		std::cout << offset << " = 0\n";
#endif
	return (data[index] & mask) != 0;
}

FORCE_INLINE void NgramBloomFilter::Set(uint64_t offset) {
	offset %= m;
	uint64_t index = offset >> 3;
	unsigned char mask = 1 << (offset & 0x7);
	data[index] |= mask;
}

FORCE_INLINE bool NgramBloomFilter::TestAndSet(uint64_t offset) {
	offset %= m;
	uint64_t index = offset >> 3;
	unsigned char mask = 1 << (offset & 0x7);
	if ((data[index] & mask) == 0) {
		data[index] |= mask;
#ifdef DEBUG
		std::cout << offset << " = 0\n";
#endif
		return false;
	}
#ifdef DEBUG
	std::cout << offset << " = 1\n";
#endif
	return true;
}

FORCE_INLINE void NgramBloomFilter::Reset(uint64_t offset) {
	offset %= m;
	uint64_t index = offset >> 3;
	unsigned char mask = 1 << (offset & 0x7);
	data[index] &= (mask ^ 0xFF);
}

inline void NgramBloomFilter::InitH1(int index) {
	uint32_t *h1p = h1+index*k*2;
	for (int i = 0; i < k*2; i++) {
		*h1p = i;
		h1p++;
	}
}

FORCE_INLINE void NgramBloomFilter::UpdateH1(int index, uint32_t k1) {
	uint32_t *h1p = h1+(index+1)*k*2-1;
	switch (k) {
	case 16:
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
	case 15:
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
	case 14:
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
	case 13:
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
	case 12:
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
	case 11:
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
	case 10:
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
	case 9:
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
	case 8:
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
	case 7:
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
	case 6:
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
	case 5:
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
	case 4:
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
	case 3:
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
	case 2:
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
	case 1:
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
		*h1p ^= k1;
		*h1p = ROTL32(*h1p, 13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
	}
}

FORCE_INLINE void NgramBloomFilter::FinishH1(int index) {
	uint32_t *h1p = h1+(index+1)*k*2-1;
	switch (k) {
	case 16:
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
	case 15:
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
	case 14:
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
	case 13:
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
	case 12:
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
	case 11:
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
	case 10:
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
	case 9:
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
	case 8:
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
	case 7:
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
	case 6:
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
	case 5:
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
	case 4:
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
	case 3:
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
	case 2:
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
	case 1:
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
		*h1p ^= 4*ngram;
  		*h1p = fmix(*h1p);
		h1p--;
	}
}

void NgramBloomFilter::PrintH1() {
	for (int i = 0; i < ngram-1; i++) {
		uint32_t *h1p = h1+i*k*2;
		for (int j = 0; j < k*2; j++) {
			printf("%x ", *(h1p+j));
		}
		printf("\n");
	}
	printf("\n");
}

bool NgramBloomFilter::TestDuplicate(std::vector<uint32_t> &values) {
	const uint32_t c1 = 0xcc9e2d51;
	const uint32_t c2 = 0x1b873593;

	// initialize n-gram buffer: fill-in h1 seeds
	for (int i = 0; i < ngram-1; i++)
		InitH1(i);

	set.clear();

	int valuesSize = values.size();
	int size = 0;
	int current = 0;
	int duplicates = 0;
	for (int i = 0; i < valuesSize; i++) {
		uint32_t k1 = values[i];
		k1 *= c1;
		k1 = ROTL32(k1, 15);
		k1 *= c2;
		for (int j = 0; j < ngram-1 && j <= size; j++)
			UpdateH1(j, k1);

		// we have seen at least ngram-1 words?
		if (size == ngram-1) {
			// finish position at bufferStart
			FinishH1(current);
			int total = 0;
			uint64_t key;
			switch (k) {
			case 16:
				key = *(uint64_t*)(h1+current*k*2+15*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 15:
				key = *(uint64_t*)(h1+current*k*2+14*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 14:
				key = *(uint64_t*)(h1+current*k*2+13*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 13:
				key = *(uint64_t*)(h1+current*k*2+12*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 12:
				key = *(uint64_t*)(h1+current*k*2+11*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 11:
				key = *(uint64_t*)(h1+current*k*2+10*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 10:
				key = *(uint64_t*)(h1+current*k*2+9*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 9:
				key = *(uint64_t*)(h1+current*k*2+8*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 8:
				key = *(uint64_t*)(h1+current*k*2+7*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 7:
				key = *(uint64_t*)(h1+current*k*2+6*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 6:
				key = *(uint64_t*)(h1+current*k*2+5*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 5:
				key = *(uint64_t*)(h1+current*k*2+4*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 4:
				key = *(uint64_t*)(h1+current*k*2+3*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 3:
				key = *(uint64_t*)(h1+current*k*2+2*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 2:
				key = *(uint64_t*)(h1+current*k*2+1*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 1:
				key = *(uint64_t*)(h1+current*k*2+0*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			}
			// all bits are 1 => duplicate
			if (k == total) {
				duplicates++;

				if ((double)duplicates/(valuesSize-ngram+1) > duplicateThreshold)
					return true;
			}

			InitH1(current);
			UpdateH1(current, k1);
			current = (current+1) % (ngram-1);
		} else {
			size++;
		}
	}

	for (std::vector<uint64_t>::iterator iter = set.begin(); iter != set.end(); ++iter)
		Set(*iter);

	return false;
}

bool NgramBloomFilter::TestDuplicate(std::vector<std::string> &values) {
	const uint32_t c1 = 0xcc9e2d51;
	const uint32_t c2 = 0x1b873593;

	// initialize n-gram buffer: fill-in h1 seeds
	for (int i = 0; i < ngram-1; i++)
		InitH1(i);

	set.clear();

	int valuesSize = values.size();
	int size = 0;
	int current = 0;
	int duplicates = 0;
	for (int i = 0; i < valuesSize; i++) {
		uint32_t k1;
		std::string *s = &values[i];
		MurmurHash3_x86_32(s->data(), s->length(), 0, &k1);
		k1 *= c1;
		k1 = ROTL32(k1, 15);
		k1 *= c2;
		for (int j = 0; j < ngram-1 && j <= size; j++)
			UpdateH1(j, k1);

		// we have seen at least ngram-1 words?
		if (size == ngram-1) {
			// finish position at bufferStart
			FinishH1(current);
			int total = 0;
			uint64_t key;
			switch (k) {
			case 16:
				key = *(uint64_t*)(h1+current*k*2+15*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 15:
				key = *(uint64_t*)(h1+current*k*2+14*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 14:
				key = *(uint64_t*)(h1+current*k*2+13*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 13:
				key = *(uint64_t*)(h1+current*k*2+12*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 12:
				key = *(uint64_t*)(h1+current*k*2+11*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 11:
				key = *(uint64_t*)(h1+current*k*2+10*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 10:
				key = *(uint64_t*)(h1+current*k*2+9*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 9:
				key = *(uint64_t*)(h1+current*k*2+8*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 8:
				key = *(uint64_t*)(h1+current*k*2+7*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 7:
				key = *(uint64_t*)(h1+current*k*2+6*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 6:
				key = *(uint64_t*)(h1+current*k*2+5*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 5:
				key = *(uint64_t*)(h1+current*k*2+4*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 4:
				key = *(uint64_t*)(h1+current*k*2+3*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 3:
				key = *(uint64_t*)(h1+current*k*2+2*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 2:
				key = *(uint64_t*)(h1+current*k*2+1*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			case 1:
				key = *(uint64_t*)(h1+current*k*2+0*2);
				if (Test(key))
					total++;
				else
					set.push_back(key);
			}
			// all bits are 1 => duplicate
			if (k == total) {
				duplicates++;

				if ((double)duplicates/(valuesSize-ngram+1) > duplicateThreshold)
					return true;
			}

			InitH1(current);
			UpdateH1(current, k1);
			current = (current+1) % (ngram-1);
		} else {
			size++;
		}
	}

	for (std::vector<uint64_t>::iterator iter = set.begin(); iter != set.end(); ++iter)
		Set(*iter);

	return false;
}

bool NgramBloomFilter::TestDuplicateSlow(std::vector<uint32_t> &values) {
	int duplicates = 0;
	for (int i = ngram-1; i < (int)values.size(); i++) {
		for (int j = 0; j < ngram; j++)
			ng[j] = values[i-ngram+1+j];
		int ones = 0;
		for (int k2 = k-1; k2 >= 0; k2--) {
			uint32_t hash[2];
			MurmurHash3_x86_32(ng, ngram*4, k2*2+0, &hash[0]);
			MurmurHash3_x86_32(ng, ngram*4, k2*2+1, &hash[1]);
			uint64_t offset = hash[1];
			offset = (offset << 32 | hash[0]) % m;
			uint64_t index = offset >> 3;
			unsigned char mask = 1 << (offset & 0x7);
			if ((data[index] & mask) != 0) {
#ifdef DEBUG
				std::cout << offset << " = 1\n";
#endif
				ones++;
			} else {
#ifdef DEBUG
				std::cout << offset << " = 0\n";
#endif
			}
		}
		if (ones == k) {
			duplicates++;

			if ((double)duplicates/(values.size()-ngram+1) > duplicateThreshold)
				return true;
		}
	}

	for (int i = ngram-1; i < (int)values.size(); i++) {
		for (int j = 0; j < ngram; j++)
			ng[j] = values[i-ngram+1+j];
		for (int k2 = k-1; k2 >= 0; k2--) {
			uint32_t hash[2];
			MurmurHash3_x86_32(ng, ngram*4, k2*2+0, &hash[0]);
			MurmurHash3_x86_32(ng, ngram*4, k2*2+1, &hash[1]);
			uint64_t offset = hash[1];
			offset = (offset << 32 | hash[0]) % m;
			uint64_t index = offset >> 3;
			unsigned char mask = 1 << (offset & 0x7);
			data[index] |= mask;
		}
	}

	return false;
}

bool NgramBloomFilter::TestDuplicateSlow(std::vector<std::string> &values) {
	int duplicates = 0;
	for (int i = ngram-1; i < (int)values.size(); i++) {
		for (int j = 0; j < ngram; j++) {
			std::string *s = &values[i-ngram+1+j];
			MurmurHash3_x86_32(s->data(), s->length(), 0, &ng[j]);
		}
		int ones = 0;
		for (int k2 = k-1; k2 >= 0; k2--) {
			uint32_t hash[2];
			MurmurHash3_x86_32(ng, ngram*4, k2*2+0, &hash[0]);
			MurmurHash3_x86_32(ng, ngram*4, k2*2+1, &hash[1]);
			uint64_t offset = hash[1];
			offset = (offset << 32 | hash[0]) % m;
			uint64_t index = offset >> 3;
			unsigned char mask = 1 << (offset & 0x7);
			if ((data[index] & mask) != 0) {
#ifdef DEBUG
				std::cout << offset << " = 1\n";
#endif
				ones++;
			} else {
#ifdef DEBUG
				std::cout << offset << " = 0\n";
#endif
			}
		}
		if (ones == k) {
			duplicates++;

			if ((double)duplicates/(values.size()-ngram+1) > duplicateThreshold)
				return true;
		}
	}

	for (int i = ngram-1; i < (int)values.size(); i++) {
		for (int j = 0; j < ngram; j++) {
			std::string *s = &values[i-ngram+1+j];
			MurmurHash3_x86_32(s->data(), s->length(), 0, &ng[j]);
		}
		for (int k2 = k-1; k2 >= 0; k2--) {
			uint32_t hash[2];
			MurmurHash3_x86_32(ng, ngram*4, k2*2+0, &hash[0]);
			MurmurHash3_x86_32(ng, ngram*4, k2*2+1, &hash[1]);
			uint64_t offset = hash[1];
			offset = (offset << 32 | hash[0]) % m;
			uint64_t index = offset >> 3;
			unsigned char mask = 1 << (offset & 0x7);
			data[index] |= mask;
		}
	}

	return false;
}

#endif
