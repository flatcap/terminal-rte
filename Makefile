CC	= gcc
PROGS	= rte window_resize simple_terminal multiple
CFLAGS	= -g -Wall

CFLAGS  += `pkg-config ncurses gtk+-2.0 --cflags`
LDFLAGS += `pkg-config ncurses gtk+-2.0 --libs`

all:	$(PROGS)

clean:
	$(RM) $(PROGS) tags

tags:
	ctags *.[ch]

