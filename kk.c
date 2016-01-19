#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>

#include "oracle.h"
#include "bitmath.h"
#include "libot/ot.h"

#define CODEN KAPPA*2
#define CODEK KAPPA
#define SSEC  40

static const size_t codewordsm = 1;
static const size_t codewordsn = 2;
static uint8_t codewords[2][CODEN/8] = {
  {[0 ... 31] = '\x00'},
  {[0 ... 31] = '\xff'}
};


void kk_sender(int sockfd, size_t m)
{
  int p[2];
  if (pipe(p) == -1) {
    perror("Cannot create pipe");
    exit(EXIT_FAILURE);
  }
  /* let m' = m + s */
  size_t ms = (m + SSEC);
  ms -= (ms % 128);

  /* Perform the base OTs, extends them and place those in a matrix Q. */
  uint8_t delta[CODEN/8];
  uint8_t unpacked_delta[CODEN];
  randombytes(delta, CODEN/8);
  for (size_t i = 0; i < CODEN; ++i) {
    unpacked_delta[i] = getbit(delta, i);
  }

  baseot_receiver(sockfd, CODEN, unpacked_delta, p[1]);
  uint8_t (*Q)[ms/8] = malloc(CODEN * sizeof(*Q));
  for (size_t i = 0; i < CODEN; ++i) {
    reading(p[0], Q[i], KAPPA/8);
    prg_extend(Q[i], ms/8);
  }

  uint8_t *u = malloc(ms/8 * sizeof(*u));
  for (size_t i = 0; i < CODEN; ++i) {
    reading(sockfd, u, ms/8);
    if (unpacked_delta[i]) {
      bitxor(Q[i], u, ms);
    }
  }

  uint8_t (*QT)[CODEN/8] = malloc(ms * sizeof(*QT));
  transpose(QT, Q, CODEN, ms);
  uint8_t q[CODEN/8];
  for (size_t j = 0; j < ms; ++j) {
    for (size_t i = 0; i < codewordsn; i++) {
      memcpy(q, delta, CODEN/8);
      bitand(q, codewords[i], CODEN);
      bitxor(q, QT[j], CODEN);
      hash(q, q, j, CODEN/8, KAPPA/8);
      // Bprint(q, KAPPA/8);
      // printf("\t");
    }
    // printf("\n");
  }
  free(Q);
  free(QT);
  free(u);
}

void kk_receiver(int sockfd, size_t m) {
  int p[2];
  if (pipe(p) == -1) {
    perror("Cannot create pipe");
    exit(EXIT_FAILURE);
  }

  /* let m' = m + s. */
  size_t ms = m + SSEC;
  ms -= (ms % 128);

  /* note: rows here are columns in the paper. */
  baseot_sender(sockfd, CODEN, p[1]);
  uint8_t (*T0)[ms/8] = malloc(CODEN * sizeof(*T0));
  uint8_t (*T1)[ms/8] = malloc(CODEN * sizeof(*T0));
  for (size_t i = 0; i < CODEN; i++) {
    reading(p[0], T0[i], KAPPA/8);
    prg_extend(T0[i], ms/8);
    reading(p[0], T1[i], KAPPA/8);
    prg_extend(T1[i], ms/8);
  }

  uint8_t (*C)[CODEN/8] = malloc(ms * sizeof(*C));
  uint8_t *choices = malloc(ms * sizeof(*choices));
  randombytes(choices, ms);
  for (size_t i = 0; i < ms; ++i) {
    choices[i] &= codewordsm;
    memcpy(C[i], codewords + choices[i], sizeof(*codewords));
  }

  uint8_t (*CT)[ms/8] = malloc(CODEN * sizeof(*CT));
  transpose(CT, C, ms, CODEN);
  uint8_t *u = malloc(ms/8 * sizeof(*u));
  for (size_t i = 0; i < CODEN; ++i) {
    memcpy(u, CT[i], ms/8);
    bitxor(u, T0[i], ms);
    bitxor(u, T1[i], ms);
    writing(sockfd, u, ms / 8);
  }

  uint8_t (*T)[CODEN/8] = malloc(ms * sizeof(*T));
  uint8_t pad[KAPPA/8];
  transpose(T, T0, CODEN, ms);
  for (size_t j = 0; j < ms; j++) {
    hash(pad, T[j], j, CODEN/8, KAPPA/8);
    // Bprint(pad, KAPPA/8);
    // printf("\n");
  }

  free(T0);
  free(T1);
  free(C);
  free(choices);
  free(CT);
  free(u);
  free(T);
}
