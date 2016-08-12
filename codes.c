#include <stdio.h>
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
  .file = "repetition.code",
};

code_t wh = {
  .name = "WH",
  .n = 256,
  .k = 8,
  .file = "wh.code",
};

void load_code(code_t *code)
{
  const size_t Gsize = octs(code->n) * code->k;
  size_t Gread;

  FILE *Gfile = fopen(code->file, "r");
  if (!Gfile) {
    perror("Cannot open code file.");
    exit(EXIT_FAILURE);
  }

  code->_G = malloc(Gsize);
  Gread = fread(code->_G, sizeof(uint8_t), Gsize, Gfile);
  if (Gread < Gsize) {
    fprintf(stderr, "Error reading file!\n");
    // XXX. free memory, return error.
    exit(EXIT_FAILURE);
  }
  fclose(Gfile);
}


void unload_code(code_t *code)
{
  free(code->_G);
}

void encode(const code_t *code, void *c, const void *word)
{
#define bitcell(M, size, nmemb)    ((const uint8_t *) M + size * octs(nmemb))
  const uint32_t n = code->n;

  bitset_zero(c, n);
  for (size_t i = 0; i < code->k; ++i) {
    if (getbit(word, i)) {
      bitxor(c, bitcell(code->_G, i, n), n);
    }
  }
}
