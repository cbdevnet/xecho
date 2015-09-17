.PHONY: all clean
CFLAGS=-g -Wall -I/usr/include/freetype2 -lXft -lm -lXext -lX11

all: xecho

clean:
	rm xecho

displaytest:
	valgrind -v --leak-check=full --track-origins=yes --show-reachable=yes ./xecho -vv qmt

updatetest:
	valgrind --track-origins=yes --leak-check=full ./xecho -stdin -padding 10 -size 20 -align nw
