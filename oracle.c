#include <blake2.h>

#include "libot/ot.h"

/**
 * Extends `out` to `to` bytes in blocks of HASHBYTES.
 */
void prg_extend(uint8_t *out, size_t to)
{
  blake2(out, out, NULL, HASHBYTES, KAPPA/8, 0);
  for (int times = to / HASHBYTES - 1; times > 0; times--) {
    if (blake2(out+HASHBYTES, out, NULL, HASHBYTES, HASHBYTES, 0) == -1) {
      perror("Cannot hash");
      exit(EXIT_FAILURE);
    }
    out += HASHBYTES;
  }
  blake2(out+HASHBYTES, out, NULL, to % HASHBYTES, HASHBYTES, 0);
}

void hash(uint8_t *out, uint8_t *in, const size_t j, const size_t inlen)
{
  uint8_t inj[inlen + sizeof(size_t)];
  memcpy(inj, &j, sizeof(size_t));
  memcpy(inj + sizeof(size_t), in, inlen);

  blake2(out, inj, NULL, KAPPA/8, inlen + sizeof(int), 0);
}
