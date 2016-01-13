CC=cc

all: ot

ot: ot.o
	$(CC) $(CFLAGS) ot.o -L libot -lot -lb2 -o ot

%.o: %.c
	$(CC) $(CFLAGS) -c $<
