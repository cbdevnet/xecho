.PHONY: all test xhello clean

all:
	$(CC) -g -Wall -I/usr/include/freetype2 -lXft -o xecho xecho.c

clean:
	rm xecho

test:
	valgrind --leak-check=full ./xecho qmt

xhello:
	$(CC) -I/usr/include/freetype2 -lXft -o xhello xhello.c
