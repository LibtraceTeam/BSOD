CXX=g++
CC=g++
CXXFLAGS = -ffast-math -O2 -Wall -Wimplicit -Wformat -pipe -g 
# change this to your libtrace path. 
# this is left here as people often have it installed in their home
# directories for testing things rather than using the system one
LIBTRACE ?= /usr/local
INCLUDE +=  -I. -I$(LIBTRACE)/include

CXXFLAGS += $(INCLUDE)
# GeoIP is required in the server for the GeoIP module
# This should be fixed, but isn't.
#LDLIBS =  -lm -lz -lpcap -ldl  -lfl -ltrace -lGeoIP
LDLIBS =  -lm  -ldl  -lfl -ltrace -lconfuse
LDFLAGS+= -L$(LIBTRACE)/lib -rdynamic 
SUBDIR_LDFLAGS = -L$(LIBTRACE)/lib

# For Mac OS X
# Mac OS X doesn't do -rdynamic
#LDFLAGS= -L./lib -L/usr/local/lib

BINS = bsod_server
OBJS = socket.o packets.o daemons.o debug.o RTTMap.o Blacklist.o 

PREFIX?=/usr/local/bsod

BINDIR=$(PREFIX)/bin
CONFDIR=$(PREFIX)/etc
LIBDIR=$(PREFIX)/plugins
BLISTDIR=$(PREFIX)/blist
.PHONY: all clean distclean install depend plugins install-init

all: $(BINS) plugins 

clean: plugins-clean
	$(RM) *~ *.o $(BINS) $(OBJS)

distclean: clean
	


bsod_server: $(OBJS) bsod_server.o 


plugins:
	cd plugins; $(MAKE) "INCLUDE=$(INCLUDE)" "LDFLAGS=$(SUBDIR_LDFLAGS)"; cd ..

plugins-clean:
	cd plugins; $(MAKE) clean; cd ..

install: all
	mkdir -p $(BINDIR)
	mkdir -p $(CONFDIR)
	mkdir -p $(LIBDIR)
	mkdir -p $(BLISTDIR)

	install -m 644 ./bsod_server.config $(CONFDIR)/bsod_server.conf.sample
	install -m 644 ./mac_addrs $(CONFDIR)/mac_addrs.sample
	install -s -m 755 ./bsod_server $(BINDIR)/bsod_server

	cd plugins; $(MAKE) "PREFIX=$(PREFIX)" install; cd ..

install-init:
	install -m 755 ./bsod_server.init /etc/init.d/bsod_server
# vim: noet ts=8 sw=8
