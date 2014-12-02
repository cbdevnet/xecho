#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xft/Xft.h>
#include <X11/extensions/Xdbe.h>

typedef enum /*_ALIGNMENT*/ {
	ALIGN_CENTER,
	ALIGN_NORTH,
	ALIGN_EAST,
	ALIGN_SOUTH,
	ALIGN_WEST,
	ALIGN_NORTHEAST,
	ALIGN_SOUTHEAST,
	ALIGN_SOUTHWEST,
	ALIGN_NORTHWEST
} TEXT_ALIGN;

typedef struct /*_CFG_ARGS*/ {
	unsigned verbosity;
	unsigned padding;
	unsigned line_spacing;
	unsigned max_size;
	TEXT_ALIGN alignment;
	bool independent_resize;
	bool handle_stdin;
	bool debug_boxes;
	bool disable_text;
	bool double_buffer;
	double force_size;
	char* text_color;
	char* bg_color;
	char* debug_color;
	char* font_name;
} CFG;

typedef struct /*XFD_AGGREG*/ {
	int* fds;
	unsigned size;
} X_FDS;

typedef struct /*_XDATA*/ {
	int screen;
	Display* display;
	Window main;
	XdbeBackBuffer back_buffer;
	XftDraw* drawable;
	XftColor text_color;
	XftColor bg_color;
	XftColor debug_color;
	X_FDS xfds;
} XRESOURCES;

typedef struct /*_TEXT_BLOCK*/ {
	unsigned layout_x;
	unsigned layout_y;
	double size;
	char* text;
	bool active;
	bool calculated;
	XGlyphInfo extents;
} TEXTBLOCK;

#define DEFAULT_FONT "verdana"
#define DEFAULT_TEXTCOLOR "black"
#define DEFAULT_WINCOLOR "white"
#define DEFAULT_DEBUGCOLOR "red"
#define STDIN_DATA_CHUNK 512

#include "colorspec.c"
#include "arguments.c"
#include "strings.c"
#include "x11.c"
#include "logic.c"
