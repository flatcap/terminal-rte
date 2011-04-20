CC	= gcc
PKG	= rte
CFLAGS	= -g -Wall

#CFLAGS  += `pkg-config gtk+-2.0 --cflags`
#LDFLAGS += `pkg-config gtk+-2.0 --libs`

all:	$(PKG)

clean:
	$(RM) $(PKG) tags

tags:
	ctags *.[ch]

