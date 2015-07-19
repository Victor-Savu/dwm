extern crate gcc;

fn main() {
    gcc::Config::new()
        .define("VERSION", Some("\"6.1\""))
        .define("XINERAMA", None)
        .define("_BSD_SOURCE", None)
        .define("_POSIX_C_SOURCE", Some("2"))
        .file("thirdparty/cleanup.c")
        .file("thirdparty/config.c")
        .file("thirdparty/config_fonts.c")
        .file("thirdparty/die.c")
        .file("thirdparty/drw.c")
        .file("thirdparty/globals.c")
        .file("thirdparty/handlers.c")
        .file("thirdparty/keyboard.c")
        .file("thirdparty/keys.c")
        .file("thirdparty/layouts.c")
        .file("thirdparty/lib.c")
        .file("thirdparty/mouse.c")
        .file("thirdparty/run.c")
        .file("thirdparty/setup.c")
        .file("thirdparty/spawn.c")
        .file("thirdparty/update.c")
        .file("thirdparty/updategeom.c")
        .flag("-O2")
        .flag("-Wall")
        .flag("-Wno-deprecated-declarations")
        .flag("-fdump-rtl-expand")
        .flag("-pedantic")
        .flag("-std=c99")
        .include("/usr/include/freetype2")
        .include("/usr/include/x86_64-linux-gnu")
        .include("thirdparty")
        .compile("libdwm.a");

    println!("cargo:rustc-link-search=/usr/lib/x86_64-linux-gnu");

    println!("cargo:rustc-link-lib=X11");
    println!("cargo:rustc-link-lib=Xinerama");
    println!("cargo:rustc-link-lib=Xft");
    println!("cargo:rustc-link-lib=fontconfig");
}
