#include <signal.h>
#include <wait.h>
#include <X11/cursorfont.h>

#include "types.h"
#include "globals.h"

#define LENGTH(X)               (sizeof X / sizeof X[0])

#include "die.h"
#include "config.h"

// globals
extern const char *fonts[1]; // config_fonts.c

// using
Bool updategeom(void);
void updatebars(void);
void updatestatus(void);
void grabkeys(void);
void focus(Client*);

// private
static void sigchld(int unused);

void
setup(void) {
	XSetWindowAttributes wa;

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
	updategeom();
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
	updatebars();
	updatestatus();
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
	grabkeys();
	focus(NULL);
}

void
sigchld(int unused) {
	if(signal(SIGCHLD, sigchld) == SIG_ERR)
		die("Can't install SIGCHLD handler");
	while(0 < waitpid(-1, NULL, WNOHANG));
}

