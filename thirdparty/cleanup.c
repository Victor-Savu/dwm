#include "types.h"
#include "drw.h" 
#include "fwd.h"
#include "handlers.h"
#include "drw_types.h"

void
cleanupmon(Monitor *mon, Display* dpy) {
	extern Monitor *mons;
	Monitor *m;

	if(mon == mons)
		mons = mons->next;
	else {
		for(m = mons; m && m->next != mon; m = m->next);
		m->next = mon->next;
	}
	XUnmapWindow(dpy, mon->barwin);
	XDestroyWindow(dpy, mon->barwin);
	free(mon);
}


static void drw_cur_free(Drw *drw, Cur *cursor);
static void drw_clr_free(Clr *clr);
static void drw_free(Drw *drw);

void
cleanup(Display* dpy) {
	extern Window root;
	extern Cur *cursor[CurLast];
	extern ClrScheme scheme[SchemeLast];
	extern Atom netatom[NetLast];
	extern Monitor *selmon;
	extern Monitor *mons;
	extern Drw *drw;

	Arg a = {.ui = ~0};
	Layout foo = { "", NULL };
	Monitor *m;

	view(&a, dpy);
	selmon->lt[selmon->sellt] = &foo;
	for(m = mons; m; m = m->next)
		while(m->stack)
			unmanage(m->stack, False, dpy);
	XUngrabKey(dpy, AnyKey, AnyModifier, root);
	while(mons)
		cleanupmon(mons, dpy);
	drw_cur_free(drw, cursor[CurNormal]);
	drw_cur_free(drw, cursor[CurResize]);
	drw_cur_free(drw, cursor[CurMove]);
	drw_clr_free(scheme[SchemeNorm].border);
	drw_clr_free(scheme[SchemeNorm].bg);
	drw_clr_free(scheme[SchemeNorm].fg);
	drw_clr_free(scheme[SchemeSel].border);
	drw_clr_free(scheme[SchemeSel].bg);
	drw_clr_free(scheme[SchemeSel].fg);
	drw_free(drw);
	XSync(dpy, False);
	XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
	XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
}

void
drw_cur_free(Drw *drw, Cur *cursor) {
	if(!drw || !cursor)
		return;
	XFreeCursor(drw->dpy, cursor->cursor);
	free(cursor);
}

void
drw_clr_free(Clr *clr) {
	if(clr)
		free(clr);
}

void
drw_free(Drw *drw) {
	size_t i;
	for (i = 0; i < drw->fontcount; i++) {
		drw_font_free(drw->fonts[i]);
	}
	XFreePixmap(drw->dpy, drw->drawable);
	XFreeGC(drw->dpy, drw->gc);
	free(drw);
}

