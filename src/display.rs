use std::ptr::{ null, null_mut };

use x11_dl::xlib;
use libc;

extern {
    fn setup(dpy: *mut xlib::Display);
    fn scan(dpy: *mut xlib::Display);
    fn run(dpy: *mut xlib::Display);
    fn cleanup(dpy: *mut xlib::Display);
}

pub struct Display {
    dpy: *mut xlib::Display,
}

type XlibError = unsafe extern "C" fn(dpy: *mut xlib::Display, ee: *mut xlib::XErrorEvent) -> libc::c_int;

static mut xerrorxlib: Option<XlibError> = Some(xerrorstart);
 
unsafe extern "C" fn xerrorstart(_: *mut xlib::Display, _: *mut xlib::XErrorEvent) -> libc::c_int {
	panic!("dwm: another window manager is already running");
}

unsafe extern "C" fn xerror(dpy: *mut xlib::Display, ee: *mut xlib::XErrorEvent) -> libc::c_int {
/* For now, thie rust version will be a bit more strict when it comes to errors becaue xlib does
 * not expose the constants which are here checked agains ee.request_code

    if ee.error_code == xlib::BadWindow
	|| (ee.request_code == xlib::X_SetInputFocus && ee.error_code == xlib::BadMatch)
	|| (ee.request_code == xlib::X_PolyText8 && ee.error_code == xlib::BadDrawable)
	|| (ee.request_code == xlib::X_PolyFillRectangle && ee.error_code == xlib::BadDrawable)
	|| (ee.request_code == xlib::X_PolySegment && ee.error_code == xlib::BadDrawable)
	|| (ee.request_code == xlib::X_ConfigureWindow && ee.error_code == xlib::BadMatch)
	|| (ee.request_code == xlib::X_GrabButton && ee.error_code == xlib::BadAccess)
	|| (ee.request_code == xlib::X_GrabKey && ee.error_code == xlib::BadAccess)
	|| (ee.request_code == xlib::X_CopyArea && ee.error_code == xlib::BadDrawable) {
		return 0;
    }
*/
	println!("dwm: fatal error: request code={}, error code={}",
			(*ee).request_code, (*ee).error_code);

	xerrorxlib.clone().map_or( 0, |xer| (xer)(dpy, ee) ) /* may call exit */
}

impl Display {
    pub fn new() -> Self {
        let xlib = xlib::Xlib::open().unwrap();
        unsafe {
            if (xlib.XSupportsLocale)() == 0 {
                println!("warning: no locale support");
            }
        };

        let dpy = unsafe { (xlib.XOpenDisplay)(null()) };

        if dpy == null_mut() {
            panic!("dwm: cannot open display");
        }
        Display { dpy: dpy }
    }

    pub fn checkotherwm(&self) {
        let xlib = xlib::Xlib::open().unwrap();
        unsafe {
            xerrorxlib = (xlib.XSetErrorHandler)(Some(xerrorstart));
            /* this causes an error if some other window manager is running */
            (xlib.XSelectInput)(self.dpy, (xlib.XDefaultRootWindow)(self.dpy), xlib::SubstructureRedirectMask);
            (xlib.XSync)(self.dpy, 0);
            (xlib.XSetErrorHandler)(Some(xerror));
            
            (xlib.XSync)(self.dpy, 0);
        }
    }
    pub fn setup(&self) {
        unsafe { setup(self.dpy) }
    }
    pub fn scan(&self) {
        unsafe { scan(self.dpy) }
    }
    pub fn run(&self) {
        unsafe { run(self.dpy) }
    }
}

impl Drop for Display {
    fn drop(&mut self) {
        let xlib = xlib::Xlib::open().unwrap();
        unsafe {
            cleanup(self.dpy);
            (xlib.XCloseDisplay)(self.dpy);
        }
    }
}
