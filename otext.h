#pragma once

#include "libot/ot.h"
#include "bitmath.h"

#define SSEC 40

void std_sender(int sockfd, int nOTs);
void std_receiver(int sockfd, int nOTs);

void kk_sender(int sockfd, size_t nOTs);
void kk_receiver(int sockfd, size_t nOTs);
