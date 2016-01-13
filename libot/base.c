#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "ot.h"


void baseot_sender(SENDER *sender, int newsockfd, int nOTs, int outfd) {
    int i, j;
    unsigned char S_pack[PACKBYTES];
    unsigned char Rs_pack[4 * PACKBYTES];
    unsigned char keys[2][4][HASHBYTES];

    sender_genS(sender, S_pack);
    writing(newsockfd, S_pack, sizeof(S_pack));

    for (i = 0; i < nOTs; i += 4) {
        reading(newsockfd, Rs_pack, sizeof(Rs_pack));

        sender_keygen(sender, Rs_pack, keys[0], keys[1]);

        for (j = 0; j < 4; j++) {
          writing(outfd, keys[0][j], HASHBYTES);
          writing(outfd, keys[1][j], HASHBYTES);
        }
    }
}


void baseot_receiver(RECEIVER *receiver, int sockfd, int nOTs, uint8_t *choices, int outfd) {
    int i, j;

    unsigned char Rs_pack[4 * PACKBYTES];
    unsigned char keys[4][HASHBYTES];

    reading(sockfd, receiver->S_pack, sizeof(receiver->S_pack));
    receiver_procS(receiver);

    receiver_maketable(receiver);

    for (i = 0; i < nOTs; i += 4) {
        receiver_rsgen(receiver, Rs_pack, choices);
        writing(sockfd, Rs_pack, sizeof(Rs_pack));

        receiver_keygen(receiver, keys);
        choices += 4;
        for (j = 0; j < 4; j++) {
          writing(outfd, keys[j], HASHBYTES);
        }
    }
}
