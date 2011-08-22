/**
  BloomFilter.h
*/

#ifndef _LIB_BLOOMFILTER_H_
#define _LIB_BLOOMFILTER_H_

#include <config.h>
#include <stdint.h>
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

/*
void MurmurHash3_x86_32  ( const void * key, int len, uint32_t seed, void * out );

void MurmurHash3_x86_32 ( const void * key, int len,
                          uint32_t seed, void * out )
{
  const uint8_t * data = (const uint8_t*)key;
  const int nblocks = len / 4;

  uint32_t h1 = seed;

  uint32_t c1 = 0xcc9e2d51;
  uint32_t c2 = 0x1b873593;

  //----------
  // body

  const uint32_t * blocks = (const uint32_t *)(data + nblocks*4);

  for(int i = -nblocks; i; i++)
  {
    uint32_t k1 = getblock(blocks,i);

    k1 *= c1;
    k1 = ROTL32(k1,15);
    k1 *= c2;
    
    h1 ^= k1;
    h1 = ROTL32(h1,13); 
    h1 = h1*5+0xe6546b64;
  }

  //----------
  // tail

  const uint8_t * tail = (const uint8_t*)(data + nblocks*4);

  uint32_t k1 = 0;

  switch(len & 3)
  {
  case 3: k1 ^= tail[2] << 16;
  case 2: k1 ^= tail[1] << 8;
  case 1: k1 ^= tail[0];
          k1 *= c1; k1 = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
  };

  //----------
  // finalization

  h1 ^= len;

  h1 = fmix(h1);

  *(uint32_t*)out = h1;
} 

*/

class NgramBloomFilter {
public:
	NgramBloomFilter(uint64_t m, int k, int ngram, int duplicateThreshold);
	~NgramBloomFilter();

	bool TestDuplicate(std::vector<uint32_t> &values);
//	bool TestDuplicate(std::vector<std::string> &values);

protected:
	bool TestAndSet(uint64_t offset);
	void Reset(uint64_t offset);

	void InitH1(int ngram);
	void UpdateH1(int index, uint32_t k1);
	void FinishH1(int index);

private:
	// bit array of size m
	unsigned char *data;
	// number of bytes to allocate for the data
	uint64_t m;
	// number of hash functions
	int k;

	// how many n-grams we compute
	int ngram;
	// percent of duplicate n-grams when we consider docs to be duplicates
	int duplicateThreshold;
	// h1 coeficients for computing Murmur hash 3
	uint32_t *h1;
	// keys that were 0 and are 1 now -> in case of reset, we need to clear them
	vector<uint64_t> reset;
};

inline NgramBloomFilter::NgramBloomFilter(int ngram, int duplicateThreshold, uint64_t m, int k): ngram(ngram), duplicateThreshold(duplicateThreshold), m(m), k(k) {
	data = new unsigned char[m/8];
	h1 = new uint32_t[(ngram-1)*k*2];
}

inline NgramBloomFilter::NgramBloomFilter(int ngram, int duplicateThreshold, uint64_t n, double false_positive_probability): ngram(ngram), duplicateThreshold(duplicateThreshold) {
- zvolime m = -n * ln(p)/ln(2)^2
- zvolime k = m/n*ln2

- zvolime optimalni k

	double min_m = std::numeric_limits<double>::infinity();
	double min_k = 0.0;
	double curr_m = 0.0;
	for (int curr_k = 0; curr_k < 8; curr_k++) {
		if ((curr_m = ((-curr_k*n)/std::log(1.0-std::pow(false_positive_probability, 1.0/curr_k)))) < min_m) {
-k*n/log(1-fp^1/k)
			min_m = curr_m;
			min_k = curr_k;
		}
	}
	k = (int)min_k;
	m = (uint64_t)min_m;
	m += 8-((m%8) & 0x7);

	data = new unsigned char[m/8];
	h1 = new uint32_t[(ngram-1)*k*2];
}

///
   void find_optimal_parameters()
   {
      /*
        Note:
        The following will attempt to find the number of hash functions
        and minimum amount of storage bits required to construct a bloom
        filter consistent with the user defined false positive probability
        and estimated element insertion count.
      */
   
      double min_m = std::numeric_limits<double>::infinity();
      double min_k = 0.0;
      double curr_m = 0.0;
      for(double k = 0.0; k < 1000.0; ++k)
      {
         if ((curr_m = ((- k * predicted_element_count_) / std::log(1.0 - std::pow(desired_false_positive_probability_, 1.0 / k)))) < min_m)
         {
            min_m = curr_m;
            min_k = k;
         }
      }
     
      salt_count_ = static_cast<std::size_t>(min_k);
      table_size_ = static_cast<std::size_t>(min_m);
      table_size_ += (((table_size_ % bits_per_char) != 0) ? (bits_per_char - (table_size_ % bits_per_char)) : 0);
   }            



// TODO: constructor with just N -- length of the data and target probability

inline NgramBloomFilter::~NgramBloomFilter {
	delete[] data;
	delete[] h1;
}

inline bool NgramBloomFilter::TestAndSet(uint64_t offset) {
	int index = offset >> 3;
	unsigned char mask = 1 << (offset & 0x7);
	if (data[index] & mask == 0) {
		data[index] |= mask;
		return false;
	}
	return true;
}

inline void NgramBloomFilter::Reset(uint64_t offset) {
	int index = offset >> 3;
	unsigned char mask = 1 << (offset & 0x7);
	data[index] &= (mask ^ 0xFF);
}

inline void NgramBloomFilter::InitH1(int ngram) {
	uint32_t *h1p = h1+index*k*2;
	int x = index*k*2;
	for (int i = 0; i < k*2 i++)
		*h1p = x++;;
}

inline void NgramBloomFilter::UpdateH1(int index, uint32_t k1) {
	uint32_t *h1p = h1+(index+1)*k*2-1;
	switch (k) {
	case 8:
		*h1p ^= k1;
		*h1p = ROTL32(*h1p,13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
		*h1p ^= k1;
		*h1p = ROTL32(*h1p,13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
	case 7:
		*h1p ^= k1;
		*h1p = ROTL32(*h1p,13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
		*h1p ^= k1;
		*h1p = ROTL32(*h1p,13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
	case 6:
		*h1p ^= k1;
		*h1p = ROTL32(*h1p,13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
		*h1p ^= k1;
		*h1p = ROTL32(*h1p,13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
	case 5:
		*h1p ^= k1;
		*h1p = ROTL32(*h1p,13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
		*h1p ^= k1;
		*h1p = ROTL32(*h1p,13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
	case 4:
		*h1p ^= k1;
		*h1p = ROTL32(*h1p,13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
		*h1p ^= k1;
		*h1p = ROTL32(*h1p,13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
	case 3:
		*h1p ^= k1;
		*h1p = ROTL32(*h1p,13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
		*h1p ^= k1;
		*h1p = ROTL32(*h1p,13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
	case 2:
		*h1p ^= k1;
		*h1p = ROTL32(*h1p,13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
		*h1p ^= k1;
		*h1p = ROTL32(*h1p,13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
	case 1:
		*h1p ^= k1;
		*h1p = ROTL32(*h1p,13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
		*h1p ^= k1;
		*h1p = ROTL32(*h1p,13); 
		*h1p = (*h1p)*5+0xe6546b64;
		h1p--;
	}
}

inline void NgramBloomFilter::FinishH1(int index) {
	uint32_t *h1p = h1+(index+1)*k*2-1;
	switch (k) {
	case 8:
		*h1p ^= 4;
  		*h1p = fmix(*h1p);
		h1p--;
		*h1p ^= 4;
  		*h1p = fmix(*h1p);
		h1p--;
	case 7:
		*h1p ^= 4;
  		*h1p = fmix(*h1p);
		h1p--;
		*h1p ^= 4;
  		*h1p = fmix(*h1p);
		h1p--;
	case 6:
		*h1p ^= 4;
  		*h1p = fmix(*h1p);
		h1p--;
		*h1p ^= 4;
  		*h1p = fmix(*h1p);
		h1p--;
	case 5:
		*h1p ^= 4;
  		*h1p = fmix(*h1p);
		h1p--;
		*h1p ^= 4;
  		*h1p = fmix(*h1p);
		h1p--;
	case 4:
		*h1p ^= 4;
  		*h1p = fmix(*h1p);
		h1p--;
		*h1p ^= 4;
  		*h1p = fmix(*h1p);
		h1p--;
	case 3:
		*h1p ^= 4;
  		*h1p = fmix(*h1p);
		h1p--;
		*h1p ^= 4;
  		*h1p = fmix(*h1p);
		h1p--;
	case 2:
		*h1p ^= 4;
  		*h1p = fmix(*h1p);
		h1p--;
		*h1p ^= 4;
  		*h1p = fmix(*h1p);
		h1p--;
	case 1:
		*h1p ^= 4;
  		*h1p = fmix(*h1p);
		h1p--;
		*h1p ^= 4;
  		*h1p = fmix(*h1p);
		h1p--;
	}
}

bool NgramBloomFilter::TestDuplicate(std::vector<uint32_t> &values) {
	const uint32_t c1 = 0xcc9e2d51;
	const uint32_t c2 = 0x1b873593;

	// initialize n-gram buffer: fill-in h1 seeds
	for (int i = 0; i < ngram-1; i++)
		InitH1(h1, k, i);

	reset.clear();

	int valuesSize = values.size();
	int size = 0;
	int current = 0;
	for (int i = 0; i < valuesSize; i++) {
		uint32_t k1 = values[i];
		k1 *= c1;
		k1 = ROTL32(k1,15);
		k1 *= c2;
		for (int j = 0; j < ngram-1; j++)
			UpdateH1(j, k1);

		// we have seen at least ngram-1 words?
		if (size == ngram-1) {
			// finish position at bufferStart
			FinishH1(current);
			int total = 0;
			uint64_t key;
			switch (k) {
			case 8:
				key = *(uint64_t*)(h1+current*k*2+7*2);
				if (TestAndSet(key))
					total++;
				else
					reset.push_back(key);
			case 7:
				key = *(uint64_t*)(h1+current*k*2+6*2);
				if (TestAndSet(key))
					total++;
				else
					reset.push_back(key);
			case 6:
				key = *(uint64_t*)(h1+current*k*2+5*2);
				if (TestAndSet(key))
					total++;
				else
					reset.push_back(key);
			case 5:
				key = *(uint64_t*)(h1+current*k*2+4*2);
				if (TestAndSet(key))
					total++;
				else
					reset.push_back(key);
			case 4:
				key = *(uint64_t*)(h1+current*k*2+3*2);
				if (TestAndSet(key))
					total++;
				else
					reset.push_back(key);
			case 3:
				key = *(uint64_t*)(h1+current*k*2+2*2);
				if (TestAndSet(key))
					total++;
				else
					reset.push_back(key);
			case 2:
				key = *(uint64_t*)(h1+current*k*2+1*2);
				if (TestAndSet(key))
					total++;
				else
					reset.push_back(key);
			case 1:
				key = *(uint64_t*)(h1+current*k*2+0*2);
				if (TestAndSet(key))
					total++;
				else
					reset.push_back(key);
			}
			// all bits are 1 => duplicate
			if (k == total)
				duplicates++;

			if ((double)duplicates/valuesSize*100 > duplicateThreshold) {
				for (std::vector<uint64_t>::iterator iter = reset.begin(); iter != reset.end(); ++iter) {
					Reset(*iter);
				}
				return false;
			}

			InitH1(current);
			UpdateH1(current, k1);
			current = (current+1) % (ngram-1);
		} else {
			size++;
		}
	}
}

#endif
