.PHONY: all xhello

all:
	$(CC) -g -Wall -I/usr/include/freetype2 -lXft -o xecho xecho.c

xhello:
	$(CC) -I/usr/include/freetype2 -lXft -o xhello xhello.c
