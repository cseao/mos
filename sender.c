#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>

#include "oracle.h"
#include "otext.h"
#include "bitmath.h"
#include "libot/ot.h"


static uint8_t *delta;
static bitmatrix_t QT;

static void sender_check(const int sockfd, const size_t m)
{
  bitmatrix_t mu = new_bitmatrix(SSEC, m);
  uint8_t q[SSEC][octs(code->n)];
  uint8_t t[SSEC][octs(code->n)];
  uint8_t c[SSEC][octs(code->n)];
  uint8_t w[SSEC];
  for (int i = 0; i < SSEC; ++i) {
    randombits(row(mu, i), KAPPA);
    writebits(sockfd, row(mu, i), KAPPA);
    prgbits(row(mu, i), m);

    bitcpy(q[i], row(QT, m+i), code->n);
  }

  for (size_t j = 0; j < m; ++j) {
    for (int i = 0; i < SSEC; ++i) {
      if (getbit(row(mu, i), j)) {
        bitxor(q[i], row(QT, j), code->n);
      }
    }
  }

  for (int i = 0; i < SSEC; ++i) {
    readbits(sockfd, t[i], code->n);
    reading(sockfd, &w[i], 1);
    if (w[i] & ~codewordsm) {
      fprintf(stderr, "Check Failed!\n");
      exit(EXIT_FAILURE);
    }
    bitcpy(c[i], codewords[w[i]], code->n);
    bitxor(q[i], t[i], code->n);
    bitand(c[i], delta, code->n);

    if (!biteq(c[i], q[i], code->n)) {
      fprintf(stderr, "Check failed!\n");
      exit(EXIT_FAILURE);
    }
  }
  free_bitmatrix(mu);
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
  delta = bitalloc(code->n);
  uint8_t unpacked_delta[code->n];
  randombits(delta, code->n);
  for (size_t i = 0; i < code->n; ++i) {
    unpacked_delta[i] = getbit(delta, i);
  }

  baseot_receiver(sockfd, code->n, unpacked_delta, p[1]);
  bitmatrix_t Q = new_bitmatrix(code->n, ms);
  for (size_t i = 0; i < code->n; ++i) {
    readbits(p[0], row(Q, i), KAPPA);
    prgbits(row(Q, i), ms);
  }

  uint8_t *u = bitalloc(ms);
  for (size_t i = 0; i < code->n; ++i) {
    readbits(sockfd, u, ms);
    if (unpacked_delta[i]) {
      bitxor(row(Q, i), u, ms);
    }
  }

  QT = new_bitmatrix(ms, code->n);
  transpose(QT.M, Q.M, code->n, ms);

  if (active_security) {
    sender_check(sockfd, m);
  }

  uint8_t c[octs(code->n)];
  struct {uint8_t q[octs(CODEN)]; size_t j; } q_j;
  for (q_j.j = 0; q_j.j < m; ++q_j.j) {
    for (size_t i = 0; i < codewordsn; i++) {
      bitcpy(q_j.q, delta, code->n);
      // XXX. also here intrinsics fucks up
      bitcpy(c, codewords[i], code->n);
      bitand(q_j.q, c, code->n);
      bitxor(q_j.q, row(QT, q_j.j), code->n);
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
  free_bitmatrix(Q);
  free_bitmatrix(QT);
  free(u);
  free(delta);
}
