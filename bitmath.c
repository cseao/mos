#include <assert.h>
#include <immintrin.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

#include "bitmath.h"

void Bprint(const uint8_t *v, size_t n)
{
  for (size_t i = 0; i < n; ++i) {
    printf("%.2X", v[i]);
  }
}

void
sse_trans(uint8_t const *inp, uint8_t *out, int nrows, int ncols)
{
#   define INP(x,y) inp[(x)*ncols/8 + (y)/8]
#   define OUT(x,y) out[(y)*nrows/8 + (x)/8]
  ssize_t rr, cc, i, h;
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

/**
 * Compute in-place transpose of a matrix of (m x n) bits.
 */
void transpose(void *dst, const void *src, size_t m, size_t n)
{
  const uint8_t *A = src;
  uint8_t *B = dst;
  sse_trans(A, B, m, n);
}


void bitxor_small(void *_a, const void *_b, size_t n)
{
  uint8_t *a = (uint8_t *) _a;
  uint8_t *b = (uint8_t *) _b;
  n >>= 3;
  while (n--) {
    *a ^= *b;
    a++; b++;
  }
}
