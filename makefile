.PHONY: all test xhello clean

all:
	$(CC) -g -Wall -I/usr/include/freetype2 -lXft -lm -lXext -o xecho xecho.c

old:
	$(CC) -g -Wall -I/usr/include/freetype2 -lXft -lm -o xecho_old xecho_old.c

clean:
	rm xecho
	rm xecho_old

displaytest:
	valgrind -v --leak-check=full --track-origins=yes --show-reachable=yes ./xecho -vv qmt

updatetest:
	valgrind --track-origins=yes --leak-check=full ./xecho -stdin -padding 10 -size 20 -align nw

xhello:
	$(CC) -I/usr/include/freetype2 -lXft -o xhello xhello.c
