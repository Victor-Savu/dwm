extern crate gcc;

fn main() {
    gcc::Config::new()
        .include("/usr/include/x86_64-linux-gnu")
        .include("/usr/include/freetype2")
        .file("thirdparty/cleanup.c")
        .file("thirdparty/config.c")
        .file("thirdparty/config_fonts.c")
        .file("thirdparty/die.c")
        .file("thirdparty/drw.c")
        .file("thirdparty/globals.c")
        .file("thirdparty/lib.c")
        .file("thirdparty/setup.c")
        .file("thirdparty/update.c")
        .file("thirdparty/updategeom.c")
        .include("thirdparty")
        .define("_POSIX_C_SOURCE", Some("2"))
        .define("_BSD_SOURCE", None)
        .define("VERSION", Some("\"6.1\""))
        .define("XINERAMA", None)
        .flag("-std=c99")
        .flag("-pedantic")
        .flag("-Wall")
        .flag("-Wno-deprecated-declarations")
        .flag("-O2")
        .flag("-fdump-rtl-expand")
        .compile("libdwm.a");

    println!("cargo:rustc-link-search=/usr/lib/x86_64-linux-gnu");

    println!("cargo:rustc-link-lib=X11");
    println!("cargo:rustc-link-lib=Xinerama");
    println!("cargo:rustc-link-lib=Xft");
    println!("cargo:rustc-link-lib=fontconfig");
}
