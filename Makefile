.PHONY: all sh clean

all: sh

sh:
	${MAKE} --no-print-directory -C src
	cp src/sh $@

clean:
	@${MAKE} --no-print-directory -C src clean
	rm -f sh
