.PHONY: all test xhello clean

all:
	$(CC) -g -Wall -I/usr/include/freetype2 -lXft -lm -o xecho xecho.c

old:
	$(CC) -g -Wall -I/usr/include/freetype2 -lXft -lm -o xecho_old xecho_old.c

clean:
	rm xecho

test:
	valgrind -v --leak-check=full --track-origins=yes --show-reachable=yes ./xecho -vv qmt

xhello:
	$(CC) -I/usr/include/freetype2 -lXft -o xhello xhello.c
