#pragma once
#include <stdint.h>

typedef struct code {
  const char* name;

  const uint32_t n;
  const uint32_t k;

  const void *G;
} code_t;


code_t repetition;
code_t wh;

void encode(const code_t* code, void *c, void *word);
