#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

typedef __uint128_t uint128_t;
typedef uint128_t uint256_t[2];

void Bprint(const uint8_t *v, size_t n);
void transpose(void *dst, const void *src, size_t m, size_t n);
void bitxor(void *_a, const void *_b, size_t n);
void bitxor_small(void *_a, const void *_b, size_t n);
void bitand(void *_a, const void *_b, size_t n);
bool biteq(const void *_a, const void *_b, size_t n);
uint8_t getbit(const void *_v, size_t pos);


/**
 * The minimum number of octets needed in order to store `bits`.
 */
//#define octs(bits)  (bits >> 3)
#define octs(bits)    (((bits) >> 3) + ((bits) % 8 != 0))


/**
 * Set to zero (respectively, one) the vector `dest` long `oct(bits)`.
 */
#define bitset_zero(dest, bits) memset(dest, 0, octs(bits))
#define bitset_one(dest, bits)  memset(dest, 0xff, octs(bits))

/**
 * Wrappers around standard library malloc/strcpy, but working on bits.
 * This with the purpose to make the rest of the program readable,
 * and allow me to think always in terms of bits.
 */
#define bitcpy(dest, src, bits) memcpy(dest, src, octs(bits))
// XXX. TODO: make this a call to malloc() instead, we shouldn't need initialization.
#define bitalloc(bits)          calloc(octs(bits), sizeof(uint8_t))

#define bitmask(v, bits)                                        \
  ((uint8_t *) v)[(bits) >> 3] &= (1 << ((bits) % 8)) - 1


typedef struct bitmatrix {
  uint8_t *M;
  size_t offset;
} bitmatrix_t;

#define new_bitmatrix(rows, cols)               \
  (bitmatrix_t) {                               \
    .M = malloc(rows * octs(cols)),             \
    .offset = octs(cols),                       \
  }

#define free_bitmatrix(NAME)                    \
  free(NAME.M)

#define row(bm, i)                              \
  bm.M + (i) * bm.offset
