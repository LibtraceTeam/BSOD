CXXFLAGS=-I. -Wall -w
CFLAGS=-I. -g -Wall -w
TESTS=$(wildcard tests/*.c)
SUCCESS=$(addsuffix .test,$(basename $(TESTS)))

all: libconfig.a

libconfig.a:  \
	     libconfig.a(config.parser.o) \
	     libconfig.a(libconfig.o) \
	     libconfig.a(config.lexer.o)
	ranlib $@

test: $(SUCCESS) libconfig.a

clean:
	rm -f libconfig.a tests/*.test *.o config.parser.h config.parser.c config.lexer.c

config.parser.c: config.y
	bison -d -o $@ config.y

config.lexer.c: config.l
	flex -o$@ config.l

config.test: %.o libconfig.a daemons.o debug.o
	gcc $(CFLAGS) -w $< -L. -lconfig -lfl debug.o daemons.o -o $(basename $<)
	$(basename $<) $(basename $<).in > $(basename $<).test

.PHONY: all
