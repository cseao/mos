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
  bitmatrix_t t = new_bitmatrix(SSEC, CODEN);
  uint8_t w[SSEC];

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
  bitmatrix_t T0 = new_bitmatrix(CODEN, ms);
  bitmatrix_t T1 = new_bitmatrix(CODEN, ms);
  for (size_t i = 0; i < CODEN; i++) {
    reading(p[0], row(T0, i), octs(KAPPA));
    prgbits(row(T0, i), ms);
    reading(p[0], row(T1, i), octs(KAPPA));
    prgbits(row(T1, i), ms);
  }

  bitmatrix_t C = new_bitmatrix(ms, CODEN);
  choices = malloc(ms * sizeof(*choices));
  randombits(choices, ms * 8);
  for (size_t i = 0; i < ms; ++i) {
    choices[i] &= codewordsm;
    bitcpy(row(C, i), codewords + choices[i], CODEN);
  }

  bitmatrix_t CT = new_bitmatrix(CODEN, ms);
  transpose(CT.M, C.M, ms, CODEN);
  free_bitmatrix(C);
  uint8_t *u = bitalloc(ms);
  for (size_t i = 0; i < CODEN; ++i) {
    bitcpy(u, row(CT, i), ms);
    bitxor(u, row(T0, i), ms);
    bitxor(u, row(T1, i), ms);
    writebits(sockfd, u, ms);
  }

  T = new_bitmatrix(ms, CODEN);
  transpose(T.M, T0.M, CODEN, ms);
  free_bitmatrix(T0);
  free_bitmatrix(T1);

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
  free(u);
  free_bitmatrix(CT);
  free_bitmatrix(T);
}
