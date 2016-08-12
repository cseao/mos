#include <stdint.h>
#include <string.h>

#include "bitmath.h"
#include "codes.h"

// I produce the systematic generator matrix G using sage.
// Then, I do:
// def b(x): return numpy.packbits(x.numpy('b')).tobytes()
// to get the blobs below.
// For example, with walsh-hadamard codes I did:
// sage: G = codes.WalshCode(8).generator_matrix_systematic()
// sage: print G.str()
// [0 1 0 1 0 1 0 1 0 …
// sage: print numpy.packbits(G.numpy('b'))
// [ 85  85 85 … 255 255]
// sage: numpy.packbits(G.numpy('b')).tobytes()
// 'UUU…\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff'


code_t repetition = {
  .name = "REP",
  .n = 128,
  .k = 1,
  .G = "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff",
};

code_t wh = {
  .name = "WH",
  .n = 256,
  .k = 8,
  .G = "UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU33333333333333333333333333333333\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x00\xff\x00\xff\x00\xff\x00\xff\x00\xff\x00\xff\x00\xff\x00\xff\x00\xff\x00\xff\x00\xff\x00\xff\x00\xff\x00\xff\x00\xff\x00\xff\x00\x00\xff\xff\x00\x00\xff\xff\x00\x00\xff\xff\x00\x00\xff\xff\x00\x00\xff\xff\x00\x00\xff\xff\x00\x00\xff\xff\x00\x00\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff",
};


void encode(const code_t *code, void *c, void *word)
{
#define bitcell(M, size, nmemb)    ((const uint8_t *) M + size * octs(nmemb))
  const uint32_t n = code->n;
  uint8_t g[octs(n)];

  bitset_zero(c, n);
  for (size_t i = 0; i < code->k; ++i) {
    if (getbit(word, i)) {
      // it looks like instrinsics fucks up if we give a read-only vector like
      // the one initialized in the structure.
      // XXX
      // Probabily best to have a constructor to avoid a vector copy every time…
      bitcpy(g, bitcell(code->G, i, n), n);
      bitxor(c, g, n);
    }
  }
}
