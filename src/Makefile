CC := gcc
CFLAGS := -O2 $(CFLAGS)
DEFINES := -D_POSIX_C_SOURCE=200809L

.PHONY: all clean

all: sh

sh: main.o builtins.o input.o global.o variables.o
	${CC} ${CFLAGS} -o $@ $^ ${LDFLAGS}

%.o: %.c
	${CC} ${CFLAGS} ${DEFINES} -c -o $@ $^

clean:
	rm -f sh *.o
