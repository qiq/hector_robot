#ifndef _MODULES_SUPERFASTHASH_H_
#define _MODULES_SUPERFASTHASH_H_

#include <config.h>

#include <stdint.h>

// NB: SuperFastHash has problematic properties when used for e.g. Bloom filter
uint32_t SuperFastHash(const char *data, int len);

#endif
