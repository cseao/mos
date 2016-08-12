#include <assert.h>
#include <emmintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitmath.h"
#include "codes.h"


void test_encode_repetition()
{
  load_code(&repetition);

  uint8_t *zero = (uint8_t *) "\x00";
  uint8_t zero_codeword[octs(repetition.n)];
  uint8_t got[octs(repetition.n)];

  bitset_one(got, repetition.n);
  bitset_zero(zero_codeword, repetition.n);
  assert(!biteq(got, zero_codeword, repetition.n));
  encode(&repetition, got, zero);
  assert(biteq(got, zero_codeword, repetition.n));

  uint8_t *one = (uint8_t *) "\x01";
  uint8_t one_codeword[octs(repetition.n)];
  bitset_one(one_codeword, repetition.n);
  encode(&repetition, got, one);
  assert(biteq(got, one_codeword, repetition.n));

  unload_code(&repetition);
}

void test_encode_wh()
{
  load_code(&wh);

  uint8_t *zero = (uint8_t *) "\x00";
  uint8_t zero_codeword[octs(wh.n)];
  uint8_t got[octs(wh.n)];
  bitset_zero(zero_codeword, wh.n);
  bitset_one(got, wh.n);
  assert(!biteq(got, zero_codeword, wh.n));
  encode(&wh, got, zero);
  assert(biteq(got, zero_codeword, wh.n));

  uint8_t one[] = "\x01";
  uint8_t one_codeword[] = "UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU";
  encode(&wh, got, one);
  assert(biteq(got, one_codeword, wh.n));

  uint8_t w[] = "\x07";
  uint8_t expected[] = "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii";
  bitset_zero(got, wh.n);
  encode(&wh, got, w);
  assert(biteq(got, expected, wh.n));

  unload_code(&wh);
}


void test_next_word()
{
  uint8_t a[4] = {0};
  next_word(a);
  assert(a[0] = 0x01);
  next_word(a);
  assert(a[0] == 0x02);

  a[0] = 0xff;
  next_word(a);
  assert(a[0] == 0x00);
  assert(a[1] == 0x01);
  next_word(a);
  assert(a[0] == 0x01 && a[1] == 0x01);

  a[0] = 0xff;
  a[1] = 0x7f;
  next_word(a);
  assert(a[0] == 0x00 && a[1] == 0x80);
}


int main()
{
  test_encode_repetition();
  test_encode_wh();
  test_next_word();

  return 0;
}
