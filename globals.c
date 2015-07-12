
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "types.h"
#include "drw.h"

#include "fwd.h"

/* variables */
const char broken[] = "broken";
char stext[256];
int screen;
int sw, sh;           /* X display screen geometry width, height */
int bh, blw = 0;      /* bar geometry */
unsigned int numlockmask = 0;
void (*handler[LASTEvent]) (XEvent *) = {
	[ButtonPress] = buttonpress,
	[ClientMessage] = clientmessage,
	[ConfigureRequest] = configurerequest,
	[ConfigureNotify] = configurenotify,
	[DestroyNotify] = destroynotify,
	[EnterNotify] = enternotify,
	[Expose] = expose,
	[FocusIn] = focusin,
	[KeyPress] = keypress,
	[MappingNotify] = mappingnotify,
	[MapRequest] = maprequest,
	[MotionNotify] = motionnotify,
	[PropertyNotify] = propertynotify,
	[UnmapNotify] = unmapnotify
};
Atom wmatom[WMLast], netatom[NetLast];
Bool running = True;
Cur *cursor[CurLast];
ClrScheme scheme[SchemeLast];
Display *dpy;
Drw *drw;
Monitor *mons, *selmon;
Window root;
