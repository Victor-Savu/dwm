/* See LICENSE file for copyright and license details.
 *
 * dynamic window manager is designed like any other X client as well. It is
 * driven through handling X events. In contrast to other X clients, a window
 * manager selects for SubstructureRedirectMask on the root window, to receive
 * events about window (dis-)appearance.  Only one X connection at a time is
 * allowed to select for this event mask.
 *
 * The event handlers of dwm are organized in an array which is accessed
 * whenever a new event has been fetched. This allows event dispatching
 * in O(1) time.
 *
 * Each child of the root window is called a client, except windows which have
 * set the override_redirect flag.  Clients are organized in a linked client
 * list on each monitor, the focus history is remembered through a stack list
 * on each monitor. Each client contains a bit array to indicate the tags of a
 * client.
 *
 * Keys and tagging rules are organized as arrays and defined in config.c.
 */
#include <X11/cursorfont.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xproto.h>
#include <X11/Xft/Xft.h>
#include <X11/Xatom.h>

#include "drw_types.h"

void die(const char *errstr, ...);

#define MAX(A, B)               ((A) > (B) ? (A) : (B))
#define MIN(A, B)               ((A) < (B) ? (A) : (B))
#define BETWEEN(X, A, B)        ((A) <= (X) && (X) <= (B))

#define BUTTONMASK              (ButtonPressMask|ButtonReleaseMask)
#define MOUSEMASK               (BUTTONMASK|PointerMotionMask)
#define INTERSECT(x,y,w,h,m)    (MAX(0, MIN((x)+(w),(m)->wx+(m)->ww) - MAX((x),(m)->wx)) \
                               * MAX(0, MIN((y)+(h),(m)->wy+(m)->wh) - MAX((y),(m)->wy)))
#define LENGTH(X)               (sizeof X / sizeof X[0])
#define TEXTW(X)                (drw_text(drw, 0, 0, 0, 0, (X), 0) + drw->fonts[0]->h)
#define ISVISIBLE(C)            ((C->tags & C->mon->tagset[C->mon->seltags]))
#define HEIGHT(X)               ((X)->h + 2 * (X)->bw)
#define WIDTH(X)                ((X)->w + 2 * (X)->bw)
#define LENGTH2(X)              (LENGTH_ ## X)
#define TAGMASK                 ((1 << LENGTH2(tags)) - 1)

#include "types.h"

#include "fwd.h"
/* configuration, allows nested code to access above variables */

/* function implementations */

static void
arrangemon(Monitor *m, Display* dpy) {
	strncpy(m->ltsymbol, m->lt[m->sellt]->symbol, sizeof m->ltsymbol);
	if(m->lt[m->sellt]->arrange)
		m->lt[m->sellt]->arrange(m, dpy);
}

void
arrange(Monitor *m, Display* dpy) {
	extern Monitor *mons;
	if(m)
		showhide(m->stack, dpy);
	else for(m = mons; m; m = m->next)
		showhide(m->stack, dpy);
	if(m) {
		arrangemon(m, dpy);
		restack(m, dpy);
	} else for(m = mons; m; m = m->next)
		arrangemon(m, dpy);
}

void
attach(Client *c) {
	c->next = c->mon->clients;
	c->mon->clients = c;
}

void
attachstack(Client *c) {
	c->snext = c->mon->stack;
	c->mon->stack = c;
}

void
buttonpress(XEvent *e, Display* dpy) {
 	extern Monitor *selmon;
	extern Drw *drw;
	extern const char *tags[];
	extern const size_t LENGTH_tags;
	extern Button buttons[];
	extern int blw;
	extern char stext[256];
	extern unsigned int numlockmask;
	extern const size_t LENGTH_buttons;

	unsigned int i, x, click;
	Arg arg = {0};
	Client *c;
	Monitor *m;
	XButtonPressedEvent *ev = &e->xbutton;

	click = ClkRootWin;
	/* focus monitor if necessary */
	if((m = wintomon(ev->window, dpy)) && m != selmon) {
		unfocus(selmon->sel, True, dpy);
		selmon = m;
		focus(NULL, dpy);
	}
	if(ev->window == selmon->barwin) {
		i = x = 0;
		do
			x += TEXTW(tags[i]);
		while(ev->x >= x && ++i < LENGTH2(tags));
		if(i < LENGTH2(tags)) {
			click = ClkTagBar;
			arg.ui = 1 << i;
		}
		else if(ev->x < x + blw)
			click = ClkLtSymbol;
		else if(ev->x > selmon->ww - TEXTW(stext))
			click = ClkStatusText;
		else
			click = ClkWinTitle;
	}
	else if((c = wintoclient(ev->window))) {
		focus(c, dpy);
		click = ClkClientWin;
	}

	#define CLEANMASK(mask)         (mask & ~(numlockmask|LockMask) & (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))
	for(i = 0; i < LENGTH2(buttons); i++)
		if(click == buttons[i].click && buttons[i].func && buttons[i].button == ev->button
		&& CLEANMASK(buttons[i].mask) == CLEANMASK(ev->state))
			buttons[i].func(click == ClkTagBar && buttons[i].arg.i == 0 ? &arg : &buttons[i].arg, dpy);
}


void
configure(Client *c, Display* dpy) {
	XConfigureEvent ce;

	ce.type = ConfigureNotify;
	ce.display = dpy;
	ce.event = c->win;
	ce.window = c->win;
	ce.x = c->x;
	ce.y = c->y;
	ce.width = c->w;
	ce.height = c->h;
	ce.border_width = c->bw;
	ce.above = None;
	ce.override_redirect = False;
	XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent *)&ce);
}

void
detach(Client *c) {
	Client **tc;

	for(tc = &c->mon->clients; *tc && *tc != c; tc = &(*tc)->next);
	*tc = c->next;
}

void
detachstack(Client *c) {
	Client **tc, *t;

	for(tc = &c->mon->stack; *tc && *tc != c; tc = &(*tc)->snext);
	*tc = c->snext;

	if(c == c->mon->sel) {
		for(t = c->mon->stack; t && !ISVISIBLE(t); t = t->snext);
		c->mon->sel = t;
	}
}

Monitor *
dirtomon(int dir) {
	extern Monitor *selmon;
	extern Monitor *mons;

	Monitor *m = NULL;

	if(dir > 0) {
		if(!(m = selmon->next))
			m = mons;
	}
	else if(selmon == mons)
		for(m = mons; m->next; m = m->next);
	else
		for(m = mons; m->next != selmon; m = m->next);
	return m;
}

static void
drw_map(Drw *drw, Window win, int x, int y, unsigned int w, unsigned int h) {
	if(!drw)
		return;
	XCopyArea(drw->dpy, drw->drawable, win, drw->gc, x, y, w, h, x, y);
	XSync(drw->dpy, False);
}

void
drawbar(Monitor *m) {
	extern ClrScheme scheme[SchemeLast];
	extern const char *tags[];
	extern const size_t LENGTH_tags;
	extern int blw;
	extern int bh;
    extern Drw* drw;
    extern Monitor *selmon;
	extern char stext[256];

	int x, xx, w;
	unsigned int i, occ = 0, urg = 0;
	Client *c;

	for(c = m->clients; c; c = c->next) {
		occ |= c->tags;
		if(c->isurgent)
			urg |= c->tags;
	}
	x = 0;
	for(i = 0; i < LENGTH2(tags); i++) {
		w = TEXTW(tags[i]);
		drw_setscheme(drw, m->tagset[m->seltags] & 1 << i ? &scheme[SchemeSel] : &scheme[SchemeNorm]);
		drw_text(drw, x, 0, w, bh, tags[i], urg & 1 << i);
		drw_rect(drw, x, 0, w, bh, m == selmon && selmon->sel && selmon->sel->tags & 1 << i,
		           occ & 1 << i, urg & 1 << i);
		x += w;
	}
	w = blw = TEXTW(m->ltsymbol);
	drw_setscheme(drw, &scheme[SchemeNorm]);
	drw_text(drw, x, 0, w, bh, m->ltsymbol, 0);
	x += w;
	xx = x;
	if(m == selmon) { /* status is only drawn on selected monitor */
		w = TEXTW(stext);
		x = m->ww - w;
		if(x < xx) {
			x = xx;
			w = m->ww - xx;
		}
		drw_text(drw, x, 0, w, bh, stext, 0);
	}
	else
		x = m->ww;
	if((w = x - xx) > bh) {
		x = xx;
		if(m->sel) {
			drw_setscheme(drw, m == selmon ? &scheme[SchemeSel] : &scheme[SchemeNorm]);
			drw_text(drw, x, 0, w, bh, m->sel->name, 0);
			drw_rect(drw, x, 0, w, bh, m->sel->isfixed, m->sel->isfloating, 0);
		}
		else {
			drw_setscheme(drw, &scheme[SchemeNorm]);
			drw_text(drw, x, 0, w, bh, NULL, 0);
		}
	}
	drw_map(drw, m->barwin, 0, 0, m->ww, bh);
}

void
drawbars(void) {
	extern Monitor *mons;
	Monitor *m;

	for(m = mons; m; m = m->next)
		drawbar(m);
}


static void
clearurgent(Client *c, Display* dpy) {
	XWMHints *wmh;

	c->isurgent = False;
	if(!(wmh = XGetWMHints(dpy, c->win)))
		return;
	wmh->flags &= ~XUrgencyHint;
	XSetWMHints(dpy, c->win, wmh);
	XFree(wmh);
}

void
focus(Client *c, Display* dpy) {
	extern Window root;
	extern Atom netatom[NetLast];
	extern Monitor *selmon;
	extern ClrScheme scheme[SchemeLast];

	if(!c || !ISVISIBLE(c))
		for(c = selmon->stack; c && !ISVISIBLE(c); c = c->snext);
	/* was if(selmon->sel) */
	if(selmon->sel && selmon->sel != c)
		unfocus(selmon->sel, False, dpy);
	if(c) {
		if(c->mon != selmon)
			selmon = c->mon;
		if(c->isurgent)
			clearurgent(c, dpy);
		detachstack(c);
		attachstack(c);
		grabbuttons(c, True, dpy);
		XSetWindowBorder(dpy, c->win, scheme[SchemeSel].border->pix);
		setfocus(c, dpy);
	}
	else {
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
		XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
	}
	selmon->sel = c;
	drawbars();
}

void
focusmon(const Arg *arg, Display* dpy) {
	extern Monitor *mons;
	extern Monitor *selmon;

	
	Monitor *m;

	if(!mons->next)
		return;
	if((m = dirtomon(arg->i)) == selmon)
		return;
	unfocus(selmon->sel, False, dpy); /* s/True/False/ fixes input focus issues
					in gedit and anjuta */
	selmon = m;
	focus(NULL, dpy);
}

Bool
getrootptr(int *x, int *y, Display* dpy) {
	extern Window root;

	int di;
	unsigned int dui;
	Window dummy;

	return XQueryPointer(dpy, root, &dummy, &dummy, x, y, &di, &di, &dui);
}

long
getstate(Window w, Display* dpy) {
	extern Atom wmatom[WMLast];

	int format;
	long result = -1;
	unsigned char *p = NULL;
	unsigned long n, extra;
	Atom real;

	if(XGetWindowProperty(dpy, w, wmatom[WMState], 0L, 2L, False, wmatom[WMState],
	                      &real, &format, &n, &extra, (unsigned char **)&p) != Success)
		return -1;
	if(n != 0)
		result = *p;
	XFree(p);
	return result;
}


void
grabbuttons(Client *c, Bool focused, Display* dpy) {
	extern Button buttons[];
	extern const size_t LENGTH_buttons;
	extern unsigned int numlockmask;

	updatenumlockmask(dpy);
	{
		unsigned int i, j;
		unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
		XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
		if(focused) {
			for(i = 0; i < LENGTH2(buttons); i++)
				if(buttons[i].click == ClkClientWin)
					for(j = 0; j < LENGTH(modifiers); j++)
						XGrabButton(dpy, buttons[i].button,
						            buttons[i].mask | modifiers[j],
						            c->win, False, BUTTONMASK,
						            GrabModeAsync, GrabModeSync, None, None);
		}
		else
			XGrabButton(dpy, AnyButton, AnyModifier, c->win, False,
			            BUTTONMASK, GrabModeAsync, GrabModeSync, None, None);
	}
}

void
killclient(const Arg *argi, Display* dpy) {
	extern Monitor *selmon;
	extern Atom wmatom[WMLast];


	XErrorHandlerT xeh;

	if(!selmon->sel)
		return;
	if(!sendevent(selmon->sel, wmatom[WMDelete], dpy)) {
		XGrabServer(dpy);
		xeh = XSetErrorHandler(xerrordummy);
		XSetCloseDownMode(dpy, DestroyAll);
		XKillClient(dpy, selmon->sel->win);
		XSync(dpy, False);
		XSetErrorHandler(xeh);
		XUngrabServer(dpy);
	}
}

// single caller
static void
applyrules(Client *c, Display* dpy) {
	extern const Rule rules[];
	extern const size_t LENGTH_rules;
	extern const char broken[];
	extern Monitor *mons;
	extern const size_t LENGTH_tags;
	
	const char *type, *instance;
	unsigned int i;
	const Rule *r;
	Monitor *m;
	XClassHint ch = { NULL, NULL };

	/* rule matching */
	c->isfloating = c->tags = 0;
	XGetClassHint(dpy, c->win, &ch);
	type    = ch.res_class ? ch.res_class : broken;
	instance = ch.res_name  ? ch.res_name  : broken;

	for(i = 0; i < LENGTH2(rules); i++) {
		r = &rules[i];
		if((!r->title || strstr(c->name, r->title))
		&& (!r->type || strstr(type, r->type))
		&& (!r->instance || strstr(instance, r->instance)))
		{
			c->isfloating = r->isfloating;
			c->tags |= r->tags;
			for(m = mons; m && m->num != r->monitor; m = m->next);
			if(m)
				c->mon = m;
		}
	}
	if(ch.res_class)
		XFree(ch.res_class);
	if(ch.res_name)
		XFree(ch.res_name);
	c->tags = c->tags & TAGMASK ? c->tags & TAGMASK : c->mon->tagset[c->mon->seltags];
}

void
manage(Window w, XWindowAttributes *wa, Display* dpy) {
	extern Window root;
	extern Monitor *selmon;
	extern int bh;
	extern int sw;
	extern ClrScheme scheme[SchemeLast];

	extern const unsigned int borderpx;
	extern Atom netatom[NetLast];
	Client *c, *t = NULL;
	Window trans = None;
	XWindowChanges wc;

	if(!(c = calloc(1, sizeof(Client))))
		die("fatal: could not malloc() %u bytes\n", sizeof(Client));
	c->win = w;
	updatetitle(c, dpy);
	if(XGetTransientForHint(dpy, w, &trans) && (t = wintoclient(trans))) {
		c->mon = t->mon;
		c->tags = t->tags;
	}
	else {
		c->mon = selmon;
		applyrules(c, dpy);
	}
	/* geometry */
	c->x = c->oldx = wa->x;
	c->y = c->oldy = wa->y;
	c->w = c->oldw = wa->width;
	c->h = c->oldh = wa->height;
	c->oldbw = wa->border_width;

	if(c->x + WIDTH(c) > c->mon->mx + c->mon->mw)
		c->x = c->mon->mx + c->mon->mw - WIDTH(c);
	if(c->y + HEIGHT(c) > c->mon->my + c->mon->mh)
		c->y = c->mon->my + c->mon->mh - HEIGHT(c);
	c->x = MAX(c->x, c->mon->mx);
	/* only fix client y-offset, if the client center might cover the bar */
	c->y = MAX(c->y, ((c->mon->by == c->mon->my) && (c->x + (c->w / 2) >= c->mon->wx)
	           && (c->x + (c->w / 2) < c->mon->wx + c->mon->ww)) ? bh : c->mon->my);
	c->bw = borderpx;

	wc.border_width = c->bw;
	XConfigureWindow(dpy, w, CWBorderWidth, &wc);
	XSetWindowBorder(dpy, w, scheme[SchemeNorm].border->pix);
	configure(c, dpy); /* propagates border_width, if size doesn't change */
	updatewindowtype(c, dpy);
	updatesizehints(c, dpy);
	updatewmhints(c, dpy);
	XSelectInput(dpy, w, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);
	grabbuttons(c, False, dpy);
	if(!c->isfloating)
		c->isfloating = c->oldstate = trans != None || c->isfixed;
	if(c->isfloating)
		XRaiseWindow(dpy, c->win);
	attach(c);
	attachstack(c);
	XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend,
	                (unsigned char *) &(c->win), 1);
	XMoveResizeWindow(dpy, c->win, c->x + 2 * sw, c->y, c->w, c->h); /* some windows require this */
	setclientstate(c, NormalState, dpy);
	if (c->mon == selmon)
		unfocus(selmon->sel, False, dpy);
	c->mon->sel = c;
	arrange(c->mon, dpy);
	XMapWindow(dpy, c->win);
	focus(NULL, dpy);
}

Client *
nexttiled(Client *c) {
	for(; c && (c->isfloating || !ISVISIBLE(c)); c = c->next);
	return c;
}

void
pop(Client *c, Display* dpy) {
	detach(c);
	attach(c);
	focus(c, dpy);
	arrange(c->mon, dpy);
}

Monitor *
recttomon(int x, int y, int w, int h) {
	extern Monitor *mons;
	extern Monitor *selmon;

	
	Monitor *m, *r = selmon;
	int a, area = 0;

	for(m = mons; m; m = m->next)
		if((a = INTERSECT(x, y, w, h, m)) > area) {
			area = a;
			r = m;
		}
	return r;
}

static Bool
applysizehints(Client *c, int *x, int *y, int *w, int *h, Bool interact) {
	extern const Bool resizehints; 
	extern int bh;
	extern int sh;
	extern int sw;

	Bool baseismin;
	Monitor *m = c->mon;

	/* set minimum possible */
	*w = MAX(1, *w);
	*h = MAX(1, *h);
	if(interact) {
		if(*x > sw)
			*x = sw - WIDTH(c);
		if(*y > sh)
			*y = sh - HEIGHT(c);
		if(*x + *w + 2 * c->bw < 0)
			*x = 0;
		if(*y + *h + 2 * c->bw < 0)
			*y = 0;
	}
	else {
		if(*x >= m->wx + m->ww)
			*x = m->wx + m->ww - WIDTH(c);
		if(*y >= m->wy + m->wh)
			*y = m->wy + m->wh - HEIGHT(c);
		if(*x + *w + 2 * c->bw <= m->wx)
			*x = m->wx;
		if(*y + *h + 2 * c->bw <= m->wy)
			*y = m->wy;
	}
	if(*h < bh)
		*h = bh;
	if(*w < bh)
		*w = bh;
	if(resizehints || c->isfloating || !c->mon->lt[c->mon->sellt]->arrange) {
		/* see last two sentences in ICCCM 4.1.2.3 */
		baseismin = c->basew == c->minw && c->baseh == c->minh;
		if(!baseismin) { /* temporarily remove base dimensions */
			*w -= c->basew;
			*h -= c->baseh;
		}
		/* adjust for aspect limits */
		if(c->mina > 0 && c->maxa > 0) {
			if(c->maxa < (float)*w / *h)
				*w = *h * c->maxa + 0.5;
			else if(c->mina < (float)*h / *w)
				*h = *w * c->mina + 0.5;
		}
		if(baseismin) { /* increment calculation requires this */
			*w -= c->basew;
			*h -= c->baseh;
		}
		/* adjust for increment value */
		if(c->incw)
			*w -= *w % c->incw;
		if(c->inch)
			*h -= *h % c->inch;
		/* restore base dimensions */
		*w = MAX(*w + c->basew, c->minw);
		*h = MAX(*h + c->baseh, c->minh);
		if(c->maxw)
			*w = MIN(*w, c->maxw);
		if(c->maxh)
			*h = MIN(*h, c->maxh);
	}
	return *x != c->x || *y != c->y || *w != c->w || *h != c->h;
}

void
resize(Client *c, int x, int y, int w, int h, Bool interact, Display* dpy) {
	if(applysizehints(c, &x, &y, &w, &h, interact))
		resizeclient(c, x, y, w, h, dpy);
}

void
resizeclient(Client *c, int x, int y, int w, int h, Display* dpy) {
	XWindowChanges wc;

	c->oldx = c->x; c->x = wc.x = x;
	c->oldy = c->y; c->y = wc.y = y;
	c->oldw = c->w; c->w = wc.width = w;
	c->oldh = c->h; c->h = wc.height = h;
	wc.border_width = c->bw;
	XConfigureWindow(dpy, c->win, CWX|CWY|CWWidth|CWHeight|CWBorderWidth, &wc);
	configure(c, dpy);
	XSync(dpy, False);
}

void
restack(Monitor *m, Display* dpy) {
	Client *c;
	XEvent ev;
	XWindowChanges wc;

	drawbar(m);
	if(!m->sel)
		return;
	if(m->sel->isfloating || !m->lt[m->sellt]->arrange)
		XRaiseWindow(dpy, m->sel->win);
	if(m->lt[m->sellt]->arrange) {
		wc.stack_mode = Below;
		wc.sibling = m->barwin;
		for(c = m->stack; c; c = c->snext)
			if(!c->isfloating && ISVISIBLE(c)) {
				XConfigureWindow(dpy, c->win, CWSibling|CWStackMode, &wc);
				wc.sibling = c->win;
			}
	}
	XSync(dpy, False);
	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void
scan(Display* dpy) {
	extern Window root;

	unsigned int i, num;
	Window d1, d2, *wins = NULL;
	XWindowAttributes wa;

	if(XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
		for(i = 0; i < num; i++) {
			if(!XGetWindowAttributes(dpy, wins[i], &wa)
			|| wa.override_redirect || XGetTransientForHint(dpy, wins[i], &d1))
				continue;
			if(wa.map_state == IsViewable || getstate(wins[i], dpy) == IconicState)
				manage(wins[i], &wa, dpy);
		}
		for(i = 0; i < num; i++) { /* now the transients */
			if(!XGetWindowAttributes(dpy, wins[i], &wa))
				continue;
			if(XGetTransientForHint(dpy, wins[i], &d1)
			&& (wa.map_state == IsViewable || getstate(wins[i], dpy) == IconicState))
				manage(wins[i], &wa, dpy);
		}
		if(wins)
			XFree(wins);
	}
}

void
sendmon(Client *c, Monitor *m, Display* dpy) {
	if(c->mon == m)
		return;
	unfocus(c, True, dpy);
	detach(c);
	detachstack(c);
	c->mon = m;
	c->tags = m->tagset[m->seltags]; /* assign tags of target monitor */
	attach(c);
	attachstack(c);
	focus(NULL, dpy);
	arrange(NULL, dpy);
}

void
setclientstate(Client *c, long state, Display* dpy) {
	extern Atom wmatom[WMLast];

	long data[] = { state, None };

	XChangeProperty(dpy, c->win, wmatom[WMState], wmatom[WMState], 32,
			PropModeReplace, (unsigned char *)data, 2);
}

Bool
sendevent(Client *c, Atom proto, Display* dpy) {
	extern Atom wmatom[WMLast];

	int n;
	Atom *protocols;
	Bool exists = False;
	XEvent ev;

	if(XGetWMProtocols(dpy, c->win, &protocols, &n)) {
		while(!exists && n--)
			exists = protocols[n] == proto;
		XFree(protocols);
	}
	if(exists) {
		ev.type = ClientMessage;
		ev.xclient.window = c->win;
		ev.xclient.message_type = wmatom[WMProtocols];
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = proto;
		ev.xclient.data.l[1] = CurrentTime;
		XSendEvent(dpy, c->win, False, NoEventMask, &ev);
	}
	return exists;
}

void
setfocus(Client *c, Display* dpy) {
	extern Window root;
	extern Atom wmatom[WMLast];
	extern Atom netatom[NetLast];

	if(!c->neverfocus) {
		XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
		XChangeProperty(dpy, root, netatom[NetActiveWindow],
 		                XA_WINDOW, 32, PropModeReplace,
 		                (unsigned char *) &(c->win), 1);
	}
	sendevent(c, wmatom[WMTakeFocus], dpy);
}

void
setfullscreen(Client *c, Bool fullscreen, Display* dpy) {
	extern Atom netatom[NetLast];

	if(fullscreen) {
		XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
		                PropModeReplace, (unsigned char*)&netatom[NetWMFullscreen], 1);
		c->isfullscreen = True;
		c->oldstate = c->isfloating;
		c->oldbw = c->bw;
		c->bw = 0;
		c->isfloating = True;
		resizeclient(c, c->mon->mx, c->mon->my, c->mon->mw, c->mon->mh, dpy);
		XRaiseWindow(dpy, c->win);
	}
	else {
		XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
		                PropModeReplace, (unsigned char*)0, 0);
		c->isfullscreen = False;
		c->isfloating = c->oldstate;
		c->bw = c->oldbw;
		c->x = c->oldx;
		c->y = c->oldy;
		c->w = c->oldw;
		c->h = c->oldh;
		resizeclient(c, c->x, c->y, c->w, c->h, dpy);
		arrange(c->mon, dpy);
	}
}


void
showhide(Client *c, Display* dpy) {
	if(!c)
		return;
	if(ISVISIBLE(c)) { /* show clients top down */
		XMoveWindow(dpy, c->win, c->x, c->y);
		if((!c->mon->lt[c->mon->sellt]->arrange || c->isfloating) && !c->isfullscreen)
			resize(c, c->x, c->y, c->w, c->h, False, dpy);
		showhide(c->snext, dpy);
	}
	else { /* hide clients bottom up */
		showhide(c->snext, dpy);
		XMoveWindow(dpy, c->win, WIDTH(c) * -2, c->y);
	}
}

void
tagmon(const Arg *arg, Display* dpy) {
	extern Monitor *mons;
	extern Monitor *selmon;

	
	if(!selmon->sel || !mons->next)
		return;
	sendmon(selmon->sel, dirtomon(arg->i), dpy);
}

void
unfocus(Client *c, Bool setfocus, Display* dpy) {
	extern ClrScheme scheme[SchemeLast];
	extern Atom netatom[NetLast];
    extern Window root;

	if(!c)
		return;
	grabbuttons(c, False, dpy);
	XSetWindowBorder(dpy, c->win, scheme[SchemeNorm].border->pix);
	if(setfocus) {
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
		XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
	}
}

void
unmanage(Client *c, Bool destroyed, Display* dpy) {
	Monitor *m = c->mon;
	XWindowChanges wc;
	XErrorHandlerT xeh;

	/* The server grab construct avoids race conditions. */
	detach(c);
	detachstack(c);
	if(!destroyed) {
		wc.border_width = c->oldbw;
		XGrabServer(dpy);
		xeh = XSetErrorHandler(xerrordummy);
		XConfigureWindow(dpy, c->win, CWBorderWidth, &wc); /* restore border */
		XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
		setclientstate(c, WithdrawnState, dpy);
		XSync(dpy, False);
		XSetErrorHandler(xeh);
		XUngrabServer(dpy);
	}
	free(c);
	focus(NULL, dpy);
	updateclientlist(dpy);
	arrange(m, dpy);
}

Client *
wintoclient(Window w) {
	extern Monitor *mons;
	
	Client *c;
	Monitor *m;

	for(m = mons; m; m = m->next)
		for(c = m->clients; c; c = c->next)
			if(c->win == w)
				return c;
	return NULL;
}

Monitor *
wintomon(Window w, Display* dpy) {
	extern Monitor *mons;
	extern Monitor *selmon;
    extern Window root;
	
	int x, y;
	Client *c;
	Monitor *m;

	if(w == root && getrootptr(&x, &y, dpy))
		return recttomon(x, y, 1, 1);
	for(m = mons; m; m = m->next)
		if(w == m->barwin)
			return m;
	if((c = wintoclient(w)))
		return c->mon;
	return selmon;
}

int
xerrordummy(Display *dpy, XErrorEvent *ee) {
	return 0;
}

