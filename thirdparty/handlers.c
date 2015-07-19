#include "handlers.h"

#include <string.h>
#include <X11/Xlib.h>

#include "types.h"
#include "drw_types.h"
#include "fwd.h"

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
