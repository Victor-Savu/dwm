#include <string.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>

#include <X11/Xlib.h>

void die(const char *errstr, ...);
void checkotherwm(Display* dpy);
void setup(Display* dpy);
void scan(Display* dpy);
void run(Display* dpy);
void cleanup(Display* dpy);

int
main(int argc, char *argv[]) {
        Display *dpy;

	if(argc == 2 && !strcmp("-v", argv[1]))
		die("dwm-"VERSION", © 2006-2014 dwm engineers, see LICENSE for details\n");
	else if(argc != 1)
		die("usage: dwm [-v]\n");
	if(!setlocale(LC_CTYPE, "") || !XSupportsLocale())
		fputs("warning: no locale support\n", stderr);
	if(!(dpy = XOpenDisplay(NULL)))
		die("dwm: cannot open display\n");
	checkotherwm(dpy);
	setup(dpy);
	scan(dpy);
	run(dpy);
	cleanup(dpy);
	XCloseDisplay(dpy);
	return EXIT_SUCCESS;
}
