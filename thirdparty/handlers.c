#include "handlers.h"

#include <string.h>
#include <X11/Xlib.h>

#include "types.h"
#include "fwd.h"
#include "drw_types.h"

#define BUTTONMASK              (ButtonPressMask|ButtonReleaseMask)
#define MOUSEMASK               (BUTTONMASK|PointerMotionMask)
#define MAX(A, B)               ((A) > (B) ? (A) : (B))
void
resizemouse(const Arg *arg, Display* dpy) {
    extern HandlerT handler[LASTEvent];
    extern Cur *cursor[CurLast];
    extern const unsigned int snap;       
    extern Monitor *selmon;
    extern Window root;

	int ocx, ocy, nw, nh;
	Client *c;
	Monitor *m;
	XEvent ev;
	Time lasttime = 0;

	if(!(c = selmon->sel))
		return;
	if(c->isfullscreen) /* no support resizing fullscreen windows by mouse */
		return;
	restack(selmon, dpy);
	ocx = c->x;
	ocy = c->y;
	if(XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
	                None, cursor[CurResize]->cursor, CurrentTime) != GrabSuccess)
		return;
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch(ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev, dpy);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 60))
				continue;
			lasttime = ev.xmotion.time;

			nw = MAX(ev.xmotion.x - ocx - 2 * c->bw + 1, 1);
			nh = MAX(ev.xmotion.y - ocy - 2 * c->bw + 1, 1);
			if(c->mon->wx + nw >= selmon->wx && c->mon->wx + nw <= selmon->wx + selmon->ww
			&& c->mon->wy + nh >= selmon->wy && c->mon->wy + nh <= selmon->wy + selmon->wh)
			{
				if(!c->isfloating && selmon->lt[selmon->sellt]->arrange
				&& (abs(nw - c->w) > snap || abs(nh - c->h) > snap))
					togglefloating(NULL, dpy);
			}
			if(!selmon->lt[selmon->sellt]->arrange || c->isfloating)
				resize(c, c->x, c->y, nw, nh, True, dpy);
			break;
		}
	} while(ev.type != ButtonRelease);
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
	XUngrabPointer(dpy, CurrentTime);
	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
	if((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
		sendmon(c, m, dpy);
		selmon = m;
		focus(NULL, dpy);
	}
}

void
setlayout(const Arg *arg, Display* dpy) {
    extern Monitor *selmon;

	if(!arg || !arg->v || arg->v != selmon->lt[selmon->sellt])
		selmon->sellt ^= 1;
	if(arg && arg->v)
		selmon->lt[selmon->sellt] = (Layout *)arg->v;
	strncpy(selmon->ltsymbol, selmon->lt[selmon->sellt]->symbol, sizeof selmon->ltsymbol);
	if(selmon->sel)
		arrange(selmon, dpy);
	else
		drawbar(selmon);
}

const size_t LENGTH_tags;
#define LENGTH(X)               (sizeof X / sizeof X[0])
#define LENGTH2(X)              (LENGTH_ ## X)
#define TAGMASK                 ((1 << LENGTH2(tags)) - 1)
void
tag(const Arg *arg, Display* dpy) {
    extern Monitor *selmon;

	if(selmon->sel && arg->ui & TAGMASK) {
		selmon->sel->tags = arg->ui & TAGMASK;
		focus(NULL, dpy);
		arrange(selmon, dpy);
	}
}

void
togglefloating(const Arg *arg, Display* dpy) {
    extern Monitor *selmon;

	if(!selmon->sel)
		return;
	if(selmon->sel->isfullscreen) /* no support for fullscreen windows */
		return;
	selmon->sel->isfloating = !selmon->sel->isfloating || selmon->sel->isfixed;
	if(selmon->sel->isfloating)
		resize(selmon->sel, selmon->sel->x, selmon->sel->y,
		       selmon->sel->w, selmon->sel->h, False, dpy);
	arrange(selmon, dpy);
}

void
toggletag(const Arg *arg, Display* dpy) {
    extern Monitor *selmon;

	unsigned int newtags;

	if(!selmon->sel)
		return;
	newtags = selmon->sel->tags ^ (arg->ui & TAGMASK);
	if(newtags) {
		selmon->sel->tags = newtags;
		focus(NULL, dpy);
		arrange(selmon, dpy);
	}
}

void
toggleview(const Arg *arg, Display* dpy) {
    extern Monitor *selmon;

	unsigned int newtagset = selmon->tagset[selmon->seltags] ^ (arg->ui & TAGMASK);

	if(newtagset) {
		selmon->tagset[selmon->seltags] = newtagset;
		focus(NULL, dpy);
		arrange(selmon, dpy);
	}
}

void
view(const Arg *arg, Display* dpy) {
    extern Monitor *selmon;

	if((arg->ui & TAGMASK) == selmon->tagset[selmon->seltags])
		return;
	selmon->seltags ^= 1; /* toggle sel tagset */
	if(arg->ui & TAGMASK)
		selmon->tagset[selmon->seltags] = arg->ui & TAGMASK;
	focus(NULL, dpy);
	arrange(selmon, dpy);
}

void
zoom(const Arg *arg, Display* dpy) {
    extern Monitor *selmon;
	Client *c = selmon->sel;

	if(!selmon->lt[selmon->sellt]->arrange
	|| (selmon->sel && selmon->sel->isfloating))
		return;
	if(c == nexttiled(selmon->clients))
		if(!c || !(c = nexttiled(c->next)))
			return;
	pop(c, dpy);
}
