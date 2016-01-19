#include <blake2.h>

#include "libot/ot.h"

/**
 * Extends `out` to `to` bytes in blocks of HASHBYTES.
 */
void prg_extend(uint8_t *out, size_t to)
{
#define STEPH BLAKE2B_OUTBYTES

  if (to < STEPH) {
      blake2(out, out, NULL, to, KAPPA/8, 0);
      return;
  }
  blake2(out, out, NULL, STEPH, KAPPA/8, 0);
  for (int times = to / STEPH - 1; times > 0; times--) {
    if (blake2(out+STEPH, out, NULL, STEPH, STEPH, 0) == -1) {
      perror("Cannot hash");
      exit(EXIT_FAILURE);
    }
    out += STEPH;
  }
  blake2(out+STEPH, out, NULL, to % STEPH, STEPH, 0);
}

void hash(uint8_t *out, uint8_t *in, const size_t j, const size_t inlen)
{
  static const int jsize = sizeof(size_t);
  uint8_t inj[inlen + jsize];
  memcpy(inj, &j, jsize);
  memcpy(inj + jsize, in, inlen);

  blake2(out, inj, NULL, KAPPA/8, inlen + jsize, 0);
}
