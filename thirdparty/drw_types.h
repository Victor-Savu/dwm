#ifndef DRW_TYPES_H
#define DRW_TYPES_H

#include "drw.h"

#include <X11/Xft/Xft.h>

#define DRW_FONT_CACHE_SIZE 32

typedef struct {
	unsigned long pix;
	XftColor rgb;
} Clr;

struct Cur {
	Cursor cursor;
};

struct Fnt {
	Display *dpy;
	int ascent;
	int descent;
	unsigned int h;
	XftFont *xfont;
	FcPattern *pattern;
};

struct ClrScheme {
	Clr *fg;
	Clr *bg;
	Clr *border;
};

struct Drw {
	unsigned int w, h;
	Display *dpy;
	int screen;
	Window root;
	Drawable drawable;
	GC gc;
	ClrScheme *scheme;
	size_t fontcount;
	Fnt *fonts[DRW_FONT_CACHE_SIZE];
};

struct Extnts {
	unsigned int w;
	unsigned int h;
};

#endif
