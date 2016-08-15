#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>

#include "libot/ot.h"
#include "codes.h"
#include "otext.h"

static size_t nOTs = 1 << 10;
bool active_security = false;
uint8_t codewordsm = 1;
size_t codewordsn = 2;
code_t *code = &wh;


#define START_TIMEIT()                                          \
  struct timeval __start, __end; gettimeofday(&__start, NULL)
#define END_TIMEIT()                                                    \
  gettimeofday(&__end, NULL);                                           \
  double __sdiff = (__end.tv_sec - __start.tv_sec), __udiff = (__end.tv_usec - __start.tv_usec)
#define GET_TIMEIT()                            \
  __sdiff + __udiff * 1e-6
#define TIMEIT_FORMAT "%lf"

static int sender_main(int port) {
    int sockfd;
    int newsockfd;
    int rcvbuf = BUFSIZE;
    bitmatrix_t V;

    sockfd = server_listen(port);
    newsockfd = server_accept(sockfd);

    if (setsockopt(newsockfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf)) !=
        0) {
        perror("ERROR setsockopt");
        exit(-1);
    }

    START_TIMEIT();
    V = kk_sender(newsockfd, nOTs);
    END_TIMEIT();

#ifndef NDEBUG
    for (size_t j = 0; j < nOTs; ++j) {
      for (size_t i = 0; i < codewordsn; i++) {
        Bprint(row(V, j*codewordsn+i), octs(KAPPA));
        printf("\t");
      }
      printf("\n");
    }
#endif

    free_bitmatrix(V);
    printf("%ld OTs sender: " TIMEIT_FORMAT " seconds\n", nOTs, GET_TIMEIT());

    uint8_t end = 0x42;
    writing(newsockfd, &end, 1);
    shutdown(newsockfd, 2);
    shutdown(sockfd, 2);

    return 0;
}

static int receiver_main(const char *host, const int port) {
    int sockfd;
    int sndbuf = BUFSIZE;
    bitmatrix_t V;

    client_connect(&sockfd, host, port);

    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(int)) != 0) {
        perror("ERROR setsockopt");
        exit(-1);
    }

    START_TIMEIT();
    V = kk_receiver(sockfd, nOTs);
    END_TIMEIT();

#ifndef NDEBUG
    for (size_t j = 0; j < nOTs; ++j) {
      Bprint(row(V, j), octs(KAPPA));
      printf("\n");
    }
#endif

    free_bitmatrix(V);
    printf("%ld OTs receiver: " TIMEIT_FORMAT " seconds\n", nOTs, GET_TIMEIT());

    uint8_t end;
    reading(sockfd, &end, 1);
    assert(end == 0x42);
    shutdown(sockfd, 2);
    return 0;
}

static int both_main(const char *host, const int port) {
  int spid;
  if ((spid = fork()) == 0) {
    sender_main(port);
    return 0;
  }

  int rpid;
  if ((rpid = fork()) == 0) {
      receiver_main(host, port);
      return 0;
  }
  waitpid(spid, NULL, 0);
  waitpid(rpid, NULL, 0);
  return 0;
}

static const char* short_options = "hH:p:m:n:aC:";

static const struct option long_options[] = {
  {"help", no_argument, 0, 'h'},
  {"host", required_argument, 0, 'H'},
  {"port", required_argument, 0, 'p'},
  {"nots", required_argument, 0, 'm'},
  {"out-of", required_argument, 0, 'n'},
  {"active", no_argument, 0, 'a'},
  {"code", required_argument, 0, 'C'},
  {0, 0, 0, 0}
};

const char help_message[] =
"Usage:\n"
"    ot <role> [options]\n"
"    ot -h | --help\n"
"\n"
"Options:\n"
"-h --help                      Shows this screen.\n"
"-m INT                         log2 number of OTs [default: 7].\n"
"-H HOST, --host HOST           IP-address [default: localhost].\n"
"-p INT, --port INT             IP-port [default: 1337].\n"
"-n INT, --out-of INT           Do 1-out-of-n oblivious transfer\n"
"-a, --active                   Perform active-security checks.\n"
"-C, --code                     Linear code to be used [default: WH]\n"
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


code_t *code_from_string(const char *s)
{
  if (!strcmp(s, wh.name)) {
    return &wh;
  } else if (!strcmp(s, repetition.name)) {
    return &repetition;
  } else {
    fputs("Invalid Linear Code.\n", stderr);
    exit(EXIT_FAILURE);
  }
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
      nOTs = 1 << atol(optarg);
      break;
    case 'n':
      /* XXX. here we assume the input will be of the form 2^{optarg} */
      codewordsn = atoi(optarg);
      break;
    case 'a':
      active_security = true;
      break;
    case 'C':
      code = code_from_string(optarg);
      break;
    case '?':
    default:
      usage();
    }
  }

  // XXX.
  // There's a bug in reaceiver.c that produces a segmentation fault when the number
  // of OTs is not divisible by 128. Right now I don't have enough time to investigate.
  // Remove the following line to reproduce.
  // (Note: tests have always been done on multiples of 128 in the past.)
  if (active_security) nOTs -= (nOTs + SSEC) % 128;
  // XXX.
  // There's a bug in receiver.c that produces a malloc() corruption when the number of OTs
  // is less than 64. Right now I don't have enough time to investigate.
  if (nOTs < 64) usage();

  load_code(code);
  if (!strcmp("sender", role))  {
    sender_main(port);
  } else if (!strcmp("receiver", role)) {
    receiver_main(host, port);
  } else if (!strcmp("both", role)) {
    both_main(host, port);
  } else {
    usage();
  }
  unload_code(code);

  return 0;
}
