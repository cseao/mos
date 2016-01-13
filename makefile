CC=cc

PROTOCOLS= std.o kk.o
OBJS= ot.o $(PROTOCOLS)

all: ot libot

ot: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -L libot -lot -lb2 -o ot

libot:
	$(MAKE) -C libot/

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	-rm -f *.o ot

.PHONY: libot clean
