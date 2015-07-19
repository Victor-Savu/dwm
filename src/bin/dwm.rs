extern crate dwm;

fn main() {
/*
	if(argc == 2 && !strcmp("-v", argv[1]))
		die("dwm-"VERSION", © 2006-2014 dwm engineers, see LICENSE for details\n");
	else if(argc != 1)
		die("usage: dwm [-v]\n");
*/
    let dpy = dwm::display::Display::new();
	dpy.checkotherwm();
    dpy.setup();
    dpy.scan();
    dpy.run();
}
