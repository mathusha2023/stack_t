#ifndef HASH_H
#define HASH_H

#include <stddef.h>

const size_t NUM_BUCKETS = 18446744073709551557ULL;

size_t hash(const char *s, size_t size);

#endif // HASH_H
