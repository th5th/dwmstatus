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
CPPFLAGS = -DVERSION=\"${VERSION}\" -DDEBUG
CFLAGS = -g -std=c99 -pedantic -Wall -O0 ${INCS} ${CPPFLAGS}
LDFLAGS = -g ${LIBS}

CC = gcc

all: dwmstatus

.o:
	${CC} -c ${CFLAGS} $<

dwmstatus: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm -f ${NAME} ${OBJ}

install: all
	echo Installing executable file to ${DESTDIR}${PREFIX}/bin
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f ${NAME} ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/${NAME}

uninstall:
	echo Removing executable file from ${DESTDIR}${PREFIX}/bin
	rm -f ${DESTDIR}${PREFIX}/bin/${NAME}

.PHONY: all clean install uninstall
