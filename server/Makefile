CXX=g++-3.0
CC=g++-3.0
CPPFLAGS = -Wall -Wimplicit -Wformat -pipe -g
INCLUDE =  -I/usr/local/include  -I. -I./lib

CPPFLAGS += $(INCLUDE)
LDLIBS =  -lm -ltrace -lz -lpcap -L./lib -L/usr/local/lib -ldl -rdynamic -lconfig -lfl

BINS = bsod_server
OBJS = socket.o packets.o daemons.o debug.o


.PHONY: all clean distclean install depend plugins

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
# vim: noet ts=8 sw=8
