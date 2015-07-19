
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "types.h"
#include "drw_types.h"

/* variables */
const char broken[] = "broken";
char stext[256];
int sw, sh;           /* X display screen geometry width, height */
int bh, blw = 0;      /* bar geometry */
unsigned int numlockmask = 0;
Atom wmatom[WMLast], netatom[NetLast];
Bool running = True;
Cur *cursor[CurLast];
ClrScheme scheme[SchemeLast];
Drw *drw;
Monitor *mons, *selmon;
Window root;
