CC := gcc
CFLAGS := -O2 $(CFLAGS)
DEFINES := -D_POSIX_C_SOURCE=200809L

# Uncomment the following lines for GNU readline support
DEFINES += -DHAVE_READLINE
LDFLAGS += -lreadline

# Uncomment the following line for color PS1 prompt (built-in)
DEFINES += -DCOLOR_PROMPT

.PHONY: all clean

all: sh

sh: main.o builtins.o input.o global.o variables.o parsing.o
	${CC} ${CFLAGS} -o $@ $^ ${LDFLAGS}

%.o: %.c
	${CC} ${CFLAGS} ${DEFINES} -c -o $@ $^

clean:
	rm -f sh *.o
