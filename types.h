#ifndef TYPES_H
#define TYPES_H

#include <X11/Xlib.h>

typedef int(*XErrorHandlerT)(Display*, XErrorEvent*);
typedef void(*HandlerT)(XEvent *, Display *);

/* enums */
enum { CurNormal, CurResize, CurMove, CurLast }; /* cursor */
enum { SchemeNorm, SchemeSel, SchemeLast }; /* color schemes */
enum { NetSupported, NetWMName, NetWMState,
       NetWMFullscreen, NetActiveWindow, NetWMWindowType,
       NetWMWindowTypeDialog, NetClientList, NetLast }; /* EWMH atoms */
enum { WMProtocols, WMDelete, WMState, WMTakeFocus, WMLast }; /* default atoms */
enum { ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle,
       ClkClientWin, ClkRootWin, ClkLast }; /* clicks */

typedef union {
	int i;
	unsigned int ui;
	float f;
	const void *v;
} Arg;
typedef void (*Function)(const Arg *, Display *);

typedef struct {
	unsigned int click;
	unsigned int mask;
	unsigned int button;
	Function func;
	const Arg arg;
} Button;

typedef struct Monitor Monitor;
typedef struct Client Client;
struct Client {
	char name[256];
	float mina, maxa;
	int x, y, w, h;
	int oldx, oldy, oldw, oldh;
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int bw, oldbw;
	unsigned int tags;
	Bool isfixed, isfloating, isurgent, neverfocus, oldstate, isfullscreen;
	Client *next;
	Client *snext;
	Monitor *mon;
	Window win;
};

typedef struct Layout Layout;
struct Monitor {
	char ltsymbol[16];
	float mfact;
	int nmaster;
	int num;
	int by;               /* bar geometry */
	int mx, my, mw, mh;   /* screen size */
	int wx, wy, ww, wh;   /* window area  */
	unsigned int seltags;
	unsigned int sellt;
	unsigned int tagset[2];
	Bool showbar;
	Bool topbar;
	Client *clients;
	Client *sel;
	Client *stack;
	Monitor *next;
	Window barwin;
	const Layout *lt[2];
};


typedef struct {
	unsigned int mod;
	KeySym keysym;
	Function func;
	const Arg arg;
} Key;

struct Layout{
	const char *symbol;
	void (*arrange)(Monitor *, Display*);
};

typedef struct {
	const char *type;
	const char *instance;
	const char *title;
	unsigned int tags;
	Bool isfloating;
	int monitor;
} Rule;

#endif
