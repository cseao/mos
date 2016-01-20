#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>

#include "oracle.h"
#include "otext.h"
#include "bitmath.h"
#include "libot/ot.h"

#define WORDS KAPPA*2
#define CODEN KAPPA*2
#define CODEK KAPPA

bool active_security = false;
uint8_t codewordsm = 1;
size_t codewordsn = 2;
static uint8_t codewords[WORDS][CODEN/8] = {
#include "wh.txt"
};

static void
sender_check(const int sockfd, uint8_t delta[CODEN/8], uint8_t (*QT)[CODEN/8], const size_t m)
{
  uint8_t mu[SSEC][m/8];
  uint8_t q_i[CODEN/8];
  uint8_t t_i[CODEN/8];
  uint8_t c_i[CODEN/8];
  uint8_t w_i;
  for (size_t i = 0; i < SSEC; ++i) {
    randombytes(mu[i], KAPPA/8);
    writing(sockfd, mu[i], KAPPA/8);
    prg_extend(mu[i], m/8);

    reading(sockfd, t_i, CODEN/8);
    reading(sockfd, &w_i, 1);
    if (w_i & ~codewordsm) {
      fprintf(stderr, "Check Failed!\n");
      exit(EXIT_FAILURE);
    }
    memcpy(q_i, QT[m + i], CODEN/8);
    memcpy(c_i, codewords[w_i], CODEN/8);
    for (size_t j = 0; j < m; j++) {
      if (getbit(mu[i], j)) {
          bitxor(q_i, QT[j], CODEN);
      }
    }
    bitxor(q_i, t_i, CODEN);
    bitand(c_i, delta, CODEN);
    if (!biteq(c_i, q_i, CODEN)) {
      fprintf(stderr, "Check failed!\n");
      exit(EXIT_FAILURE);
    }
  }

}

void kk_sender(int sockfd, size_t m)
{
  int p[2];
  if (pipe(p) == -1) {
    perror("Cannot create pipe");
    exit(EXIT_FAILURE);
  }
  /* let m' = m + s */
  size_t ms = (m + SSEC);

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


static void
receiver_check(const int sockfd,  uint8_t *choices, uint8_t (*T)[CODEN/8], const size_t m)
{
  uint8_t mu[SSEC][m/8];
  uint8_t w_i;
  uint8_t t_i[CODEN/8];

  for (size_t i = 0; i < SSEC; ++i) {
    reading(sockfd, mu[i], KAPPA/8);
    prg_extend(mu[i], m/8);

    memcpy(t_i, T[m + i], CODEN/8);
    w_i = choices[m + i];
    for (size_t j = 0; j < m; ++j) {
      if (getbit(mu[i], j)) {
        w_i ^= choices[j];
        bitxor(t_i, T[j], CODEN);
      }
    }
    writing(sockfd, t_i, CODEN/8);
    writing(sockfd, &w_i, 1);
  }
}

void kk_receiver(int sockfd, size_t m) {
  int p[2];
  if (pipe(p) == -1) {
    perror("Cannot create pipe");
    exit(EXIT_FAILURE);
  }

  /* let m' = m + s. */
  size_t ms = m + SSEC;

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
  transpose(T, T0, CODEN, ms);

  if (active_security) {
    receiver_check(sockfd, choices, T, m);
  }

  uint8_t pad[KAPPA/8];
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
