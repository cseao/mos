#pragma once

#include "libot/ot.h"
#include "bitmath.h"
#include "codes.h"

#define SSEC 40

#define writebits(from, to, howmany) writing(from, to, octs(howmany))
#define readbits(from, to, howmany)  reading(from, to, octs(howmany))

#define PROTOCOL_ABORT()                        \
  fprintf(stderr, "Check Failed!\n");           \
  exit(EXIT_FAILURE)

extern bool active_security;
extern uint8_t codewordsm;
extern size_t codewordsn;
extern code_t *code;

void std_sender(int sockfd, int nOTs);
void std_receiver(int sockfd, int nOTs);

void kk_sender(int sockfd, size_t nOTs);
void kk_receiver(int sockfd, size_t nOTs);
