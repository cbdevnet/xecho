#include <stdio.h>
#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xft/Xft.h>
#include <ctype.h>

typedef enum /*_ALIGNMENT*/ {
	ALIGN_CENTER,
	ALIGN_LEFT,
	ALIGN_RIGHT
} TEXT_ALIGN;

typedef struct /*_CFG_ARGS*/ {
	unsigned verbosity;
	unsigned padding;
	TEXT_ALIGN alignment;
	bool independent_resize;
	bool handle_stdin;
	unsigned force_size;
	char* text_color;
	char* bg_color;
	char* font_name;
} CFG;

typedef struct /*_XDATA*/ {
	int screen;
	Display* display;
	Window main;
	XftDraw* drawable;
	XftColor text_color;
	XftColor bg_color;
} XRESOURCES;

#include "colorspec.c"
#include "arguments.c"
#include "x11.c"
