include= /usr/local/dag-2.4.11/include
dagcdir= /usr/local/dag-2.4.11/utils/

CXX=g++-3.0
CC=g++-3.0
CPPFLAGS = -Wall -Wimplicit -Wformat -pipe -g
INCLUDE = -I$(include) -I/usr/local/wand/include -I$(dagcdir)

CPPFLAGS += $(INCLUDE)
LDLIBS = -lpcap -lm -lrtclient -lz -L/usr/local/wand/lib

BINS = test
OBJS = socket.o packets.o

.PHONY: all clean distclean install depend

all: $(BINS) 

clean:
	$(RM) *~ *.o $(BINS) $(OBJS)

distclean:
	$(RM) *~ *.o $(BINS) $(OBJS)


test: $(OBJS) test.o

# vim: noet ts=8 sw=8
