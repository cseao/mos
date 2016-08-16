/**
 * Enoding utilities.
 *
 * The following example codes are dynamically loaded from a file containing the
 * generator matrix (possibly in systematic form) stored as a bit vector of k*n bits.
 *
 * Note: the length MUST be a multiple of 8. We cannot possibly transmit less than
 * 8-bits at a time.
 *
 * I generated my codes using sage.
 * The following function:
 *
 * def b(x): return numpy.packbits(x.numpy('b')).tobytes()
 *
 * packs a binary matrix/vector into bytes, just as we do require.
 * So for example once you have a linear code C, you can retrieve the generator
 * matrix with:
 *
 * sage: G = codes.WalshCode(8).generator_matrix_systematic()
 *
 * and from here it's rock'n'roll:
 *
 * sage: print G.str()
 * [0 1 0 1 0 1 0 1 0 …
 * sage: print numpy.packbits(G.numpy('b'))
 * [ 85  85 85 … 255 255]
 * sage: numpy.packbits(G.numpy('b')).tobytes()
'* UUU…\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff'
 *
 *
 * ***  Repetition code
 * Trivial, use string multiplication.
 *
 * ***  Walsh-Hadamard code
 * It is avaliable in sage:
 *
 * sage: G = codes.WalshCode(8).generator_matrix_systematic()
 *
 * *** Shortened Walsh-Hadamard Code
 * Obtained from:
 * sage: V = VectorSpace(GF(2), 8)
 * sage: Gt = matrix([[1] + b.list() for b in V])
 * sage: C = LinearCode(Gt.T)
 *
 * *** Repeated Golay Codes
 * Obtained with:
 *
 * sage: C = codes.ExtendedBinaryGolayCode()
 * sage: G = C.generator_matrix_systematic()
 * sage: Ge = matrix([c.list() * 16 for c in G])
 *
 *
 * *** BCH Code Example
 * Obtained with:
 *
 * sage: C = codes.BCHCode(511, 2*85, GF(2), b=1)
 * sage: G = C.generator_matrix_systematic()
 * sage: Ge = matrix([g.list() + [0] for g in G])
 *
 */

#include <stdio.h>
#include <stdint.h>

#include "bitmath.h"
#include "codes.h"


code_t codes[] = {
  {.name = "repetition", .n = 128, .k = 1, .file = "repetition.code"},
  {.name = "wh", .n = 256, .k = 8, .file = "wh.code"},
  {.name = "shortened-wh", .n = 256, .k = 9, .file = "shwh.code"},
  {.name = "golay", .n = 384, .k = 12, .file = "extgolay.code"},
  {.name = "bch-511", .n = 512, .k = 76, .file = "bch-511.code"},

  {.name = NULL, .n = 0, .k = 0, .file = NULL},
};

// XXX. return error.
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
    free_bitmatrix(code->_G);

    fprintf(stderr, "Error reading file!\n");
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
