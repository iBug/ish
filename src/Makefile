CC := gcc
CFLAGS := -O2

.PHONY: all clean

all: sh

sh: shell.o builtins.o input.o global.o
	${CC} ${CFLAGS} -o $@ $^ ${LDFLAGS}

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $^

clean:
	rm -f sh *.o
