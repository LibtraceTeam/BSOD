CXX=g++
CC=g++
INCLUDE= -I/usr/local/include -I/home/perry/lib/include
#CXXFLAGS = -Wall -Wimplicit -Wformat -pipe -g -fPIC
CXXFLAGS+=-g -fPIC -shared -Wall -Wimplicit -Wformat -W -pipe -pedantic
CXXFLAGS+=$(INCLUDE)
LDFLAGS+=-L/home/perry/lib/lib -shared -fPIC

DIRS := position direction
SOURCES := $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cc))
SUBDIRS := colour

OBJS := $(patsubst %.cc, %.so, $(SOURCES))

PREFIX=/usr/local/bsod
LIBDIR=$(PREFIX)/plugins

.PHONY: all clean distclean install depend

all: $(OBJS)
	for i in $(SUBDIRS); do \
		$(MAKE) -C $$i all; \
	done

distclean: clean

clean:
	$(RM) *~ $(OBJS)
	for i in $(SUBDIRS); do \
		$(MAKE) -C $$i clean; \
	done

%.o: %.c
	$(CXX) -c $(CXXFLAGS) \
	$<
	
%.so: %.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) \
	-o $@ $^

install:
	for dir in $(DIRS); do mkdir -p $(LIBDIR)/$$dir; done
	for obj in $(OBJS); do install -m 644 $$obj $(LIBDIR)/$$(dirname $$obj) ; done
	
# vim: noet ts=8 sw=8
