CXX=g++-3.0
CC=g++-3.0
CXXFLAGS = -Wall -Wimplicit -Wformat -pipe -g
INCLUDE =  -I/usr/local/include  -I. -I./lib

CXXFLAGS += $(INCLUDE)
LDLIBS =  -lm -ltrace -lz -lpcap -L./lib -L/usr/local/lib -ldl -rdynamic -lconfig -lfl

BINS = bsod_server
OBJS = socket.o packets.o daemons.o debug.o

PREFIX=/usr/local/bsod

BINDIR=$(PREFIX)/bin
CONFDIR=$(PREFIX)/etc
LIBDIR=$(PREFIX)/plugins
.PHONY: all clean distclean install depend plugins instlal-init

all: libconfig $(BINS) plugins 

libconfig:
	cd lib && $(MAKE)  || exit 1


clean: plugins-clean
	$(RM) *~ *.o $(BINS) $(OBJS)
	cd lib && $(MAKE) || exit 1

distclean: clean
	


bsod_server: $(OBJS) bsod_server.o 


plugins:
	cd plugins; make; cd ..

plugins-clean:
	cd plugins; make clean; cd ..

install:
	mkdir -p $(BINDIR)
	mkdir -p $(CONFDIR)
	mkdir -p $(LIBDIR)

	install -m 644 ./bsod_server.config $(CONFDIR)/bsod_server.dist
	install -s -m 755 ./bsod_server $(BINDIR)/bsod_server

	cd plugins; make install; cd ..

install-init:
	install -m 755 ./bsod_server.init /etc/init.d/bsod_server
# vim: noet ts=8 sw=8
