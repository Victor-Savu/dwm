#ifndef DRW_H
#define DRW_H

typedef struct Drw Drw;
typedef struct Fnt Fnt;
typedef struct ClrScheme ClrScheme;
typedef struct Extnts Extnts;
typedef struct Cur Cur;

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
