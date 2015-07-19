#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/Xlib.h>

#include "types.h"

void
spawn(const Arg *arg, Display* dpy) {
    extern const char *dmenucmd[];
    extern char dmenumon[2]; 
    extern Monitor *selmon;
	if(arg->v == dmenucmd)
		dmenumon[0] = '0' + selmon->num;
	if(fork() == 0) {
		if(dpy)
			close(ConnectionNumber(dpy));
		setsid();
		execvp(((char **)arg->v)[0], (char **)arg->v);
		fprintf(stderr, "dwm: execvp %s", ((char **)arg->v)[0]);
		perror(" failed");
		exit(EXIT_SUCCESS);
	}
}


