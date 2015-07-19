#ifndef HANDLERS_H
#define HANDLERS_H

#include "types.h"

void setlayout(const Arg *arg, Display* dpy);
void tag(const Arg *arg, Display* dpy);
void togglefloating(const Arg *arg, Display* dpy);
void toggletag(const Arg *arg, Display* dpy);
void toggleview(const Arg *arg, Display* dpy);
void view(const Arg *arg, Display* dpy);
void zoom(const Arg *arg, Display* dpy);

#endif
