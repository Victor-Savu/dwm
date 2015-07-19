#ifndef FWD_H
#define FWD_H
#include "types.h"

/* function declarations */
void arrange(Monitor *m, Display* dpy);
void attach(Client *c);
void attachstack(Client *c);
void buttonpress(XEvent *e, Display* dpy);
void cleanupmon(Monitor *mon, Display* dpy);
void configure(Client *c, Display* dpy);
void detach(Client *c);
void detachstack(Client *c);
Monitor *dirtomon(int dir);
void drawbar(Monitor *m);
void drawbars(void);
void focus(Client *c, Display* dpy);
void focusmon(const Arg *arg, Display* dpy);
Bool getrootptr(int *x, int *y, Display* dpy);
long getstate(Window w, Display* dpy);
void grabbuttons(Client *c, Bool focused, Display* dpy);
void grabkeys(Display* dpy);
void keypress(XEvent *e, Display* dpy);
void killclient(const Arg *arg, Display* dpy);
void manage(Window w, XWindowAttributes *wa, Display* dpy);
Client *nexttiled(Client *c);
void pop(Client *, Display* dpy);
Monitor *recttomon(int x, int y, int w, int h);
void resize(Client *c, int x, int y, int w, int h, Bool interact, Display* dpy);
void resizeclient(Client *c, int x, int y, int w, int h, Display* dpy);
void restack(Monitor *m, Display* dpy);
Bool sendevent(Client *c, Atom proto, Display* dpy);
void sendmon(Client *c, Monitor *m, Display* dpy);
void setclientstate(Client *c, long state, Display* dpy);
void setfocus(Client *c, Display* dpy);
void setfullscreen(Client *c, Bool fullscreen, Display* dpy);
void showhide(Client *c, Display* dpy);
void tagmon(const Arg *arg, Display* dpy);
void unfocus(Client *c, Bool setfocus, Display* dpy);
void unmanage(Client *c, Bool destroyed, Display* dpy);
Bool updategeom(Display* dpy);
void updatebarpos(Monitor *m);
void updatebars(Display* dpy);
void updateclientlist(Display* dpy);
void updatenumlockmask(Display* dpy);
void updatesizehints(Client *c, Display* dpy);
void updatestatus(Display* dpy);
void updatewindowtype(Client *c, Display* dpy);
void updatetitle(Client *c, Display* dpy);
void updatewmhints(Client *c, Display* dpy);
Client *wintoclient(Window w);
Monitor *wintomon(Window w, Display* dpy);
int xerrordummy(Display *dpy, XErrorEvent *ee);

#endif
