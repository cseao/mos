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
sender_check(const int sockfd, uint8_t delta[octs(CODEN)], uint8_t (*QT)[octs(CODEN)], const size_t m)
{
  uint8_t (*mu)[octs(m)] = malloc(SSEC * sizeof(*mu));
  uint8_t q[SSEC][octs(CODEN)];
  uint8_t t[SSEC][octs(CODEN)];
  uint8_t c[SSEC][octs(CODEN)];
  uint8_t w[SSEC];
  for (int i = 0; i < SSEC; ++i) {
    randombytes(mu[i], octs(KAPPA));
    writing(sockfd, mu[i], octs(KAPPA));
    prg_extend(mu[i], octs(m));

    memcpy(q[i], QT[m + i], octs(CODEN));
  }

  for (size_t j = 0; j < m; ++j) {
    for (int i = 0; i < SSEC; ++i) {
      if (getbit(mu[i], j)) {
        bitxor(q[i], QT[j], CODEN);
      }
    }
  }

  for (int i = 0; i < SSEC; ++i) {
    reading(sockfd, t[i], octs(CODEN));
    reading(sockfd, &w[i], 1);
    if (w[i] & ~codewordsm) {
      fprintf(stderr, "Check Failed!\n");
      exit(EXIT_FAILURE);
    }
    memcpy(c[i], codewords[w[i]], octs(CODEN));
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
  uint8_t delta[octs(CODEN)];
  uint8_t unpacked_delta[CODEN];
  randombytes(delta, octs(CODEN));
  for (size_t i = 0; i < CODEN; ++i) {
    unpacked_delta[i] = getbit(delta, i);
  }

  baseot_receiver(sockfd, CODEN, unpacked_delta, p[1]);
  uint8_t (*Q)[octs(ms)] = malloc(CODEN * sizeof(*Q));
  for (size_t i = 0; i < CODEN; ++i) {
    reading(p[0], Q[i], octs(KAPPA));
    prg_extend(Q[i], octs(ms));
  }

  uint8_t *u = malloc(octs(ms) * sizeof(*u));
  for (size_t i = 0; i < CODEN; ++i) {
    reading(sockfd, u, octs(ms));
    if (unpacked_delta[i]) {
      bitxor(Q[i], u, ms);
    }
  }

  uint8_t (*QT)[octs(CODEN)] = malloc(ms * sizeof(*QT));
  transpose(QT, Q, CODEN, ms);

  if (active_security) {
    sender_check(sockfd, delta, QT, m);
  }

  uint8_t c[octs(CODEN)];
  struct {uint8_t q[octs(CODEN)]; size_t j; } q_j;
  for (q_j.j = 0; q_j.j < m; ++q_j.j) {
    for (size_t i = 0; i < codewordsn; i++) {
      memcpy(q_j.q, delta, octs(CODEN));
      // XXX. also here intrinsics fucks up
      bitcpy(c, codewords[i], CODEN);
      bitand(q_j.q, c, CODEN);
      bitxor(q_j.q, QT[q_j.j], CODEN);
      base_hash((void *) &q_j, sizeof(q_j));
#ifndef NDEBUG
      Bprint(q_j.q, octs(KAPPA));
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
