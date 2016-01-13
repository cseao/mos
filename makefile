CC=cc

all: ot libot

ot: ot.o
	$(CC) $(CFLAGS) ot.o -L libot -lot -lb2 -o ot

libot:
	$(MAKE) -C libot/

%.o: %.c
	$(CC) $(CFLAGS) -c $<

.PHONY: libot
