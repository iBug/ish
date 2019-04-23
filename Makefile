.PHONY: all test sh clean

all: sh

test: sh
	./sh < test.sh

sh:
	${MAKE} --no-print-directory -C src
	cp src/sh $@

clean:
	@${MAKE} --no-print-directory -C src clean
	rm -f sh
