CXX=g++
CC=g++
INCLUDE = -I/usr/local/wand/include
#CXXFLAGS = -Wall -Wimplicit -Wformat -pipe -g -fPIC
CXXFLAGS = -g -fPIC -shared -Wall -Wimplicit -Wformat -pipe -pedantic
CXXFLAGS += $(INCLUDE)

DIRS := colour position direction
SOURCES := $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cc))

OBJS := $(patsubst %.cc, %.so, $(SOURCES))

PREFIX=/usr/local/bsod
LIBDIR=$(PREFIX)/plugins

.PHONY: all clean distclean install depend

all: $(OBJS)

distclean: clean

clean:
	$(RM) *~ $(OBJS)

%.o: %.c
	$(CXX) -c $(CXXFLAGS) \
	$<
	
%.so: %.o
	$(CXX) $(CXXFLAGS) \
	-o $@ $^

install:
	for dir in $(DIRS); do mkdir -p $(LIBDIR)/$$dir; done
	for obj in $(OBJS); do install -m 644 $$obj $(LIBDIR)/$$(dirname $$obj) ; done
	
# vim: noet ts=8 sw=8
