#ifndef HASHING_H
#define HASHING_H

#include <string.h>
#include <inttypes.h>

uint32_t murmur3_32(const uint8_t* key, size_t len, uint32_t seed);

#endif // HASHING_H
