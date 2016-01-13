#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "ot.h"


void baseot_sender(SENDER *sender, int newsockfd, int nOTs) {
    int i, j, k;
    unsigned char S_pack[PACKBYTES];
    unsigned char Rs_pack[4 * PACKBYTES];
    unsigned char keys[2][4][HASHBYTES];

    sender_genS(sender, S_pack);
    writing(newsockfd, S_pack, sizeof(S_pack));

    for (i = 0; i < nOTs; i += 4) {
        reading(newsockfd, Rs_pack, sizeof(Rs_pack));

        sender_keygen(sender, Rs_pack, keys);

        for (j = 0; j < 4; j++) {
          printf("%4d-th sender keys:", i + j);

          for (k = 0; k < HASHBYTES; k++)
            printf("%.2X", keys[0][j][k]);
          printf(" ");
          for (k = 0; k < HASHBYTES; k++)
            printf("%.2X", keys[1][j][k]);
          printf("\n");
        }

        printf("\n");
    }
}


void baseot_receiver(RECEIVER *receiver, int sockfd, int nOTs) {
    int i, j, k;

    unsigned char Rs_pack[4 * PACKBYTES];
    unsigned char keys[4][HASHBYTES];
    unsigned char cs[4];

    reading(sockfd, receiver->S_pack, sizeof(receiver->S_pack));
    receiver_procS(receiver);

    receiver_maketable(receiver);

    for (i = 0; i < nOTs; i += 4) {
        randombytes(cs, sizeof(cs));

        for (j = 0; j < 4; j++) {
            cs[j] &= 1;
            printf("%4d-th choose bit = %d\n", i + j, cs[j]);
        }

        receiver_rsgen(receiver, Rs_pack, cs);

        writing(sockfd, Rs_pack, sizeof(Rs_pack));

        receiver_keygen(receiver, keys);

        for (j = 0; j < 4; j++) {
          printf("%4d-th reciever key:", i + j);

          for (k = 0; k < HASHBYTES; k++)
            printf("%.2X", keys[j][k]);
          printf("\n");
        }
    }
}
