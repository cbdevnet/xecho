xecho is an easy way to display text via an X Window.
It takes input from the commandline or via stdin and
tries to render it at the largest font size the window
permits.

Additional options allow customizing the display
for special cases, such as signage or status displays.

xecho aims to have minimal dependencies and bloat,
directly using Xlib and Xft for it's functionality.
Double buffering is done by using the X Double
Buffering Extension (XDBE) and can be disabled.

This repository contains the latest development version
of xecho. Release tarballs are available at the tag page
and are archived at
	https://dev.cbcdn.com/xecho/

Should this repository ever move, the new location will be
announced there.

Options:
--			Stop argument parsing
-font <fontspec>	Font to be used
-bc <colorspec>		Background color
-fc <colorspec>		Text color
-dc <colorspec>		Debug color
-title <title>		Set window title
-size <n>		Set static font size
-maxsize <n>		Set maximum size for scaling
-align <alignspec>	Align text
-padding <n>		Pad entire text
-linespacing <n>	Pad between lines

Flags:
-stdin			Deprecated / No-op
-no-stdin		Disable text content update via stdin
-windowed		Do not force window to fullscreen
-independent-lines	Scale lines independently
-debugboxes		Draw debug boxes
-disable-text		Do not draw text
-disable-doublebuffer	What it says on the tin
-h | -help | --help	Display usage information
-v[v[v[v]]]		Increase verbosity

Where <colorspec> is either an X Color name (blue, red,
yellow etc) or an HTML-style RGB value (#rrggbb),
<fontspec> is a freetype font name (e.g. verdana, monospace)
and <alignspec> is one of n|ne|e|se|s|sw|w|nw

Options must be given before a text argument starts.
Command line option parsing can be stopped with --,
eg.: ./xecho -bc blue -fc yellow -- -help shows usage information

Text passed via the command line is scanned once for
control character encodings (\n and \\), which
are replaced by their ASCII codepoints.

By default, xecho reads text from stdin and appends it to the window
content. Control characters on stdin are handled as follows
	\n		Starts new line
	\f		Clears display
	\r		Clears current line
	\b		Backspace

Usage examples:

	while :; do printf "\f%s" "`date`" \
		&& sleep 1; done | ./xecho

	Displays the current date updated by every second.
	The output of `date` is handled by printf to avoid
	a race condition where the pipe is flushed after
	the form feed, but before date has printed its
	output, thus leading to flicker.

Build prerequisites:
	- libxft-dev
	- libx11-dev
	- libxext-dev
	- pkg-config
	- A C compiler (tcc does the trick)

To compile, simply run make.

