#include <X11/Xlib.h>

#include "types.h"
#include "drw_types.h"
#include "fwd.h"
#include "spawn.h"
#include "handlers.h"

#define BUTTONMASK              (ButtonPressMask|ButtonReleaseMask)
#define MOUSEMASK               (BUTTONMASK|PointerMotionMask)
#define WIDTH(X)                ((X)->w + 2 * (X)->bw)
#define HEIGHT(X)               ((X)->h + 2 * (X)->bw)
static void
movemouse(const Arg *arg, Display* dpy) {
	extern Cur *cursor[CurLast];
	extern HandlerT handler[LASTEvent];
	extern const unsigned int snap;       
	extern Monitor *selmon;
	extern Window root;

	int ocx, ocy;
	int x, y, nx, ny;
	Client *c;
	Monitor *m;
	XEvent ev;
	Time lasttime = 0;

	if(!(c = selmon->sel))
		return;
	if(c->isfullscreen) /* no support for fullscreen windows */
		return;
	restack(selmon, dpy);
	ocx = c->x;
	ocy = c->y;
	if(XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
				None, cursor[CurMove]->cursor, CurrentTime) != GrabSuccess)
		return;
	if(!getrootptr(&x, &y, dpy))
		return;
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

				nx = ocx + (ev.xmotion.x - x);
				ny = ocy + (ev.xmotion.y - y);
				if(nx >= selmon->wx && nx <= selmon->wx + selmon->ww
						&& ny >= selmon->wy && ny <= selmon->wy + selmon->wh) {
					if(abs(selmon->wx - nx) < snap)
						nx = selmon->wx;
					else if(abs((selmon->wx + selmon->ww) - (nx + WIDTH(c))) < snap)
						nx = selmon->wx + selmon->ww - WIDTH(c);
					if(abs(selmon->wy - ny) < snap)
						ny = selmon->wy;
					else if(abs((selmon->wy + selmon->wh) - (ny + HEIGHT(c))) < snap)
						ny = selmon->wy + selmon->wh - HEIGHT(c);
					if(!c->isfloating && selmon->lt[selmon->sellt]->arrange
							&& (abs(nx - c->x) > snap || abs(ny - c->y) > snap))
						togglefloating(NULL, dpy);
				}
				if(!selmon->lt[selmon->sellt]->arrange || c->isfloating)
					resize(c, nx, ny, c->w, c->h, True, dpy);
				break;
		}
	} while(ev.type != ButtonRelease);
	XUngrabPointer(dpy, CurrentTime);
	if((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
		sendmon(c, m, dpy);
		selmon = m;
		focus(NULL, dpy);
	}
}

#define MAX(A, B)               ((A) > (B) ? (A) : (B))
static void
resizemouse(const Arg *arg, Display* dpy) {
	extern Cur *cursor[CurLast];
	extern HandlerT handler[LASTEvent];
	extern const unsigned int snap;       
	extern Monitor *selmon;
	extern Window root;

	int ocx, ocy;
	int nw, nh;
	Client *c;
	Monitor *m;
	XEvent ev;
	Time lasttime = 0;

	if(!(c = selmon->sel))
		return;
	if(c->isfullscreen) /* no support for fullscreen windows */
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

extern const Layout layouts[];
extern const char* termcmd[];
#define MODKEY Mod4Mask

/* button definitions */
/* click can be ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};
#define LENGTH(X)               (sizeof X / sizeof X[0])
const size_t LENGTH_buttons = LENGTH(buttons);

