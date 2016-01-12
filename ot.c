#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "ot_sender.h"
#include "ot_receiver.h"

#include "ot_config.h"
#include "network.h"
#include "cpucycles.h"
#include "randombytes.h"


void ot_sender_test(SENDER *sender, int newsockfd) {
    int i, j, k;
    unsigned char S_pack[PACKBYTES];
    unsigned char Rs_pack[4 * PACKBYTES];
    unsigned char keys[2][4][HASHBYTES];

    sender_genS(sender, S_pack);
    writing(newsockfd, S_pack, sizeof(S_pack));

    for (i = 0; i < NOTS; i += 4) {
        reading(newsockfd, Rs_pack, sizeof(Rs_pack));

        sender_keygen(sender, Rs_pack, keys);

        if (VERBOSE) {
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
}

#define START_TIMEIT() long long __t = 0; __t -= cpucycles_amd64cpuinfo()
#define END_TIMEIT()   __t += cpucycles_amd64cpuinfo()
#define GET_TIMEIT()   __t

int sender_main(int port) {
    int sockfd;
    int newsockfd;
    int rcvbuf = BUFSIZE;
    int reuseaddr = 1;

    SENDER sender;

    sockfd = server_listen(port);
    newsockfd = server_accept(sockfd);

    if (setsockopt(newsockfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf)) !=
        0) {
        perror("ERROR setsockopt");
        exit(-1);
    }
    if (setsockopt(newsockfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr,
                   sizeof(reuseaddr)) != 0) {
        perror("ERROR setsockopt");
        exit(-1);
    }
    START_TIMEIT();
    ot_sender_test(&sender, newsockfd);
    END_TIMEIT();
    printf("[n=%ld] Elapsed time:  %lld cycles\n", nOTs, GET_TIMEIT());

    shutdown(newsockfd, 2);
    shutdown(sockfd, 2);

    return 0;
}

void ot_receiver_test(RECEIVER *receiver, int sockfd) {
    int i, j, k;

    unsigned char Rs_pack[4 * PACKBYTES];
    unsigned char keys[4][HASHBYTES];
    unsigned char cs[4];

    reading(sockfd, receiver->S_pack, sizeof(receiver->S_pack));
    receiver_procS(receiver);

    receiver_maketable(receiver);

    for (i = 0; i < NOTS; i += 4) {
        randombytes(cs, sizeof(cs));

        for (j = 0; j < 4; j++) {
            cs[j] &= 1;

            if (VERBOSE)
                printf("%4d-th choose bit = %d\n", i + j, cs[j]);
        }

        receiver_rsgen(receiver, Rs_pack, cs);

        writing(sockfd, Rs_pack, sizeof(Rs_pack));

        receiver_keygen(receiver, keys);

        if (VERBOSE) {
            for (j = 0; j < 4; j++) {
                printf("%4d-th reciever key:", i + j);

                for (k = 0; k < HASHBYTES; k++)
                    printf("%.2X", keys[j][k]);
                printf("\n");
            }
        }
    }
}

int receiver_main(const char *host, const int port) {
    int sockfd;
    int sndbuf = BUFSIZE;
    int flag = 1;

    long long t = 0;

    RECEIVER receiver;

    client_connect(&sockfd, host, port);

    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(int)) != 0) {
        perror("ERROR setsockopt");
        exit(-1);
    }
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int)) != 0) {
        perror("ERROR setsockopt");
        exit(-1);
    }

    START_TIMEIT();
    ot_receiver_test(&receiver, sockfd);
    END_TIMEIT();
    printf("[n=%ld] Elapsed time:  %lld cycles\n", nOTs, GET_TIMEIT());

    shutdown(sockfd, 2);

    return 0;
}

static const char* short_options = "hH:p:n:";
static const struct option long_options[] = {
  {"help", no_argument, 0, 'h'},
  {"host", required_argument, 0, 'H'},
  {"port", required_argument, 0, 'p'},
  {0, 0, 0, 0}
};

const char help_message[] =
"Usage:\n"
"    ot <role> [options]\n"
"    ot -h | --help\n"
"\n"
"Options:\n"
"-h --help                      Shows this screen.\n"
"-n INT                         Number of OTs [default: 1e6].\n"
"-H HOST, --host HOST           IP-address [default: localhost].\n"
"-p INT, --port INT             IP-port [default: 7766].\n"
"";

const char usage_pattern[] =
"Usage:\n"
"    ot <role> [options]\n"
"    ot -h | --help\n"
"";

void usage()
{
  fputs(usage_pattern, stderr);
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
  char *host = "localhost";
  int port = 1337;

  if (argc < 2) {
    usage();
  }
  char *role = argv[1];

  char opt;
  int option_index;
  while ((opt=getopt_long(argc, argv,
                          short_options, long_options,
                          &option_index)) != -1)
    switch (opt) {
    case 'h':
      fputs(help_message, stdout);
      exit(EXIT_SUCCESS);
      break;
    case 'H':
      host = optarg;
      break;
    case 'p':
      port = atoi(optarg);
      break;
    case 'n':
      nOTs = atol(optarg);
      break;
    case '?':
    default:
      usage();
    }

  if (!strcmp("sender", role))  {
    sender_main(port);
  } else if (!strcmp("receiver", role)) {
    receiver_main(host, port);
  } else {
    usage();
  }

  return 0;
}
