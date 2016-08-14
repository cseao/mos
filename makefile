CC?=cc
CFLAGS+=-mavx -Wall -Wextra -Werror -pedantic -funroll-loops -std=c99 -g

HEADERS=bitmath.h codes.h otext.h oracle.h
OBJS= ot.o bitmath.o oracle.o std.o sender.o receiver.o codes.o

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

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $<

bitmath_test: bitmath.o bitmath.h bitmath_test.o
	$(CC) $(CFLAGS) bitmath.o bitmath_test.o -o bitmath_test

codes_test: codes.o codes.h codes_test.o bitmath.o
	$(CC) $(CFLAGS) codes.o codes_test.o bitmath.o -o codes_test

codes_bench: bitmath.o codes.o codes_bench.o
	$(CC) $(CFLAGS) $^ -o codes_bench

oracle_bench: bitmath.o oracle.o oracle_bench.o
	$(CC) $(CFLAGS) $^ -lb2  -lot -L libot -o oracle_bench

bench: codes_bench oracle_bench
	./oracle_bench | pv -s 500m -S > /dev/null
	./codes_bench < /dev/urandom | pv -s 500m -S > /dev/null

.PHONY: libot clean bitmath_test bench
