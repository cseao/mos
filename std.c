#include <fcntl.h>
#include <unistd.h>

#include "libot/ot.h"

void std_sender(SENDER *s, int sockfd, int nOTs)
{
  int p[2];
  uint8_t key[2][HASHBYTES];

  if (pipe(p) == -1) {
    perror("Cannot create pipe");
    exit(EXIT_FAILURE);
  }
  baseot_sender(s, sockfd, nOTs, p[1]);

  for (int i = 0; i < nOTs; i++) {
    printf("%d-th OT: ", i);
    reading(p[0], key[0], HASHBYTES);
    reading(p[0], key[1], HASHBYTES);
    for (int k = 0; k < HASHBYTES; k++) {
      printf("%.2X", key[0][k]);
    }
    printf(" ");
    for (int k = 0; k < HASHBYTES; k++) {
      printf("%.2X", key[1][k]);
    }
    printf("\n");
  }
}

void std_receiver(RECEIVER *r, int sockfd, int nOTs)
{
  int p[2];
  if (pipe(p) == -1) {
    perror("Cannot create pipe");
    exit(EXIT_FAILURE);
  }

  uint8_t choices[nOTs];
  randombytes(choices, nOTs);
  for (int i = 0; i < nOTs; ++i) {
    choices[i] &= 1;
    printf("choose bit = %d\n", choices[i]);
  }

  uint8_t key[HASHBYTES];
  baseot_receiver(r, sockfd, nOTs, choices, p[1]);
  for (int i = 0; i < nOTs; ++i) {
    reading(p[0], key, HASHBYTES);
    for (int k = 0; k < HASHBYTES; k++) {
      printf("%.2X", key[k]);
    }
    printf("\n");
  }
}
