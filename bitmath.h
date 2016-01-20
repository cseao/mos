#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef __uint128_t uint128_t;
typedef uint128_t uint256_t[2];


void Bprint(const uint8_t *v, size_t n);
void transpose(void *dst, const void *src, size_t m, size_t n);
void bitxor(void *_a, const void *_b, size_t n);
void bitand(void *_a, const void *_b, size_t n);
bool biteq(const void *_a, const void *_b, size_t n);
uint8_t getbit(const void *_v, size_t pos);
