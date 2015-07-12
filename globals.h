#ifndef GLOBALS_H
#define GLOBALS_H
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "types.h"
#include "drw.h"

/* variables */
extern const char broken[];
extern char stext[256];
extern int screen;
extern int sw, sh;           /* X display screen geometry width, height */
extern int bh, blw;      /* bar geometry */
extern unsigned int numlockmask;
extern void (*handler[LASTEvent]) (XEvent *);
extern Atom wmatom[WMLast], netatom[NetLast];
extern Bool running;
extern Cur *cursor[CurLast];
extern ClrScheme scheme[SchemeLast];
extern Display *dpy;
extern Drw *drw;
extern Monitor *mons, *selmon;
extern Window root;

#endif
