#include <immintrin.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "bitmath.h"

void print_matrix(void *_m)
{
  uint8_t *m = (uint8_t *) _m;
  for (int i = 0; i < 32; ++i) {
    for (int j = 0; j < 32; ++j) {
      for (int k = 1; k < 8; k++) {
        printf("%d ", (m[i * 32 + j] & (1<<k)) != 0);
      }
    }
    printf("\n");
  }
}

static inline void naive_transpose256(void *_m, const size_t offset)
{
  uint256_t w;
  uint256_t *m = (uint256_t *) _m;

  for (size_t i = 0; i < 256; ++i) {
    w[0] = m[i + offset][0];
    w[1] = m[i + offset][1];
    for (size_t j = 0; j < 256; ++j) {
      if (j < 128) {
        m[j + offset][0] |= ((w[0] >> j) & 1) << i;
      } else {
        m[j + offset][1] |= ((w[1] >> (128-j)) & 1) << i;
      }
    }
  }
}


uint8_t getbit(const void *_v, size_t pos)
{
  uint8_t *v = (uint8_t *) _v;
  return v[pos >> 3] & (1 << (pos % 8));
}

/**
 * Compute in-place transpose of a matrix (N x 256) where N = 256 * n.
 */
void transpose(void *m, const size_t n)
{
  for (size_t i = 0; i < n; i++) {
    naive_transpose256(m, i);
  }
}

/**
 * Compute fast bitwise xor between two bit-vectors a, b of size N, where N = 256*n.
 * Places the result in the first one.
 */
void xor(void *_a, const void *_b, size_t n)
{
  __m128i *a = (__m128i *) _a;
  __m128i *b = (__m128i *) _b;
  n <<= 1;
  while (n--) {
    *a = _mm_xor_si128(*a, *b);
    ++a; ++b;
  }
}

/**
 * Compute fast equality between two bit-vectors a, b of size N, where N = 256*n.
 */
bool biteq(const void *_a, const void *_b, size_t n)
{
  __m128i *a = (__m128i *) _a;
  __m128i *b = (__m128i *) _b;
  __m128i iseq;
  n <<= 1;
  while (n--) {
    iseq = _mm_cmpeq_epi8(*a, *b);
    if (_mm_movemask_epi8(iseq) != 0xffff) {
      return false;
    }
    ++a; ++b;
  }
  return true;
}
