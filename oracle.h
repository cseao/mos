#pragma once
#include <stdint.h>
#include <stddef.h>

#include "bitmath.h"

void prg_extend(uint8_t *out, size_t to);

void hash(uint8_t *out, uint8_t *in, const size_t j, const size_t inlen, const size_t outlen);
void base_hash(void *in, const size_t inlen);


#define randombits(v, bits)  randombytes((uint8_t *) v, octs(bits))
#define prgbits(v, bits)  prg_extend(v, octs(bits))
