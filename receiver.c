#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>

#include "oracle.h"
#include "otext.h"
#include "bitmath.h"
#include "libot/ot.h"



static void
receiver_check(const int sockfd,  uint8_t *choices, uint8_t (*T)[CODEN/8], const size_t m)
{
  uint8_t (*mu)[m/8] = malloc(SSEC * sizeof(*mu));
  uint8_t w[SSEC];
  uint8_t t[SSEC][CODEN/8];

  /* Read μs from the network */
  for (int i = 0; i < SSEC; ++i) {
    reading(sockfd, mu[i], KAPPA/8);
    prg_extend(mu[i], m/8);
    /* Initialize checks to base otp */
    memcpy(t[i], T[m + i], CODEN/8);
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
    writing(sockfd, t[i], CODEN/8);
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
  free(C);
  uint8_t *u = malloc(ms/8 * sizeof(*u));
  for (size_t i = 0; i < CODEN; ++i) {
    memcpy(u, CT[i], ms/8);
    bitxor(u, T0[i], ms);
    bitxor(u, T1[i], ms);
    writing(sockfd, u, ms / 8);
  }

  uint8_t (*T)[CODEN/8] = malloc(ms * sizeof(*T));
  transpose(T, T0, CODEN, ms);
  free(T0);
  free(T1);

  if (active_security) {
    receiver_check(sockfd, choices, T, m);
  }

  uint8_t pad[KAPPA/8];
  for (size_t j = 0; j < m; ++j) {
    hash(pad, T[j], j, CODEN/8, KAPPA/8);
#ifndef NDEBUG
    Bprint(pad, KAPPA/8);
    printf("\n");
#endif
  }

  free(choices);
  free(CT);
  free(u);
  free(T);
}