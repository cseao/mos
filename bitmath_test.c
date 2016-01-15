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
  transpose(m, 8, 8);
  assert(getbit(m, 0) == 1);
  assert(getbit(m, 1) == 0);
}

void test_transpose()
{
  uint256_t m[256];
  memset(m, 0, sizeof(m));
  setbit(m, 1, 1);
  setbit(m, 2, 1);
  setbit(m, 3, 1);
  transpose(m, 256, 256);
  assert(getbit(m, 0) == 0);
  assert(getbit(m, 1) == 0);
  assert(getbit(m, 256*1) == 1);
  assert(getbit(m, 256*2) == 1);
}

void test_xor()
{
  uint256_t a = {0x0f, 0xff};
  uint256_t b = {0xf0, 0xff};
  uint256_t c = {0xff, 0x00};
  xor(a, b, 1);
  assert(biteq(a, c, 1));
}



int main()
{
  test_transpose_identity();
  test_xor();
  test_transpose();
  return 0;
}
