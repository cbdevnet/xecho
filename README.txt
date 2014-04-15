xecho is an easy way to display text via an X Window.
It takes input from the commandline or via stdin and
tries to render it at the largest font size the window
permits.

xecho aims to have minimal dependencies and bloat,
directly using Xlib and Xft for it's functionality.

The latest development version can be checked out via 
git from http://git.services.cbcdn.com/xecho
Tagged releases are archived at 
	http://dev.cbcdn.com/xecho/

Options:
-font <fontspec>	Font to be used
-bc <colorspec>		Background color
-tc <colorspec>		Text color

Where <colorspec> is either an X Color name (blue, red,
yellow etc) or an HTML-style RGB value (#rrggbb) and
<fontspec> is a freetype font name (e.g. verdana, monospace)

Options must be given before a text argument starts.
Command line option parsing can be stopped with --,
eg.: ./xecho -bc blue -tc yellow -- --stdin is cool!

Build prerequisites:
	- libxft-dev
	- libx11-dev
	- A C compiler (tcc does the trick)

To compile, simply run make.

