#ifndef DRW_H
#define DRW_H

/* See LICENSE file for copyright and license details. */
#include <X11/Xft/Xft.h>

#define DRW_FONT_CACHE_SIZE 32

typedef struct {
	unsigned long pix;
	XftColor rgb;
} Clr;

typedef struct {
	Cursor cursor;
} Cur;

typedef struct {
	Display *dpy;
	int ascent;
	int descent;
	unsigned int h;
	XftFont *xfont;
	FcPattern *pattern;
} Fnt;

typedef struct {
	Clr *fg;
	Clr *bg;
	Clr *border;
} ClrScheme;

typedef struct {
	unsigned int w, h;
	Display *dpy;
	int screen;
	Window root;
	Drawable drawable;
	GC gc;
	ClrScheme *scheme;
	size_t fontcount;
	Fnt *fonts[DRW_FONT_CACHE_SIZE];
} Drw;

typedef struct {
	unsigned int w;
	unsigned int h;
} Extnts;

/* Drawable abstraction */
void drw_resize(Drw *drw, unsigned int w, unsigned int h);

/* Fnt abstraction */
Fnt *drw_font_create(Drw *drw, const char *fontname);
void drw_font_free(Fnt *font);
void drw_font_getexts(Fnt *font, const char *text, unsigned int len, Extnts *extnts);
unsigned int drw_font_getexts_width(Fnt *font, const char *text, unsigned int len);

/* Drawing context manipulation */
void drw_setfont(Drw *drw, Fnt *font);
void drw_setscheme(Drw *drw, ClrScheme *scheme);

/* Drawing functions */
void drw_rect(Drw *drw, int x, int y, unsigned int w, unsigned int h, int filled, int empty, int invert);
int drw_text(Drw *drw, int x, int y, unsigned int w, unsigned int h, const char *text, int invert);
#endif
