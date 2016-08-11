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
  uint8_t m[8];

  for (size_t i=0; i != 8; ++i) {
    setbit(m, i*8 + i, 1);
  }

  assert(getbit(m, 0) == 1);
  assert(getbit(m, 1) == 0);
  transpose(m, m, 8, 8);
  assert(getbit(m, 0) == 1);
  assert(getbit(m, 1) == 0);
}

void test_transpose()
{
  uint256_t m[256];
  memset(m, 0, sizeof(m));
  setbit(m, 0, 1);
  setbit(m, 1, 1);
  setbit(m, 2, 1);
  setbit(m, 3, 1);
  transpose(m, m, 256, 256);
  assert(getbit(m, 0) == 1);
  assert(getbit(m, 1) == 0);
  assert(getbit(m, 256 * 1) == 1);
  assert(getbit(m, 256 * 2) == 1);

  uint8_t A[8];
  uint8_t B[8];
  int r = open("/dev/urandom", O_RDONLY);
  read(r, A, 8);
  transpose(B, A, 8, 8);
  for (size_t i = 0; i < 8; i++) {
    for (size_t j = 0; j < 8; j ++) {
      assert(getbit(A, i * 8 + j) == getbit(B, j * 8 + i));
    }
  }

  size_t Crows = 1 << 10;
  size_t Ccols = 256;
  uint8_t C[Crows][Ccols/8];
  uint8_t CT[Ccols][Crows/8];
  for (size_t i = 0; i < Crows; ++i) {
    read(r, C[i], Ccols/8);
  }
  transpose(CT, C, Crows, Ccols);
  for (size_t i = 0; i < Crows; i++) {
    for (size_t j = 0; j < Ccols; j++) {
      assert(getbit(C, i * Ccols + j) == getbit(CT, j * Crows + i));
    }
  }
  /* for (size_t i = 0; i < 8; i++) { */
  /*   printf("%.2X\t%.2X\n", A[i], B[i]); */
  /* } */
}

void test_transpose_endianness()
{
  uint8_t m[8];
  m[0] = 0x0f;
  assert(getbit(m, 0) == 1);
  transpose(m, m, 8, 8);
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


int main()
{
  test_bitxor();
  test_bitand();
  test_transpose();
  test_transpose_identity();
  test_transpose_endianness();
  test_biteq();
  test_bitmatrix();
  return 0;
}
