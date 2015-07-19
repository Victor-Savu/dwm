#include <X11/cursorfont.h>

#include "types.h"
#include "drw_types.h"
#include "fwd.h"

static Cur *
drw_cur_create(Drw *drw, int shape) {
	Cur *cur = (Cur *)calloc(1, sizeof(Cur));

	if(!drw || !cur)
		return NULL;
	cur->cursor = XCreateFontCursor(drw->dpy, shape);
	return cur;
}

void die(const char *errstr, ...);

static Clr *
drw_clr_create(Drw *drw, const char *clrname) {
	Clr *clr;
	Colormap cmap;
	Visual *vis;

	if(!drw)
		return NULL;
	clr = (Clr *)calloc(1, sizeof(Clr));
	if(!clr)
		return NULL;
	cmap = DefaultColormap(drw->dpy, drw->screen);
	vis = DefaultVisual(drw->dpy, drw->screen);
	if(!XftColorAllocName(drw->dpy, vis, cmap, clrname, &clr->rgb))
		die("error, cannot allocate color '%s'\n", clrname);
	clr->pix = clr->rgb.pixel;
	return clr;
}

static Drw *
drw_create(Display *dpy, int screen, Window root, unsigned int w, unsigned int h) {
	Drw *drw = (Drw *)calloc(1, sizeof(Drw));
	if(!drw)
		return NULL;
	drw->dpy = dpy;
	drw->screen = screen;
	drw->root = root;
	drw->w = w;
	drw->h = h;
	drw->drawable = XCreatePixmap(dpy, root, w, h, DefaultDepth(dpy, screen));
	drw->gc = XCreateGC(dpy, root, 0, NULL);
	drw->fontcount = 0;
	XSetLineAttributes(dpy, drw->gc, 1, LineSolid, CapButt, JoinMiter);
	return drw;
}

static void
drw_load_fonts(Drw* drw, const char *fonts[], size_t fontcount) {
	size_t i;
	Fnt *font;
	for (i = 0; i < fontcount; i++) {
		if (drw->fontcount >= DRW_FONT_CACHE_SIZE) {
			die("Font cache exhausted.\n");
		} else if ((font = drw_font_create(drw, fonts[i]))) {
			drw->fonts[drw->fontcount++] = font;
		}
	}
}


#define LENGTH(X)               (sizeof X / sizeof X[0])

void die(const char *errstr, ...);

// globals
extern const char *fonts[1]; // config_fonts.c


// private
static void sigchld(int unused);

extern int sw;
extern int sh;
extern Window root;
#include "drw.h"
extern Drw *drw;
extern int bh;
extern Atom wmatom[WMLast];
extern Atom netatom[NetLast];
extern Cur *cursor[CurLast];
extern ClrScheme scheme[SchemeLast];
#include <X11/Xatom.h>
extern const char normbordercolor[];
extern const char normbgcolor[];
extern const char normfgcolor[];
extern const char selbordercolor[];
extern const char selbgcolor[];
extern const char selfgcolor[];
void
setup(Display* dpy) {
	XSetWindowAttributes wa;
	int screen;

	/* clean up any zombies immediately */
	sigchld(0);

	/* init screen */
	screen = DefaultScreen(dpy);
	sw = DisplayWidth(dpy, screen);
	sh = DisplayHeight(dpy, screen);
	root = RootWindow(dpy, screen);
	drw = drw_create(dpy, screen, root, sw, sh);
	drw_load_fonts(drw, fonts, LENGTH(fonts));
	if (!drw->fontcount)
		die("No fonts could be loaded.\n");
	bh = drw->fonts[0]->h + 2;
	updategeom(dpy);
	/* init atoms */
	wmatom[WMProtocols] = XInternAtom(dpy, "WM_PROTOCOLS", False);
	wmatom[WMDelete] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	wmatom[WMState] = XInternAtom(dpy, "WM_STATE", False);
	wmatom[WMTakeFocus] = XInternAtom(dpy, "WM_TAKE_FOCUS", False);
	netatom[NetActiveWindow] = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
	netatom[NetSupported] = XInternAtom(dpy, "_NET_SUPPORTED", False);
	netatom[NetWMName] = XInternAtom(dpy, "_NET_WM_NAME", False);
	netatom[NetWMState] = XInternAtom(dpy, "_NET_WM_STATE", False);
	netatom[NetWMFullscreen] = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
	netatom[NetWMWindowType] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
	netatom[NetWMWindowTypeDialog] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);
	netatom[NetClientList] = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
	/* init cursors */
	cursor[CurNormal] = drw_cur_create(drw, XC_left_ptr);
	cursor[CurResize] = drw_cur_create(drw, XC_sizing);
	cursor[CurMove] = drw_cur_create(drw, XC_fleur);
	/* init appearance */
	scheme[SchemeNorm].border = drw_clr_create(drw, normbordercolor);
	scheme[SchemeNorm].bg = drw_clr_create(drw, normbgcolor);
	scheme[SchemeNorm].fg = drw_clr_create(drw, normfgcolor);
	scheme[SchemeSel].border = drw_clr_create(drw, selbordercolor);
	scheme[SchemeSel].bg = drw_clr_create(drw, selbgcolor);
	scheme[SchemeSel].fg = drw_clr_create(drw, selfgcolor);
	/* init bars */
	updatebars(dpy);
	updatestatus(dpy);
	/* EWMH support per view */
	XChangeProperty(dpy, root, netatom[NetSupported], XA_ATOM, 32,
			PropModeReplace, (unsigned char *) netatom, NetLast);
	XDeleteProperty(dpy, root, netatom[NetClientList]);
	/* select for events */
	wa.cursor = cursor[CurNormal]->cursor;
	wa.event_mask = SubstructureRedirectMask|SubstructureNotifyMask|ButtonPressMask|PointerMotionMask
	                |EnterWindowMask|LeaveWindowMask|StructureNotifyMask|PropertyChangeMask;
	XChangeWindowAttributes(dpy, root, CWEventMask|CWCursor, &wa);
	XSelectInput(dpy, root, wa.event_mask);
	grabkeys(dpy);
	focus(NULL, dpy);
}

#include <signal.h>
#include <wait.h>
void
sigchld(int unused) {
	if(signal(SIGCHLD, sigchld) == SIG_ERR)
		die("Can't install SIGCHLD handler");
	while(0 < waitpid(-1, NULL, WNOHANG));
}

