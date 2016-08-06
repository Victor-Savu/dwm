extern crate dwm;

use dwm::display::{Display, OpenDisplayError};

fn main() {
    match Display::default() {
        Ok(dpy) => dpy.run(),
        Err(OpenDisplayError::CannotOpen) =>
            println!("Cannot open default display."),
        Err(OpenDisplayError::CannotSelect) =>
            println!("Cannot use this display. Another window manager might be using it."),
    }
}
