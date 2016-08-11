#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>

#include "codes.h"
#include "oracle.h"
#include "otext.h"
#include "bitmath.h"
#include "libot/ot.h"

bool active_security = false;
const code_t *code;
uint8_t codewordsm = 1;
size_t codewordsn = 2;
