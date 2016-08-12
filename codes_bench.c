#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "bitmath.h"
#include "codes.h"

int main()
{

  uint8_t w;
  uint8_t *c = bitalloc(wh.n);
  load_code(&wh);
  while (1) {
    w = getc(stdin);
    encode(&wh, c, &w);
    write(1, c, octs(wh.n));
  }
  free(c);
  unload_code(&wh);
}
