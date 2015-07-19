#include "types.h"
#include <X11/Xlib.h>

extern HandlerT handler[LASTEvent];
extern Bool running;
void
run(Display* dpy) {
	XEvent ev;
	/* main event loop */
	XSync(dpy, False);
	while(running && !XNextEvent(dpy, &ev))
		if(handler[ev.type])
			handler[ev.type](&ev, dpy); /* call handler */
}


