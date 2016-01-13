CC=cc

OBJS= ot.o std.o
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
