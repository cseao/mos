#pragma once

#include "libot/ot.h"
#include "bitmath.h"
#include "codes.h"

#define SSEC 40

#define writebits(from, to, howmany) writing(from, to, octs(howmany))
#define readbits(from, to, howmany)  reading(from, to, octs(howmany))

#define PROTOCOL_ABORT(MSG)                        \
  fprintf(stderr, MSG "\n");           \
  exit(EXIT_FAILURE)

extern bool active_security;
extern size_t codewordsn;
extern code_t *code;

void std_sender(int sockfd, int nOTs);
void std_receiver(int sockfd, int nOTs);

bitmatrix_t kk_sender(int sockfd, size_t nOTs);
void kk_receiver(int sockfd, size_t nOTs);
