#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "oracle.h"
#include "otext.h"
#include "bitmath.h"
#include "libot/ot.h"

static uint8_t *delta;
static bitmatrix_t QT;

static bool sender_check(const int sockfd, const size_t m)
{
  bitmatrix_t mu = new_bitmatrix(SSEC, m);
  uint8_t q[SSEC][octs(code->n)];
  uint8_t t[SSEC][octs(code->n)];
  uint8_t c[SSEC][octs(code->n)];
  uint8_t w[SSEC][octs(code->k)];
  bool pass = true;

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
    readbits(sockfd, w[i], code->k);
    encode(code, c[i], w[i]);
    bitxor(q[i], t[i], code->n);
    bitand(c[i], delta, code->n);

    pass &= biteq(c[i], q[i], code->n);
  }

  free_bitmatrix(mu);
  return pass;
}


bitmatrix_t kk_sender(int sockfd, size_t m)
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
  transpose(&QT, &Q, code->n, ms);

  if (active_security && !sender_check(sockfd, m)) {
    PROTOCOL_ABORT("Check Failed!");
  }

  uint8_t q[octs(code->n)];
  uint8_t w[octs(code->k)];
  bitmatrix_t V = new_bitmatrix(m * codewordsn, KAPPA);

#pragma omp parallel for private(q, w)
  for (size_t j = 0; j < m; ++j) {
    bitset_zero(w, code->k);
    for (size_t i = 0; i < codewordsn; i++) {
      encode(code, q, w);
      bitand(q, delta, code->n);
      bitxor(q, row(QT, j), code->n);

      hashbits(row(V, j*codewordsn + i), q, j, code->n);
      next_word(w);
    }
  }
  free_bitmatrix(Q);
  free_bitmatrix(QT);
  free(u);
  free(delta);

  return V;
}
