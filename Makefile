CC	= gcc
PROGS	= multiple # window_resize rte term_resize simple_terminal
OBJ	= cache.o debug.o detab.o list.o options.o view.o
CFLAGS	= -g -Wall

CFLAGS  += `pkg-config ncurses gtk+-3.0 --cflags`
LDFLAGS += `pkg-config ncurses gtk+-3.0 --libs`

all:	$(PROGS)

clean:
	$(RM) $(PROGS) $(OBJ) tags

tags:
	ctags *.[ch]

multiple:	$(OBJ)

