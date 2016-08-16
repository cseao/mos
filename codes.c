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

// Obtained with:
// sage: G = codes.WalshCode(8).generator_matrix_systematic()


// Shortened Walsh-Hadamard code. Obtained with:
// sage: V = VectorSpace(GF(2), 8)
// sage: Gt = matrix([[1] + b.list() for b in V])
// sage: C = LinearCode(Gt.T)
// sage: C
// Linear code of length 256, dimension 9 over Finite Field of size 2
// sage: C.minimum_distance()
// 128

// sage: C = codes.ExtendedBinaryGolayCode()
// sage: C
// Linear code of length 24, dimension 12 over Finite Field of size 2
// sage: G = C.generator_matrix_systematic()
// sage: Ge = matrix([c.list() * 16 for c in G])
// sage: Ge
// 12 x 384 dense matrix over Finite Field of size 2 (use the '.str()' method to see the entries)


code_t codes[] = {
  {.name = "repetition", .n = 128, .k = 1, .file = "repetition.code"},
  {.name = "wh", .n = 256, .k = 8, .file = "wh.code"},
  {.name = "shortened-wh", .n = 256, .k = 9, .file = "shwh.code"},
  {.name = "golay", .n = 384, .k = 12, .file = "extgolay.code"},

  {.name = NULL, .n = 0, .k = 0, .file = NULL},
};


static void load_G(code_t *code)
{
  const size_t Gsize = code->k * octs(code->n);
  size_t Gread;

  FILE *Gfile = fopen(code->file, "r");
  if (!Gfile) {
    perror("Cannot open code file.");
    exit(EXIT_FAILURE);
  }

  code->_G = new_bitmatrix(code->k, code->n);
  Gread = fread(code->_G.M, sizeof(uint8_t), Gsize, Gfile);
  if (Gread < Gsize) {
    fprintf(stderr, "Error reading file!\n");
    // XXX. free memory, return error.
    exit(EXIT_FAILURE);
  }
  fclose(Gfile);
}

code_t *load_codestr(const char *alias)
{
  for (int i = 0; codes[i].name; i++) {
    if (!strcmp(alias, codes[i].name)) {
      code_t *code = calloc(1, sizeof(code_t));
      memcpy(code, &codes[i], sizeof(code_t));
      load_G(code);
      return code;
    }
  }
  return NULL;
}


void unload_code(code_t *code)
{
  free_bitmatrix(code->_G);
  free(code);
}

void encode(const code_t *code, void *c, const void *word)
{
  bitset_zero(c, code->n);

  for (size_t i = 0; i < code->k; ++i) {
    if (getbit(word, i)) {
      bitxor(c, row(code->_G, i), code->n);
    }
  }
}
