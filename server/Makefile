include= /usr/local/dag-2.4.11/include
dagcdir= /usr/local/dag-2.4.11/utils/

CXX=g++-3.0
CC=g++-3.0
CPPFLAGS = -Wall -Wimplicit -Wformat -pipe -g
INCLUDE = -I$(include) -I/usr/local/wand/include -I$(dagcdir) -I.

CPPFLAGS += $(INCLUDE)
LDLIBS =  -lm -ltrace -lz -lpcap -L/usr/local/wand/lib -ldl -rdynamic 

BINS = bsod_server
OBJS = socket.o packets.o

.PHONY: all clean distclean install depend plugins

all: $(BINS) plugins

clean: plugins-clean
	$(RM) *~ *.o $(BINS) $(OBJS)

distclean: clean
	


bsod_server: $(OBJS) bsod_server.o

plugins:
	cd plugins; make; cd ..

plugins-clean:
	cd plugins; make clean; cd ..
# vim: noet ts=8 sw=8
