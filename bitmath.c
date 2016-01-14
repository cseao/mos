#include <emmintrin.h>
#include <immintrin.h>
#include <avxintrin.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

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

static void naive_transpose256(void *_m, const size_t offset)
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
void xor(void *_a, void *_b, size_t n)
{
  __m256d *a = (__m256d *) _a;
  __m256d *b = (__m256d *) _b;
  for (size_t i = 0; i < n; i++) {
    *a = _mm256_xor_pd(*a, *b);
    ++a; ++b;
  }
}
