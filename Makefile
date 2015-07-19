# dwm - dynamic window manager
# See LICENSE file for copyright and license details.

include config.mk

LIB_SRC = cleanup.c config.c config_fonts.c die.c drw.c globals.c lib.c main.c setup.c update.c updategeom.c xerrors.c
DWM_SRC = main.c

LIB_OBJ = ${LIB_SRC:.c=.o}

all: options dwm

options:
	@echo dwm build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${LIB_OBJ}: config.mk
${DWM_OBJ}: config.mk

lib.a: ${LIB_OBJ}
	@echo AR ruv $@
	@${AR} ruv $@ ${LIB_OBJ}

dwm: ${DWM_OBJ} lib.a
	@echo CC -o $@
	@${CC} -o $@ ${DWM_OBJ} lib.a ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f dwm lib.a ${DWM_OBJ} ${LIB_OBJ} dwm-${VERSION}.tar.gz

.PHONY: all options clean
