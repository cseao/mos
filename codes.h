#pragma once
#include <stdint.h>

#include "bitmath.h"

typedef struct code {
  const char* name;

  const uint32_t n;
  const uint32_t k;
  const char *file;

  bitmatrix_t _G;
} code_t;


code_t repetition;
code_t wh;

void encode(const code_t* code, void *c, const void *word);
void load_code(code_t * code);
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
