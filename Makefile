# Makefile
NAME = dwmstatus
VERSION = 0.1-`date +%d%m%y-%H%M%S`

SRC = dwmstatus.c
OBJ = ${SRC:.c=.o}

# paths
PREFIX = /home/th5th/

# includes and libs
#INCS = -I/include/path
LIBS = -lX11

# flags
CPPFLAGS = -DVERSION=\"${VERSION}\"
CFLAGS = -g -std=c99 -pedantic -Wall -O0 ${INCS}
LDFLAGS = -g ${LIBS}

CC = gcc

debug: dwmstatus
debug: CPPFLAGS += -DDEBUG 

release: dwmstatus

.o:
	${CC} -c ${CFLAGS} ${CPPFLAGS} $<

dwmstatus: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm -f ${NAME} ${OBJ}

install: release
	echo Installing executable file to ${DESTDIR}${PREFIX}/bin
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f ${NAME} ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/${NAME}

uninstall:
	echo Removing executable file from ${DESTDIR}${PREFIX}/bin
	rm -f ${DESTDIR}${PREFIX}/bin/${NAME}

.PHONY: clean debug install release uninstall
