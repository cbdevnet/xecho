/* 
 This file contains verbatim code taken from the Xft source,
 modified to display more debug info.

Copyright © 2001,2003 Keith Packard

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of Keith Packard not be used in
advertising or publicity pertaining to distribution of the software without
specific, written prior permission.  Keith Packard makes no
representations about the suitability of this software for any purpose.  It
is provided "as is" without express or implied warranty.

KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.


*/
static short maskbase (unsigned long m){
	short        i;
	if (!m)
		return 0;
	i = 0;
	while (!(m&1)){
		m>>=1;
		i++;
	}
	return i;
}

static short masklen (unsigned long m){
	unsigned long y;
	y = (m >> 1) &033333333333;
	y = m - y - ((y >>1) & 033333333333);
	return (short) (((y + (y >> 3)) & 030707070707) % 077);
}


Bool XftColorAllocValueDebug (Display *dpy, Visual *visual, Colormap cmap, XRenderColor *color, XftColor *result){
	if (visual->class == TrueColor){
		printf("xft_coloralloc: truecolor mode\n");
		int         red_shift, red_len;
		int         green_shift, green_len;
		int         blue_shift, blue_len;
	
		red_shift = maskbase (visual->red_mask);
		red_len = masklen (visual->red_mask);
		green_shift = maskbase (visual->green_mask);
		green_len = masklen (visual->green_mask);
		blue_shift = maskbase (visual->blue_mask);
		blue_len = masklen (visual->blue_mask);

		printf("Red Length: %d Shift: %d, Value %d\n", red_len, red_shift, color->red);
		printf("Green Length: %d Shift: %d, Value %d\n", green_len, green_shift, color->green);
		printf("Blue Length: %d Shift: %d, Value %d\n", blue_len, blue_shift, color->blue);

		int red_value=((color->red >> (16 - red_len)) << red_shift);
		int green_value= ((color->green >> (16 - green_len)) << green_shift) ;
		int blue_value=((color->blue >> (16 - blue_len)) << blue_shift);

		result->pixel = ( red_value|green_value|blue_value );
		printf("Pixel value: %d (%x=%x|%x|%x)\n", result->pixel, result->pixel, red_value, green_value, blue_value);
	}
	else{
		printf("xft_coloralloc: non-truecolor mode\n");
		XColor  xcolor;

		xcolor.red = color->red;
		xcolor.green = color->green;
		xcolor.blue = color->blue;
		if (!XAllocColor (dpy, cmap, &xcolor))
			return False;
		result->pixel = xcolor.pixel;
	}
	result->color.red = color->red;
	result->color.green = color->green;
	result->color.blue = color->blue;
	result->color.alpha = color->alpha;
	return True;
}

