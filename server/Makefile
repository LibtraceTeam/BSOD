include= /usr/local/dag-2.4.11/include
dagcdir= /usr/local/dag-2.4.11/utils/

CC=gcc-3.0
#LINK_FLAGS := `gtk-config --libs`
CFLAGS = -Wall -Wimplicit -Wformat -pipe -g `gtk-config --cflags`
INCLUDE = -I$(include) -I/usr/local/wand/include -I$(dagcdir)

CFLAGS += $(INCLUDE)
#LDFLAGS = -L/home/dhtrl1/wdcap/lib
LDLIBS = -lpcap -lm -lrtclient -lz -L/usr/local/wand/lib `gtk-config --libs`

BINS = test
OBJS = socket.o packets.o
#DAGCOBJ = pcapio.o utils.o erfio.o
#PCAPFIFOOBJS = $(addprefix $(dagcdir), $(DAGCOBJ))

.PHONY: all clean distclean install depend

all: $(BINS) 

clean:
	$(RM) *~ *.o $(BINS) $(OBJS)

distclean:
	$(RM) *~ *.o $(BINS) $(OBJS)


test: $(OBJS) test.o #$(LINK_FLAGS)

# vim: noet ts=8 sw=8
