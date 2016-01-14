#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "ot.h"

/**
 * Perform SimpleOT as a sender, where:
 * - sockfd is the file descriptor to interact with the receiver.
 * - nOTs is the number of OTs to be performed
 * - outfd is the file descriptor for writing the choices as soon as they come out.
 *   Results are in the form {(x_0, x_1)}_j.
 *  The OT length is HASHBYTES.
 */
void baseot_sender(int newsockfd, int nOTs, int outfd) {
    int i, j;
    unsigned char S_pack[PACKBYTES];
    unsigned char Rs_pack[4 * PACKBYTES];
    unsigned char keys[2][4][HASHBYTES];
    SENDER sender;

    sender_genS(&sender, S_pack);
    writing(newsockfd, S_pack, sizeof(S_pack));

    for (i = 0; i < nOTs; i += 4) {
        reading(newsockfd, Rs_pack, sizeof(Rs_pack));

        sender_keygen(&sender, Rs_pack, keys[0], keys[1]);

        for (j = 0; j < 4; j++) {
          writing(outfd, keys[0][j], HASHBYTES);
          writing(outfd, keys[1][j], HASHBYTES);
        }
    }
}

/**
 * Perform SimpleOT as a receiver, where:
 * - sockfd is the file descriptor to interact with sender.
 * - nOTs is the number of OTs to be performed.
 * - choices is a vector of nOTs bytes where each choice is identified by 1/0.
 * - outfd in the file descriptor for writing the results as soon as they come out.
 *
 * The OT length is HASHBYTES.
 */
void baseot_receiver(int sockfd, int nOTs, uint8_t *choices, int outfd) {
    int i, j;

    unsigned char Rs_pack[4 * PACKBYTES];
    unsigned char keys[4][HASHBYTES];
    RECEIVER receiver;

    reading(sockfd, receiver.S_pack, sizeof(receiver.S_pack));
    receiver_procS(&receiver);

    receiver_maketable(&receiver);

    for (i = 0; i < nOTs; i += 4) {
        receiver_rsgen(&receiver, Rs_pack, choices);
        writing(sockfd, Rs_pack, sizeof(Rs_pack));

        receiver_keygen(&receiver, keys);
        choices += 4;
        for (j = 0; j < 4; j++) {
          writing(outfd, keys[j], HASHBYTES);
        }
    }
}
