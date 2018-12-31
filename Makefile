export PREFIX ?= /usr
export DOCDIR ?= $(DESTDIR)$(PREFIX)/share/man/man1

.PHONY: all clean
PKG_CONFIG ?= pkg-config
CFLAGS ?= -g -Wall
CFLAGS += $(shell $(PKG_CONFIG) --cflags freetype2)
LDLIBS += $(shell $(PKG_CONFIG) --libs freetype2) -lXft -lm -lXext -lX11
SRC = src/xecho.c

all: xecho xecho.1.gz

install:
	install -d "$(DESTDIR)$(PREFIX)/bin"
	install -m 0755 xecho "$(DESTDIR)$(PREFIX)/bin"
	install -d "$(DOCDIR)"
	install -g 0 -o 0 -m 0644 xecho.1.gz "$(DOCDIR)"

xecho: $(SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

xecho.1.gz:
	gzip -c < man/man1/xecho.1 > $@

clean:
	$(RM) xecho xecho.1.gz

displaytest:
	valgrind -v --leak-check=full --track-origins=yes --show-reachable=yes ./xecho -vv qmt

updatetest:
	valgrind --track-origins=yes --leak-check=full ./xecho -stdin -padding 10 -size 20 -align nw
