#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>

#include "bitmath.h"

void setbit(void *_v, size_t pos, bool bit)
{
  uint8_t *v = (uint8_t *) _v;
  v[pos >> 3] |= (bit << (pos % 8));
}

void test_transpose_identity()
{
  bitmatrix_t m = new_bitmatrix(8, 8);

  for (size_t i=0; i != 8; ++i) {
    setbit(m.M, i*8 + i, 1);
  }

  assert(getbit(m.M, 0) == 1);
  assert(getbit(m.M, 1) == 0);
  transpose(&m, &m, 8, 8);
  assert(getbit(m.M, 0) == 1);
  assert(getbit(m.M, 1) == 1);
}

void test_transpose()
{
  bitmatrix_t m = new_bitmatrix(256, 256);
  bitset_zero(m.M, 256*256);
  setbit(m.M, 0, 1);
  setbit(m.M, 1, 1);
  setbit(m.M, 2, 1);
  setbit(m.M, 3, 1);
  transpose(&m, &m, 256, 256);
  assert(getbit(m.M, 0) == 1);
  assert(getbit(m.M, 1) == 0);
  assert(getbit(m.M, 256 * 1) == 1);
  assert(getbit(m.M, 256 * 2) == 1);
  free_bitmatrix(m);

  bitmatrix_t A = new_bitmatrix(8, 8);
  bitmatrix_t B = new_bitmatrix(8, 8);
  int r = open("/dev/urandom", O_RDONLY);
  read(r, A.M, 8);
  transpose(&B, &A, 8, 8);
  for (size_t i = 0; i < 8; i++) {
    for (size_t j = 0; j < 8; j ++) {
      assert(getbit(A.M, i * 8 + j) == getbit(B.M, j * 8 + i));
    }
  }
  free_bitmatrix(A);
  free_bitmatrix(B);

  size_t Crows = 1 << 10;
  size_t Ccols = 256;
  bitmatrix_t C = new_bitmatrix(Crows, Ccols);
  bitmatrix_t CT = new_bitmatrix(Ccols, Crows);
  for (size_t i = 0; i < Crows; ++i) {
    read(r, row(C, i), Ccols/8);
  }
  transpose(&CT, &C, Crows, Ccols);
  for (size_t i = 0; i < Crows; i++) {
    for (size_t j = 0; j < Ccols; j++) {
      assert(getbit(C.M, i * Ccols + j) == getbit(CT.M, j * Crows + i));
    }
  }
  free_bitmatrix(C);
  free_bitmatrix(CT);
  /* for (size_t i = 0; i < 8; i++) { */
  /*   printf("%.2X\t%.2X\n", A[i], B[i]); */
  /* } */
}

void test_transpose_endianness()
{
  bitmatrix_t m = new_bitmatrix(8, 8);
  *row(m, 0) = 0x0f;
  assert(getbit(m.M, 0) == 1);
  transpose(&m, &m, 8, 8);
  free_bitmatrix(m);
  // ??
}

void test_bitxor()
{
  uint256_t a = {0x0f, 0xff};
  uint256_t b = {0xf0, 0xff};
  uint256_t c = {0xff, 0x00};
  bitxor(a, b, 256);
  assert(biteq(a, c, 256));
}


void test_bitand()
{
  uint8_t a[256/8];
  memset(a, 0xff, 256/8);
  uint8_t b[256/8] = {0};
  bitand(a, b, 256);
  assert(getbit(a, 0) == 0);

  memset(a, 0xff, 256/8);
  memset(b, 0xff, 256/8);
  bitand(a, b, 256);
  assert(getbit(a, 128) != 0);
  assert(getbit(a, 1) != 0);
}

void test_biteq()
{
  uint8_t a[256] = {0};
  uint8_t *b[256] = {0};
  memcpy(a, "\xDE\xAD\xBE\xEF", 4);
  memcpy(b, "\xDE\xAD\x00\x00", 4);
  assert(!biteq(a, b, 256));
  memcpy(b, "\xDE\xAD\xBE\xEF", 4);
  assert(biteq(a, b, 256));
}

void test_bitmatrix()
{
  bitmatrix_t m = new_bitmatrix(8, 8);
  uint8_t v[] = "ABCDEFGH";
  bitcpy(m.M, v, 8*8);
  assert(!memcmp(row(m, 0), v, 1));
  assert(!memcmp(row(m, 1), v+1, 1));
  assert(!memcmp(row(m, 7), v+7, 1));
}


void test_bitmask()
{
  uint8_t v[] = "\xff\xff\xff";
  bitmask(v, 20);
  assert(getbit(v, 10) == 1);
  assert(getbit(v, 12) == 1);
  assert(getbit(v, 15) == 1);
  assert(getbit(v, 20) == 0);
  assert(getbit(v, 21) == 0);
  assert(getbit(v, 22) == 0);
  assert(getbit(v, 23) == 0);

  bitmask(v, 2);
  assert(getbit(v, 0) == 1);
  assert(getbit(v, 1) == 1);
  assert(getbit(v, 2) == 0);
}

int main()
{
  test_bitxor();
  test_bitand();
  test_transpose();
  test_transpose_identity();
  test_transpose_endianness();
  test_biteq();
  test_bitmatrix();
  test_bitmask();
  return 0;
}
