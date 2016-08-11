CC?=cc
CFLAGS+=-mavx -Wall -Wextra -Werror -pedantic -funroll-loops -std=c99 -g

PROTOCOLS= std.o kk.o
OBJS= ot.o bitmath.o oracle.o $(PROTOCOLS)

all: ot

ot: libot $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -L libot -lot -lb2 -o ot

libot:
	$(MAKE) -C libot/

test: bitmath_test codes_test
	./bitmath_test
	./codes_test

clean:
	-rm -f *.o ot
#	-$(MAKE) clean -C libot/

%.o: %.c
	$(CC) $(CFLAGS) -c $<

bitmath_test: bitmath.o bitmath_test.o
	$(CC) $(CFLAGS) bitmath.o bitmath_test.o -o bitmath_test

codes_test: codes.o codes_test.o bitmath.o
	$(CC) $(CFLAGS) codes.o codes_test.o bitmath.o -o codes_test

.PHONY: libot clean bitmath_test
