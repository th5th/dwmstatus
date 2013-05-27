include config.mk

OBJ = ${SRC:.c=.o}
DEPEND = ${SRC:.c=.d} 

all: ${NAME}

${NAME}: ${OBJ}
	${CC} -o $@ $^ ${LDFLAGS}

%.d: %.c
	@set -e; rm -f $@
	${CC} -MM ${CFLAGS} ${CPPFLAGS} $< | sed 's/\(^.*\)\.o[ :]*/\1.o $@ : /g' > $@

include ${DEPEND}

install: ${NAME}
	@echo Installing executable to ${DESTDIR}/bin
	mkdir -p ${DESTDIR}/bin
	cp -f ${NAME} ${DESTDIR}/bin
	chmod 755 ${DESTDIR}/bin/${NAME}

uninstall:
	@echo Removing executable from ${DESTDIR}/bin
	rm -f ${DESTDIR}/bin/${NAME}

clean:
	rm -f ${NAME} ${OBJ} ${DEPEND}

.PHONY: clean install uninstall
