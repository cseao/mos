#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>

#include "oracle.h"
#include "otext.h"
#include "bitmath.h"
#include "libot/ot.h"

#define WORDS KAPPA*2
#define CODEN KAPPA*2
#define CODEK KAPPA

bool active_security = false;
uint8_t codewordsm = 1;
size_t codewordsn = 2;
uint8_t codewords[WORDS][CODEN/8+1] = {
#include "wh.txt"
};