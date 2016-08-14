#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "bitmath.h"
#include "oracle.h"
#include "oracle.h"
#include "otext.h"

int main()
{

  const size_t bits = 1024;
  void *v = bitalloc(bits);
  while (1) {
    randombits(v, bits);
    prgbits(v, bits);
    write(1, v, octs(bits));
  }
  free(v);
}
