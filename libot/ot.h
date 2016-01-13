#pragma once

#include "cpucycles.h"
#include "randombytes.h"
#include "ot_config.h"
#include "ot_sender.h"
#include "ot_receiver.h"
#include "network.h"

void baseot_sender(SENDER *sender, int newsockfd, int nOTs);
void baseot_receiver(RECEIVER *receiver, int sockfd, int nOTs);
