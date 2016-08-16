#pragma once
#include <stdint.h>
#include <string.h>

#include "bitmath.h"

#define DEFAULT_CODE "wh"

typedef struct code {
  const char* name;

  const uint32_t n;
  const uint32_t k;
  const char *file;

  bitmatrix_t _G;
} code_t;


static inline
void encode(const code_t *code, void *c, const void *word)
{
  bitset_zero(c, code->n);

  for (size_t i = 0; i < code->k; ++i) {
    if (getbit(word, i)) {
      bitxor(c, row(code->_G, i), code->n);
    }
  }
}

code_t *load_codestr(const char *alias);
void unload_code(code_t * code);

/**
 *  Increment an arbitrary-length vector by recursively adding 8-bit blocks.
 *  If a block overflows, increment the next one.
 */
static inline
void next_word(uint8_t *v)
{
  if (++(*v) == 0) next_word(++v);
}
