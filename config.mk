NAME = dwmstatus
VERSION = 0.1-`date +%d%m%y-%H%M%S`
SRC = dwmstatus.c
CFLAGS = -g -std=c99 -DDEBUG -DVERSION=\"${VERSION}\" -Wall -Werror
LDFLAGS = -g -lX11 -liw
DESTDIR = /home/th5th
# CC optionally defined by argument to buildenv
