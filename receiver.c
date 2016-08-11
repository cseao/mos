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

static void
receiver_check(const int sockfd,  uint8_t (*T)[octs(CODEN)], const size_t m)
{
  uint8_t (*mu)[octs(m)] = malloc(SSEC * sizeof(*mu));
  uint8_t w[SSEC];
  uint8_t t[SSEC][octs(CODEN)];

  /* Read μs from the network */
  for (int i = 0; i < SSEC; ++i) {
    reading(sockfd, mu[i], octs(KAPPA));
    prg_extend(mu[i], octs(m));
    /* Initialize checks to base otp */
    memcpy(t[i], T[m + i], octs(CODEN));
    w[i] = choices[m + i];
  }

  /* Compute check values */
  for (size_t j = 0; j < m; ++j) {
    for (int i = 0; i < SSEC; ++i) {
      if (getbit(mu[i], j)) {
        w[i] ^= choices[j];
        bitxor(t[i], T[j], CODEN);
      }
    }
  }

  /* Send check values */
  for (int i = 0; i < SSEC; ++i) {
    writing(sockfd, t[i], octs(CODEN));
    writing(sockfd, &w[i], 1);
  }
  free(mu);
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
    prg_extend(T0[i], octs(ms));
    reading(p[0], T1[i], octs(KAPPA));
    prg_extend(T1[i], octs(ms));
  }

  uint8_t (*C)[octs(CODEN)] = malloc(ms * sizeof(*C));
  choices = malloc(ms * sizeof(*choices));
  randombytes(choices, ms);
  for (size_t i = 0; i < ms; ++i) {
    choices[i] &= codewordsm;
    memcpy(C[i], codewords + choices[i], sizeof(*codewords));
  }

  uint8_t (*CT)[octs(ms)] = malloc(CODEN * sizeof(*CT));
  transpose(CT, C, ms, CODEN);
  free(C);
  uint8_t *u = malloc(octs(ms) * sizeof(*u));
  for (size_t i = 0; i < CODEN; ++i) {
    memcpy(u, CT[i], octs(ms));
    bitxor(u, T0[i], ms);
    bitxor(u, T1[i], ms);
    writing(sockfd, u, octs(ms));
  }

  uint8_t (*T)[octs(CODEN)] = malloc(ms * sizeof(*T));
  transpose(T, T0, CODEN, ms);
  free(T0);
  free(T1);

  if (active_security) {
    receiver_check(sockfd, T, m);
  }

  uint8_t pad[octs(KAPPA)];
  for (size_t j = 0; j < m; ++j) {
    hash(pad, T[j], j, octs(CODEN), octs(KAPPA));
#ifndef NDEBUG
    Bprint(pad, octs(KAPPA));
    printf("\n");
#endif
  }

  free(choices);
  free(CT);
  free(u);
  free(T);
}
