#include <assert.h>
#include <immintrin.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

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

void
sse_trans(uint8_t const *inp, uint8_t *out, int nrows, int ncols)
{
#   define INP(x,y) inp[(x)*ncols/8 + (y)/8]
#   define OUT(x,y) out[(y)*nrows/8 + (x)/8]
  int rr, cc, i, h;
  union { __m128i x; uint8_t b[16]; } tmp;
  assert(nrows % 8 == 0 && ncols % 8 == 0);

  // Do the main body in 16x8 blocks:
  for (rr = 0; rr <= nrows - 16; rr += 16) {
    for (cc = 0; cc < ncols; cc += 8) {
      for (i = 0; i < 16; ++i)
        tmp.b[i] = INP(rr + i, cc);
      for (i = 8; --i >= 0; tmp.x = _mm_slli_epi64(tmp.x, 1))
        *(uint16_t*)&OUT(rr,cc+i)= _mm_movemask_epi8(tmp.x);
    }
  }
  if (rr == nrows) return;

  // The remainder is a block of 8x(16n+8) bits (n may be 0).
  //  Do a PAIR of 8x8 blocks in each step:
  for (cc = 0; cc <= ncols - 16; cc += 16) {
    for (i = 0; i < 8; ++i) {
      tmp.b[i] = h = *(uint16_t const*)&INP(rr + i, cc);
      tmp.b[i + 8] = h >> 8;
    }
    for (i = 8; --i >= 0; tmp.x = _mm_slli_epi64(tmp.x, 1)) {
      OUT(rr, cc + i) = h = _mm_movemask_epi8(tmp.x);
      OUT(rr, cc + i + 8) = h >> 8;
    }
  }
  if (cc == ncols) return;

  //  Do the remaining 8x8 block:
  for (i = 0; i < 8; ++i)
    tmp.b[i] = INP(rr + i, cc);
  for (i = 8; --i >= 0; tmp.x = _mm_slli_epi64(tmp.x, 1))
    OUT(rr, cc + i) = _mm_movemask_epi8(tmp.x);
}

uint8_t getbit(const void *_v, size_t pos)
{
  uint8_t *v = (uint8_t *) _v;
  return v[pos >> 3] & (1 << (pos % 8));
}

/**
 * Compute in-place transpose of a matrix of (m x n) bits.
 */
void transpose(void *_A, size_t m, size_t n)
{
  uint8_t *A = (uint8_t *) _A;
  uint8_t B[m * n / 8];
  memcpy(B, A, m * n / 8);
  sse_trans(B, A, m, n);
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
