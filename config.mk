NAME = dwmstatus
VERSION = 0.2-`date +%d%m%y-%H%M%S`
SRC = dwmstatus.c
CFLAGS = -g -std=gnu99 -Wall -Werror -DVERSION=\"${VERSION}\"
LDFLAGS = -g -lX11
DESTDIR = /usr/local
CC = clang
