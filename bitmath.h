#pragma once
#include <assert.h>
#include <immintrin.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

void Bprint(const uint8_t *v, size_t n);

typedef __uint128_t uint128_t;
typedef uint128_t uint256_t[2];

/**
 * Compute fast bitwise xor between two bit-vectors a, b of n bits,
 * where n is a multiple of 128.
 * Place the result in a.
 */
static inline
void bitxor_m128i(void *_a, const void *_b, size_t n)
{
  assert(n % 128 == 0);

  __m128i *a = (__m128i *) _a;
  __m128i *b = (__m128i *) _b;

  n >>= 7;
  while (n--) {
    *a = _mm_xor_si128(*a, *b);
    ++a; ++b;
  }
}

/**
 * Compute bitwise xor between to bit-vectors a, b of n bits,
 * where n is a multiple of 8.
 * Place the result in a.
 */
static inline
void bitxor_small(void *_a, const void *_b, size_t n)
{
  assert(n % 8 == 0);

  uint8_t *a = (uint8_t *) _a;
  uint8_t *b = (uint8_t *) _b;

  n >>= 3;
  while (n--) {
    *a ^= *b;
    a++; b++;
  }
}


static inline
void bitxor(void *_a, const void *_b, const size_t n)
{
  assert(n % 8 == 0);

  uint8_t *a = (uint8_t *) _a;
  uint8_t *b = (uint8_t *) _b;
  const size_t small = n % 128;
  const size_t big = n - small;

  bitxor_m128i(a, b,  big);
  bitxor_small(a+big, b+big, small);
}


/**
 * Compute fast bitwise and between two bit-vectors a, b of n bits,
 * where n is a multiple of 128.
 * Place the result in a.
 */
static inline
void bitand(void *_a, const void *_b, size_t n)
{
  __m128i *a = (__m128i *) _a;
  __m128i *b = (__m128i *) _b;
  n >>= 7;
  while (n--) {
    *a = _mm_and_si128(*a, *b);
    ++a; ++b;
  }
}

/**
 * Compute fast equality between two bit-vectors a, b of n bits,
 * where n is a multiple of 128.
 */
static inline
bool biteq(const void *_a, const void *_b, size_t n)
{
  __m128i *a = (__m128i *) _a;
  __m128i *b = (__m128i *) _b;
  __m128i iseq;
  n >>= 7;
  while (n--) {
    iseq = _mm_cmpeq_epi8(*a, *b);
    if (_mm_movemask_epi8(iseq) != 0xffff) {
      return false;
    }
    ++a; ++b;
  }
  return true;
}

static inline
uint8_t getbit(const void *_v, size_t pos)
{
  const uint8_t *v = _v;
  return (v[pos >> 3] & (1 << (pos % 8))) != 0;
}


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
  ((uint8_t *) v)[(bits-1) >> 3] &= (1 << ((bits-1) % 8)) - 1


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
  (bm.M + (i) * bm.offset)


void __sse_trans(uint8_t const *inp, uint8_t *out, int nrows, int ncols);

/**
 * Compute in-place transpose of a matrix of (m x n) bits.
 */
static inline void transpose(bitmatrix_t *dst, bitmatrix_t const * src, size_t m, size_t n)
{
  const uint8_t *A = (uint8_t *) src->M;
  uint8_t *B = (uint8_t *) dst->M;
  __sse_trans(A, B, m, n);
}
