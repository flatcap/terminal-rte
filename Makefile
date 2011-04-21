CC	= gcc
PROGS	= rte window_resize simple_terminal
CFLAGS	= -g -Wall

CFLAGS  += `pkg-config ncurses --cflags`
LDFLAGS += `pkg-config ncurses --libs`

all:	$(PROGS)

clean:
	$(RM) $(PROGS) tags

tags:
	ctags *.[ch]

