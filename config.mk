# dwm version
VERSION = 6.1

# Customize below to fit your system

# paths
PREFIX = /home/victor/Tools/local
MANPREFIX = ${PREFIX}/share/man

X11INC = /usr/include/x86_64-linux-gnu
X11LIB = /usr/lib/x86_64-linux-gnu

# Xinerama, comment if you don't want it
XINERAMALIBS  = -lXinerama
XINERAMAFLAGS = -DXINERAMA

# includes and libs
INCS = -I${X11INC} -I/usr/include/freetype2
LIBS = -L${X11LIB} -lX11 ${XINERAMALIBS} -lfontconfig -lXft

# flags
CPPFLAGS = -D_BSD_SOURCE -D_POSIX_C_SOURCE=2 -DVERSION=\"${VERSION}\" ${XINERAMAFLAGS}
#CFLAGS   = -g -std=c99 -pedantic -Wall -O0 ${INCS} ${CPPFLAGS}
#CFLAGS   = -std=c99 -pedantic -Wall -Wno-deprecated-declarations -Os ${INCS} ${CPPFLAGS}
CFLAGS   = -std=c99 -w -Os ${INCS} ${CPPFLAGS}
LDFLAGS  = -s ${LIBS}

# Solaris
#CFLAGS = -fast ${INCS} -DVERSION=\"${VERSION}\"
#LDFLAGS = ${LIBS}

# compiler and linker
CC = cc
