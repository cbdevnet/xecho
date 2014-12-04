xecho is an easy way to display text via an X Window.
It takes input from the commandline or via stdin and
tries to render it at the largest font size the window
permits.

Additional options allow fixing customizing the display
for special cases, such as signage or status displays.

xecho aims to have minimal dependencies and bloat,
directly using Xlib and Xft for it's functionality.
Double buffering is done by using the X Double
Buffering Extension (XDBE) and can be disabled.

The latest development version can be checked out via 
git from http://git.services.cbcdn.com/xecho/
Tagged releases are archived at 
	http://dev.cbcdn.com/xecho/

Options:
--			Stop argument parsing
-font <fontspec>	Font to be used
-bc <colorspec>		Background color
-fc <colorspec>		Text color
-size <n>		Set static font size
-maxsize <n>		Set maximum size for scaling
-align <alignspec>	Align text
-padding <n>		Pad entire text
-linespacing <n>	Pad between lines

Flags:
-stdin			Read text from stdin
-independent-lines	Scale lines independently
-debugboxes		Draw debug boxes
-disable-text		Do not draw text
-disable-doublebuffer	What it says on the tin
-v[v[v[v]]]		Increase verbosity

Where <colorspec> is either an X Color name (blue, red,
yellow etc) or an HTML-style RGB value (#rrggbb),
<fontspec> is a freetype font name (e.g. verdana, monospace)
and <alignspec> is one of n|ne|e|se|s|sw|w|nw

Options must be given before a text argument starts.
Command line option parsing can be stopped with --,
eg.: ./xecho -bc blue -tc yellow -- -stdin is cool!

Text passed via the command line is scanned once for
control character encodings (\n and \\), which
are replaced by their ASCII codepoints.

Control characters are handled as follows

	\n		Starts new line
	\f		Clears display
	\r		Clears current line
	\b		Backspace

Build prerequisites:
	- libxft-dev
	- libx11-dev
	- libxext-dev
	- A C compiler (tcc does the trick)

To compile, simply run make.

