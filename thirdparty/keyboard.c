#include <X11/Xft/Xft.h>
#include <X11/Xatom.h>

#include "types.h"
#include "fwd.h"
#include "drw.h"

#define ISVISIBLE(C)            ((C->tags & C->mon->tagset[C->mon->seltags]))

static void
mappingnotify(XEvent *e, Display* dpy) {
	XMappingEvent *ev = &e->xmapping;

	XRefreshKeyboardMapping(ev);
	if(ev->request == MappingKeyboard)
		grabkeys(dpy);
}

static void
clientmessage(XEvent *e, Display* dpy) {
    extern Atom netatom[NetLast];
	XClientMessageEvent *cme = &e->xclient;
	Client *c = wintoclient(cme->window);

	if(!c)
		return;
	if(cme->message_type == netatom[NetWMState]) {
		if(cme->data.l[1] == netatom[NetWMFullscreen] || cme->data.l[2] == netatom[NetWMFullscreen])
			setfullscreen(c, (cme->data.l[0] == 1 /* _NET_WM_STATE_ADD    */
			              || (cme->data.l[0] == 2 /* _NET_WM_STATE_TOGGLE */ && !c->isfullscreen)), dpy);
	}
	else if(cme->message_type == netatom[NetActiveWindow]) {
		if(!ISVISIBLE(c)) {
			c->mon->seltags ^= 1;
			c->mon->tagset[c->mon->seltags] = c->tags;
		}
		pop(c, dpy);
	}
}

#define WIDTH(X)                ((X)->w + 2 * (X)->bw)
#define HEIGHT(X)               ((X)->h + 2 * (X)->bw)
static void
configurerequest(XEvent *e, Display* dpy) {
    extern Monitor *selmon;
	Client *c;
	Monitor *m;
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	XWindowChanges wc;

	if((c = wintoclient(ev->window))) {
		if(ev->value_mask & CWBorderWidth)
			c->bw = ev->border_width;
		else if(c->isfloating || !selmon->lt[selmon->sellt]->arrange) {
			m = c->mon;
			if(ev->value_mask & CWX) {
				c->oldx = c->x;
				c->x = m->mx + ev->x;
			}
			if(ev->value_mask & CWY) {
				c->oldy = c->y;
				c->y = m->my + ev->y;
			}
			if(ev->value_mask & CWWidth) {
				c->oldw = c->w;
				c->w = ev->width;
			}
			if(ev->value_mask & CWHeight) {
				c->oldh = c->h;
				c->h = ev->height;
			}
			if((c->x + c->w) > m->mx + m->mw && c->isfloating)
				c->x = m->mx + (m->mw / 2 - WIDTH(c) / 2); /* center in x direction */
			if((c->y + c->h) > m->my + m->mh && c->isfloating)
				c->y = m->my + (m->mh / 2 - HEIGHT(c) / 2); /* center in y direction */
			if((ev->value_mask & (CWX|CWY)) && !(ev->value_mask & (CWWidth|CWHeight)))
				configure(c, dpy);
			if(ISVISIBLE(c))
				XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
		}
		else
			configure(c, dpy);
	}
	else {
		wc.x = ev->x;
		wc.y = ev->y;
		wc.width = ev->width;
		wc.height = ev->height;
		wc.border_width = ev->border_width;
		wc.sibling = ev->above;
		wc.stack_mode = ev->detail;
		XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
	}
	XSync(dpy, False);
}

static void
configurenotify(XEvent *e, Display* dpy) {
    extern Monitor *mons;
    extern Window root;
    extern int sw;
    extern int sh;
    extern Drw *drw;
    extern int bh;
	Monitor *m;
	XConfigureEvent *ev = &e->xconfigure;
	Bool dirty;

	// TODO: updategeom handling sucks, needs to be simplified
	if(ev->window == root) {
		dirty = (sw != ev->width || sh != ev->height);
		sw = ev->width;
		sh = ev->height;
		if(updategeom(dpy) || dirty) {
			drw_resize(drw, sw, bh);
			updatebars(dpy);
			for(m = mons; m; m = m->next)
				XMoveResizeWindow(dpy, m->barwin, m->wx, m->by, m->ww, bh);
			focus(NULL, dpy);
			arrange(NULL, dpy);
		}
	}
}

static void
destroynotify(XEvent *e, Display* dpy) {
	Client *c;
	XDestroyWindowEvent *ev = &e->xdestroywindow;

	if((c = wintoclient(ev->window)))
		unmanage(c, True, dpy);
}

static void
enternotify(XEvent *e, Display* dpy) {
    extern Monitor *selmon;
    extern Window root;
	Client *c;
	Monitor *m;
	XCrossingEvent *ev = &e->xcrossing;

	if((ev->mode != NotifyNormal || ev->detail == NotifyInferior) && ev->window != root)
		return;
	c = wintoclient(ev->window);
	m = c ? c->mon : wintomon(ev->window, dpy);
	if(m != selmon) {
		unfocus(selmon->sel, True, dpy);
		selmon = m;
	}
	else if(!c || c == selmon->sel)
		return;
	focus(c, dpy);
}

static void
expose(XEvent *e, Display* dpy) {
	Monitor *m;
	XExposeEvent *ev = &e->xexpose;

	if(ev->count == 0 && (m = wintomon(ev->window, dpy)))
		drawbar(m);
}

static void
focusin(XEvent *e, Display* dpy) { /* there are some broken focus acquiring clients */
    extern Monitor *selmon;
	XFocusChangeEvent *ev = &e->xfocus;

	if(selmon->sel && ev->window != selmon->sel->win)
		setfocus(selmon->sel, dpy);
}

static void
maprequest(XEvent *e, Display* dpy) {
	static XWindowAttributes wa;
	XMapRequestEvent *ev = &e->xmaprequest;

	if(!XGetWindowAttributes(dpy, ev->window, &wa))
		return;
	if(wa.override_redirect)
		return;
	if(!wintoclient(ev->window))
		manage(ev->window, &wa, dpy);
}

static void
motionnotify(XEvent *e, Display* dpy) {
    extern Monitor *selmon;
    extern Window root;
	static Monitor *mon = NULL;
	Monitor *m;
	XMotionEvent *ev = &e->xmotion;

	if(ev->window != root)
		return;
	if((m = recttomon(ev->x_root, ev->y_root, 1, 1)) != mon && mon) {
		unfocus(selmon->sel, True, dpy);
		selmon = m;
		focus(NULL, dpy);
	}
	mon = m;
}

static void
propertynotify(XEvent *e, Display* dpy) {
    extern Window root;
	extern Atom netatom[NetLast];
	Client *c;
	Window trans;
	XPropertyEvent *ev = &e->xproperty;

	if((ev->window == root) && (ev->atom == XA_WM_NAME))
		updatestatus(dpy);
	else if(ev->state == PropertyDelete)
		return; /* ignore */
	else if((c = wintoclient(ev->window))) {
		switch(ev->atom) {
		default: break;
		case XA_WM_TRANSIENT_FOR:
			if(!c->isfloating && (XGetTransientForHint(dpy, c->win, &trans)) &&
			   (c->isfloating = (wintoclient(trans)) != NULL))
				arrange(c->mon, dpy);
			break;
		case XA_WM_NORMAL_HINTS:
			updatesizehints(c, dpy);
			break;
		case XA_WM_HINTS:
			updatewmhints(c, dpy);
			drawbars();
			break;
		}
		if(ev->atom == XA_WM_NAME || ev->atom == netatom[NetWMName]) {
			updatetitle(c, dpy);
			if(c == c->mon->sel)
				drawbar(c->mon);
		}
		if(ev->atom == netatom[NetWMWindowType])
			updatewindowtype(c, dpy);
	}
}

static void
unmapnotify(XEvent *e, Display* dpy) {
	Client *c;
	XUnmapEvent *ev = &e->xunmap;

	if((c = wintoclient(ev->window))) {
		if(ev->send_event)
			setclientstate(c, WithdrawnState, dpy);
		else
			unmanage(c, False, dpy);
	}
}

HandlerT handler[LASTEvent] = {
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

