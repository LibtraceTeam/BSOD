CXX=g++
CC=g++
CXXFLAGS = -Wall -Wimplicit -Wformat -pipe -g
INCLUDE =  -I/usr/local/include  -I. -I./lib

CXXFLAGS += $(INCLUDE)
LDLIBS =  -lm -ltrace -lz -lpcap -L./lib -L/usr/local/lib -ldl -rdynamic -lconfig -lfl

BINS = bsod_server
OBJS = socket.o packets.o daemons.o debug.o RTTMap.o Blacklist.o

PREFIX=/usr/local/bsod

BINDIR=$(PREFIX)/bin
CONFDIR=$(PREFIX)/etc
LIBDIR=$(PREFIX)/plugins
BLISTDIR=$(PREFIX)/blist
.PHONY: all clean distclean install depend plugins install-init

all: libconfig $(BINS) plugins 

libconfig:
	cd lib && $(MAKE)  || exit 1


clean: plugins-clean
	$(RM) *~ *.o $(BINS) $(OBJS)
	cd lib && $(MAKE) clean || exit 1

distclean: clean
	


bsod_server: $(OBJS) bsod_server.o 


plugins:
	cd plugins; $(MAKE); cd ..

plugins-clean:
	cd plugins; $(MAKE) clean; cd ..

install:
	mkdir -p $(BINDIR)
	mkdir -p $(CONFDIR)
	mkdir -p $(LIBDIR)
	mkdir -p $(BLISTDIR)

	install -b -m 644 ./bsod_server.config $(CONFDIR)/bsod_server.conf
	install -b -m 644 ./mac_addrs $(CONFDIR)/mac_addrs
	install -s -m 755 ./bsod_server $(BINDIR)/bsod_server

	cd plugins; $(MAKE) install; cd ..

install-init:
	install -m 755 ./bsod_server.init /etc/init.d/bsod_server
# vim: noet ts=8 sw=8
