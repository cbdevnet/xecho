xecho is an easy way to display text via an X Window.
It takes input from the commandline or via stdin and
tries to render it at the largest font size the window
permits.

Options:
-font <fontspec>	Font to be used
-bc <colorspec>		Background color
-tc <colorspec>		Text color

Where <colorspec> is either an X Color name (blue, red,
yellow etc) or an HTML-style RGB value (#rrggbb) and
<fontspec> is a freetype font name (e.g. verdana, monospace)
