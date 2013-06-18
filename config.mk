NAME = dwmstatus
VERSION = 0.1-`date +%d%m%y-%H%M%S`
SRC = dwmstatus.c
CFLAGS = -g -std=gnu99 -DDEBUG -DVERSION=\"${VERSION}\" -Wall -Werror
LDFLAGS = -g -lX11 -liw
DESTDIR = /usr/local
CC = clang
