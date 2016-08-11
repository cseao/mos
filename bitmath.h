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
void bitand(void *_a, const void *_b, size_t n);
bool biteq(const void *_a, const void *_b, size_t n);
uint8_t getbit(const void *_v, size_t pos);


#define bitset_zero(dest, bits) memset(dest, 0, bits >> 3)
#define bitset_one(dest, bits)  memset(dest, 0xff, bits >> 3)
#define bitcpy(dest, src, bits) memcpy(dest, src, bits >> 3)
#define bitalloc(bits)          calloc(bits >> 3, sizeof(uint8_t))
#define octs(bits)              bits >> 3
#define u8(bits)                bits >> 3
#define bitcell(M, row, col)    ((const uint8_t *) M + row * (col >> 3))

typedef struct bitmatrix {
  uint8_t *M;
  size_t offset;
} bitmatrix_t;

#define new_bitmatrix(rows, cols)               \
  (bitmatrix_t) {                               \
    .M = bitalloc(rows * cols),                 \
    .offset = octs(cols),                       \
  }

#define free_bitmatrix(NAME)                    \
  free(NAME.M)
#define row(bitmatrix, i)                       \
  bitmatrix.M + (i) * bitmatrix.offset
