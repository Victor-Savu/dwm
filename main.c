#include <string.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>

#include <X11/Xlib.h>

void die(const char *errstr, ...); // die.c
void checkotherwm(void); // xerrors.c
void setup(void); // setup.c
void scan(void);
void run(void);
void cleanup(void);

extern Display *dpy;

int
main(int argc, char *argv[]) {
	if(argc == 2 && !strcmp("-v", argv[1]))
		die("dwm-"VERSION", © 2006-2014 dwm engineers, see LICENSE for details\n");
	else if(argc != 1)
		die("usage: dwm [-v]\n");
	if(!setlocale(LC_CTYPE, "") || !XSupportsLocale())
		fputs("warning: no locale support\n", stderr);
	if(!(dpy = XOpenDisplay(NULL)))
		die("dwm: cannot open display\n");
	checkotherwm();
	setup();
	scan();
	run();
	cleanup();
	XCloseDisplay(dpy);
	return EXIT_SUCCESS;
}
