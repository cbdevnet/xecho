.PHONY: all xhello

all:
	cc -I/usr/include/freetype2 -lXft -o xecho xecho.c

xhello:
	cc -I/usr/include/freetype2 -lXft -o xhello xhello.c
