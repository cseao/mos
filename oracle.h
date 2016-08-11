#pragma once
#include <stdint.h>
#include <stddef.h>

void prg_extend(uint8_t *out, size_t to);

void hash(uint8_t *out, uint8_t *in, const size_t j, const size_t inlen, const size_t outlen);
void base_hash(void *in, const size_t inlen);


#define randombits(v, bits)  randombytes(v, bits >> 3)
#define prgbits(v, howmany)  prg_extend(v, howmany >> 3)
