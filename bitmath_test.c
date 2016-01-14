#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>

#include "bitmath.h"

void mzero(void *m)
{
  memset(m, 0, 256 * 256);
}

uint8_t getbit(void *_m, size_t r, size_t i, size_t j)
{

  uint8_t *m = (uint8_t *) _m;
  return m[i * r + (j/8)] & (1 << j % 8);
}


int main()
{
  uint256_t m[256];

  for (size_t i=0; i != 256; ++i) {
    if (i < 128) {
      m[i][0] |= 1 << i;
    } else {
      m[i][1] |= 1 << (128-i);
    }
  }

  assert(getbit(m, 256, 0, 0) == 1);
  assert(getbit(m, 256, 0, 1) == 0);
  transpose(m, 1);
  assert(getbit(m, 256, 0, 0) == 1);
  assert(getbit(m, 256, 0, 1) == 0);
}
