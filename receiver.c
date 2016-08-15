#include <stdint.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>

#include "oracle.h"
#include "otext.h"
#include "bitmath.h"
#include "libot/ot.h"

static bitmatrix_t choices;
static bitmatrix_t T;


static void receiver_check(const int sockfd, const size_t m)
{
  bitmatrix_t mu = new_bitmatrix(SSEC, m);
  bitmatrix_t t = new_bitmatrix(SSEC, code->n);
  uint8_t w[SSEC][octs(code->k)];

  /* Read μs from the network */
  for (int i = 0; i < SSEC; ++i) {
    readbits(sockfd, row(mu, i), KAPPA);
    prgbits(row(mu, i), m);
    /* Initialize checks to base otp */
    bitcpy(row(t, i), row(T, m+i), code->n);
    bitcpy(w[i], row(choices, m+i), code->k);
  }

  /* Compute check values */
  for (size_t j = 0; j < m; ++j) {
    for (int i = 0; i < SSEC; ++i) {
      if (getbit(row(mu, i), j)) {
        bitxor(w[i], row(choices, j), code->k);
        bitxor(row(t, i), row(T, j), code->n);
      }
    }
  }

  /* Send check values */
  for (int i = 0; i < SSEC; ++i) {
    writebits(sockfd, row(t, i), code->n);
    writebits(sockfd, &w[i], code->k);
  }
  free_bitmatrix(mu);
  free_bitmatrix(t);
}

bitmatrix_t kk_receiver(int sockfd, size_t m) {
  int p[2];
  if (pipe(p) == -1) {
    perror("Cannot create pipe");
    exit(EXIT_FAILURE);
  }

  /* let m' = m + s. */
  size_t ms = active_security? m + SSEC : m;

  /* note: rows here are columns in the paper. */
  baseot_sender(sockfd, code->n, p[1]);
  bitmatrix_t T0 = new_bitmatrix(code->n, ms);
  bitmatrix_t T1 = new_bitmatrix(code->n, ms);
  for (size_t i = 0; i < code->n; i++) {
    readbits(p[0], row(T0, i), KAPPA);
    prgbits(row(T0, i), ms);
    readbits(p[0], row(T1, i), KAPPA);
    prgbits(row(T1, i), ms);
  }

  bitmatrix_t C = new_bitmatrix(ms, code->n);
  choices = new_bitmatrix(ms, 8);
  randombits(choices.M, ms * code->k);
  for (size_t i = 0; i < ms; ++i) {
    bitmask(row(choices, i), codewordsn);
    encode(code, row(C, i), row(choices, i));
  }

  bitmatrix_t CT = new_bitmatrix(code->n, ms);
  transpose(&CT, &C, ms, code->n);
  free_bitmatrix(C);

  uint8_t *u = bitalloc(ms);
  for (size_t i = 0; i < code->n; ++i) {
    bitcpy(u, row(CT, i), ms);
    bitxor(u, row(T0, i), ms);
    bitxor(u, row(T1, i), ms);
    writebits(sockfd, u, ms);
  }

  T = new_bitmatrix(ms, code->n);
  transpose(&T, &T0, code->n, ms);
  free_bitmatrix(T0);
  free_bitmatrix(T1);

  if (active_security) receiver_check(sockfd, m);

  bitmatrix_t V = new_bitmatrix(m, KAPPA);
#pragma omp parallel for
  for (size_t j = 0; j < m; ++j) {
    hash(row(V, j), row(T, j), j, octs(code->n));
  }

  free(u);
  free_bitmatrix(choices);
  free_bitmatrix(CT);
  free_bitmatrix(T);

  return V;
}
