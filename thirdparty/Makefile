# dwm - dynamic window manager
# See LICENSE file for copyright and license details.

include config.mk

SRC = cleanup.c config.c config_fonts.c die.c drw.c globals.c lib.c setup.c update.c updategeom.c xerrors.c

OBJ = ${SRC:.c=.o}

all: options libdwm.a

options:
	@echo libdwm.a build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"
	@echo "AR       = ${AR}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

libdwm.a: ${OBJ}
	@echo AR ruv $@
	@${AR} ruv $@ ${OBJ}

clean:
	@echo cleaning
	@rm -f libdwm.a ${OBJ}

.PHONY: all options clean
