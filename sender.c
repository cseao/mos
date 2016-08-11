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
sender_check(const int sockfd, uint8_t delta[CODEN/8], uint8_t (*QT)[CODEN/8], const size_t m)
{
  uint8_t (*mu)[m/8] = malloc(SSEC * sizeof(*mu));
  uint8_t q[SSEC][CODEN/8];
  uint8_t t[SSEC][CODEN/8];
  uint8_t c[SSEC][CODEN/8];
  uint8_t w[SSEC];
  for (int i = 0; i < SSEC; ++i) {
    randombytes(mu[i], KAPPA/8);
    writing(sockfd, mu[i], KAPPA/8);
    prg_extend(mu[i], m/8);

    memcpy(q[i], QT[m + i], CODEN/8);
  }

  for (size_t j = 0; j < m; ++j) {
    for (int i = 0; i < SSEC; ++i) {
      if (getbit(mu[i], j)) {
        bitxor(q[i], QT[j], CODEN);
      }
    }
  }

  for (int i = 0; i < SSEC; ++i) {
    reading(sockfd, t[i], CODEN/8);
    reading(sockfd, &w[i], 1);
    if (w[i] & ~codewordsm) {
      fprintf(stderr, "Check Failed!\n");
      exit(EXIT_FAILURE);
    }
    memcpy(c[i], codewords[w[i]], CODEN/8);
    bitxor(q[i], t[i], CODEN);
    bitand(c[i], delta, CODEN);

    if (!biteq(c[i], q[i], CODEN)) {
      fprintf(stderr, "Check failed!\n");
      exit(EXIT_FAILURE);
    }
  }
  free(mu);
}

void kk_sender(int sockfd, size_t m)
{
  int p[2];
  if (pipe(p) == -1) {
    perror("Cannot create pipe");
    exit(EXIT_FAILURE);
  }
  /* let m' = m + s */
  size_t ms = active_security? m + SSEC : m;

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

  if (active_security) {
    sender_check(sockfd, delta, QT, m);
  }

  uint8_t c[octs(CODEN)];
  struct {uint8_t q[CODEN/8]; size_t j; } q_j;
  for (q_j.j = 0; q_j.j < m; ++q_j.j) {
    for (size_t i = 0; i < codewordsn; i++) {
      memcpy(q_j.q, delta, CODEN/8);
      // XXX. also here intrinsics fucks up
      bitcpy(c, codewords[i], CODEN);
      bitand(q_j.q, c, CODEN);
      bitxor(q_j.q, QT[q_j.j], CODEN);
      base_hash((void *) &q_j, sizeof(q_j));
#ifndef NDEBUG
      Bprint(q_j.q, KAPPA/8);
      printf("\t");
#endif
    }
#ifndef NDEBUG
    printf("\n");
#endif
  }
  free(Q);
  free(QT);
  free(u);
}
