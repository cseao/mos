CC?=cc
CFLAGS?=-mavx -O3 -Wall -Wextra -Werror -pedantic -funroll-loops

PROTOCOLS= std.o kk.o
OBJS= ot.o bitmath.o oracle.o $(PROTOCOLS)

all: ot

ot: libot $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -L libot -lot -lb2 -o ot

libot:
	$(MAKE) -C libot/

test: bitmath_test

clean:
	-rm -f *.o ot
#	-$(MAKE) clean -C libot/

%.o: %.c
	$(CC) $(CFLAGS) -c $<

bitmath_test: bitmath.o bitmath_test.o
	$(CC) $(CFLAGS) bitmath.o bitmath_test.o -o bitmath_test
	./bitmath_test

.PHONY: libot clean bitmath_test
