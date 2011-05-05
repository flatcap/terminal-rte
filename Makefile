CC	= gcc
PROGS	= multiple # rte window_resize simple_terminal
OBJ	= view.o cache.o list.o debug.o
CFLAGS	= -g -Wall

CFLAGS  += `pkg-config ncurses gtk+-2.0 --cflags`
LDFLAGS += `pkg-config ncurses gtk+-2.0 --libs`

all:	$(PROGS)

clean:
	$(RM) $(PROGS) $(OBJ) tags

tags:
	ctags *.[ch]

multiple:	$(OBJ)

