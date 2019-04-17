.PHONY: all clean

all: sh

sh: src/sh
	cp $< $@

src/sh: src/Makefile
	${MAKE} -C src

clean:
	@${MAKE} --no-print-directory -C src clean
	rm -f sh
