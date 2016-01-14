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

  uint8_t delta[nOTs];
  randombytes(delta, nOTs);
  for (int i = 0; i < nOTs; ++i) {
    delta[i] &= 1;
    printf("choose bit = %d\n", delta[i]);
  }


  uint8_t key[m];
  baseot_receiver(sockfd, nOTs, delta, p[1]);
  for (int i = 0; i < nOTs; ++i) {
    reading(p[0], key, HASHBYTES);
    prg_extend(key, m);
    for (int k = 0; k < m; k++) {
      printf("%.2X", key[k]);
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

  uint8_t key[2][m];

  baseot_sender(sockfd, nOTs, p[1]);
  for (int i = 0; i < nOTs; i++) {
    printf("%d-th OT: ", i);
    reading(p[0], key[0], HASHBYTES);
    prg_extend(key[0], m);

    reading(p[0], key[1], HASHBYTES);
    prg_extend(key[1], m);
    for (int k = 0; k < m; k++) {
      printf("%.2X", key[0][k]);
    }
    printf(" ");
    for (int k = 0; k < m; k++) {
      printf("%.2X", key[1][k]);
    }
    printf("\n");
  }
}
