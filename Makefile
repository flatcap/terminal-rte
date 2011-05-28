CC	= gcc
PROGS	= multiple # rte term_resize simple_terminal
OBJ	= view.o cache.o list.o debug.o detab.o
CFLAGS	= -g -Wall

CFLAGS  += `pkg-config ncurses gtk+-3.0 --cflags`
LDFLAGS += `pkg-config ncurses gtk+-3.0 --libs`

all:	$(PROGS)

clean:
	$(RM) $(PROGS) $(OBJ) tags

tags:
	ctags *.[ch]

multiple:	$(OBJ)

