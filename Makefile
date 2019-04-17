CC := gcc
CFLAGS := -O2

.PHONY: all clean

all: sh

sh: shell.c
	${CC} ${CFLAGS} -o $@ $^ ${LDFLAGS}

clean:
	rm -f *.o sh
