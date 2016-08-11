#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>

#include "oracle.h"
#include "otext.h"
#include "bitmath.h"
#include "libot/ot.h"

static uint8_t *choices;
static bitmatrix_t T;

static void
receiver_check(const int sockfd, const size_t m)
{
  bitmatrix_t mu = new_bitmatrix(SSEC, m);
  uint8_t w[SSEC];
  bitmatrix_t t = new_bitmatrix(SSEC, CODEN);

  /* Read μs from the network */
  for (int i = 0; i < SSEC; ++i) {
    readbits(sockfd, row(mu, i), KAPPA);
    prgbits(row(mu, i), m);
    /* Initialize checks to base otp */
    bitcpy(row(t, i), row(T, m+i), CODEN);
    w[i] = choices[m + i];
  }

  /* Compute check values */
  for (size_t j = 0; j < m; ++j) {
    for (int i = 0; i < SSEC; ++i) {
      if (getbit(row(mu, i), j)) {
        w[i] ^= choices[j];
        bitxor(row(t, i), row(T, j), CODEN);
      }
    }
  }

  /* Send check values */
  for (int i = 0; i < SSEC; ++i) {
    writebits(sockfd, row(t, i), CODEN);
    writing(sockfd, &w[i], 1);
  }
  free_bitmatrix(mu);
  free_bitmatrix(t);
}

void kk_receiver(int sockfd, size_t m) {
  int p[2];
  if (pipe(p) == -1) {
    perror("Cannot create pipe");
    exit(EXIT_FAILURE);
  }

  /* let m' = m + s. */
  size_t ms = active_security? m + SSEC : m;

  /* note: rows here are columns in the paper. */
  baseot_sender(sockfd, CODEN, p[1]);
  uint8_t (*T0)[octs(ms)] = malloc(CODEN * sizeof(*T0));
  uint8_t (*T1)[octs(ms)] = malloc(CODEN * sizeof(*T0));
  for (size_t i = 0; i < CODEN; i++) {
    reading(p[0], T0[i], octs(KAPPA));
    prgbits(T0[i], ms);
    reading(p[0], T1[i], octs(KAPPA));
    prgbits(T1[i], ms);
  }

  uint8_t (*C)[octs(CODEN)] = malloc(ms * sizeof(*C));
  choices = malloc(ms * sizeof(*choices));
  randombits(choices, ms * 8);
  for (size_t i = 0; i < ms; ++i) {
    choices[i] &= codewordsm;
    memcpy(C[i], codewords + choices[i], sizeof(*codewords));
  }

  uint8_t (*CT)[octs(ms)] = malloc(CODEN * sizeof(*CT));
  transpose(CT, C, ms, CODEN);
  free(C);
  uint8_t *u = malloc(octs(ms) * sizeof(*u));
  for (size_t i = 0; i < CODEN; ++i) {
    bitcpy(u, CT[i], ms);
    bitxor(u, T0[i], ms);
    bitxor(u, T1[i], ms);
    writebits(sockfd, u, ms);
  }

  T = new_bitmatrix(ms, CODEN);
  transpose(T.M, T0, CODEN, ms);
  free(T0);
  free(T1);

  if (active_security) {
    receiver_check(sockfd, m);
  }

  uint8_t pad[octs(KAPPA)];
  for (size_t j = 0; j < m; ++j) {
    hash(pad, row(T, j), j, octs(CODEN), octs(KAPPA));
#ifndef NDEBUG
    Bprint(pad, octs(KAPPA));
    printf("\n");
#endif
  }

  free(choices);
  free(CT);
  free(u);
  free_bitmatrix(T);
}
