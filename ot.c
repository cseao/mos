#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>

#include "libot/ot.h"
#include "otext.h"

static size_t nOTs = 1 << 10;
extern uint8_t codewordsm;
extern size_t codewordsn;
extern bool active_security;

#define START_TIMEIT() struct timeval __start, __end; gettimeofday(&__start, NULL)
#define END_TIMEIT()   \
  gettimeofday(&__end, NULL); \
  long int __sdiff = (__end.tv_sec - __start.tv_sec), __udiff = (__end.tv_usec - __start.tv_usec)
#define GET_TIMEIT()   __sdiff - (__udiff < 0 ? 1 : 0), __udiff + (__udiff < 0 ? 1000000 : 0)
#define TIMEIT_FORMAT "%ld.%ld"

int sender_main(int port) {
    int sockfd;
    int newsockfd;
    int rcvbuf = BUFSIZE;

    sockfd = server_listen(port);
    newsockfd = server_accept(sockfd);

    if (setsockopt(newsockfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf)) !=
        0) {
        perror("ERROR setsockopt");
        exit(-1);
    }

    START_TIMEIT();
    kk_sender(newsockfd, nOTs);
    END_TIMEIT();
    printf("[n=%ld] Elapsed time: " TIMEIT_FORMAT " seconds\n", nOTs, GET_TIMEIT());

    shutdown(newsockfd, 2);
    shutdown(sockfd, 2);

    return 0;
}

int receiver_main(const char *host, const int port) {
    int sockfd;
    int sndbuf = BUFSIZE;
    int flag = 1;

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
    kk_receiver(sockfd, nOTs);
    END_TIMEIT();
    printf("[n=%ld] Elapsed time: " TIMEIT_FORMAT " seconds\n", nOTs, GET_TIMEIT());

    shutdown(sockfd, 2);

    return 0;
}

static const char* short_options = "hH:p:m:n:a";

static const struct option long_options[] = {
  {"help", no_argument, 0, 'h'},
  {"host", required_argument, 0, 'H'},
  {"port", required_argument, 0, 'p'},
  {"nots", required_argument, 0, 'm'},
  {"out-of", required_argument, 0, 'n'},
  {"active", no_argument, 0, 'a'},
  {0, 0, 0, 0}
};

const char help_message[] =
"Usage:\n"
"    ot <role> [options]\n"
"    ot -h | --help\n"
"\n"
"Options:\n"
"-h --help                      Shows this screen.\n"
"-m INT                         Number of OTs [default: 1e6].\n"
"-H HOST, --host HOST           IP-address [default: localhost].\n"
"-p INT, --port INT             IP-port [default: 7766].\n"
"-n INT, --out-of INT           Do 1-out-of-n oblivious transfer\n"
"-a, --active                   Perform active-security checks.\n"
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
                          &option_index)) != -1) {
    /* XXX. we are not really sanitizing the input */
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
    case 'm':
      nOTs = atol(optarg);
      break;
    case 'n':
      /* XXX. here we assume the input will be of the form 2^{optarg} */
      codewordsm = atoi(optarg) - 1;
      codewordsn = atoi(optarg);
      break;
    case 'a':
      active_security = true;
      break;
    case '?':
    default:
      usage();
    }
  }

  /*
   * Our internal arithmetic functions assume that the input can always be
   * splitted in branches of 128 bits, so here we get to the nearest multiple.
   */
  nOTs -= (nOTs + SSEC) % 128;

  if (!strcmp("sender", role))  {
    sender_main(port);
  } else if (!strcmp("receiver", role)) {
    receiver_main(host, port);
  } else {
    usage();
  }

  return 0;
}
