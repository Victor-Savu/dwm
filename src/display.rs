use std::ptr::{null, null_mut};

use x11_dl::xlib;
use libc;

extern "C" {
    fn setup(dpy: *mut xlib::Display);
    fn scan(dpy: *mut xlib::Display);
    fn run(dpy: *mut xlib::Display);
    fn cleanup(dpy: *mut xlib::Display);
}

pub struct Display {
    dpy: *mut xlib::Display,
}

static mut err: bool = false;

unsafe extern "C" fn xerrorstart(_: *mut xlib::Display, _: *mut xlib::XErrorEvent) -> libc::c_int {
    err = true;
    return 0;
}

unsafe fn select(xlib: &xlib::Xlib, dpy: *mut xlib::Display) -> bool {
    let xerrorxlib = (xlib.XSetErrorHandler)(Some(xerrorstart));
    // this causes an error if some other window manager is running
    let win = (xlib.XDefaultRootWindow)(dpy);
    (xlib.XSelectInput)(dpy, win, xlib::SubstructureRedirectMask);
    (xlib.XSync)(dpy, 0);
    (xlib.XSetErrorHandler)(xerrorxlib);
    (xlib.XSync)(dpy, 0);
    let ret = !err;
    err = false;
    return ret;
}

unsafe extern "C" fn log_error(_: *mut xlib::Display, ee: *mut xlib::XErrorEvent) -> libc::c_int {
    println!("dwm: fatal error: request code={}, error code={}",
             (*ee).request_code,
             (*ee).error_code);
    0
}

#[derive(Debug)]
pub enum OpenDisplayError {
    CannotOpen,
    CannotSelect,
}

impl Display {
    pub fn default() -> Result<Self, OpenDisplayError> {
        let xlib = xlib::Xlib::open().unwrap();

        let dpy = unsafe {
            (xlib.XSetErrorHandler)(Some(log_error));
            if (xlib.XSupportsLocale)() == 0 {
                println!("warning: no locale support");
            }

            let dpy = (xlib.XOpenDisplay)(null());

            if dpy == null_mut() {
                return Err(OpenDisplayError::CannotOpen);
            }

            if !select(&xlib, dpy) {
                return Err(OpenDisplayError::CannotSelect);
            }

            setup(dpy);
            scan(dpy);

            dpy
        };

        Ok(Display { dpy: dpy })
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
