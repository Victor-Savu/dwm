use std::ptr::{ null, null_mut };

use x11_dl::xlib;

extern {
    fn checkotherwm(dpy: *mut xlib::Display);
    fn setup(dpy: *mut xlib::Display);
    fn scan(dpy: *mut xlib::Display);
    fn run(dpy: *mut xlib::Display);
    fn cleanup(dpy: *mut xlib::Display);
}

pub struct Display {
    dpy: *mut xlib::Display,
}

impl Display {
    pub fn new() -> Self {
        let xlib = xlib::Xlib::open().unwrap();
        unsafe {
            /*
               if(!setlocale(LC_CTYPE, "") || )
               */
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
        unsafe { checkotherwm(self.dpy) }
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
