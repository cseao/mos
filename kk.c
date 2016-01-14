#include <fcntl.h>
#include <unistd.h>

#include <blake2.h>

#include "libot/ot.h"

#define CODEN KAPPA*2
#define CODEK KAPPA
#define KAPPA 128

static const int m = 512;

/**
 * Extends
 */
void prg_extend(uint8_t *out, size_t to)
{
  for (int times = to / HASHBYTES - 1; times > 0; times--) {
    if (blake2(out+HASHBYTES, out, NULL, HASHBYTES, HASHBYTES, 0) == -1) {
      perror("Cannot hash");
      exit(EXIT_FAILURE);
    }
    out += HASHBYTES;
  }
}

void kk_sender(int sockfd, int nOTs)
{
  int p[2];
  if (pipe(p) == -1) {
    perror("Cannot create pipe");
    exit(EXIT_FAILURE);
  }

  /* Perform the base OTs, extends them and place those in a matrix Q. */
  uint8_t delta[CODEN];
  randombytes(delta, CODEN);
  for (int i = 0; i < CODEN; ++i) {
    delta[i] &= 1;
  }
  uint8_t Q[CODEN][m];
  baseot_receiver(sockfd, CODEN, delta, p[1]);
  for (int i = 0; i < CODEN; ++i) {
    reading(p[0], Q[i], HASHBYTES);
    prg_extend(Q[i], m);

  }

  for (int i = 0; i < CODEN; ++i) {
    printf("choose bit = %d\n", delta[i]);
    for (int k = 0; k < m; k++) {
      printf("%.2X", Q[i][k]);
    }
    printf("\n");
  }
}

void kk_receiver(int sockfd, int nOTs)
{
  int p[2];
  if (pipe(p) == -1) {
    perror("Cannot create pipe");
    exit(EXIT_FAILURE);
  }

  uint8_t T0[CODEN][m], T1[CODEN][m];

  baseot_sender(sockfd, CODEN, p[1]);
  for (int i = 0; i < CODEN; i++) {
    reading(p[0], T0[i], HASHBYTES);
    prg_extend(T0[i], m);
    reading(p[0], T1[i], HASHBYTES);
    prg_extend(T1[i], m);
  }

  for (int i = 0; i < CODEN; ++i) {
    printf("%d-th OT: ", i);

    for (int k = 0; k < m; k++) {
      printf("%.2X", T0[i][k]);
    }
    printf(" ");
    for (int k = 0; k < m; k++) {
      printf("%.2X", T1[i][k]);
    }
    printf("\n");
  }
}
