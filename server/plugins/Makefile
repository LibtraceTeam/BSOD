CXX=g++-3.0
CC=g++-3.0
INCLUDE = -I/usr/local/wand/include
#CPPFLAGS = -Wall -Wimplicit -Wformat -pipe -g -fPIC
CPPFLAGS = -g -fPIC -shared -Wall -Wimplicit -Wformat -pipe -Wpedantic
CPPFLAGS += $(INCLUDE)

DIRS := colour position direction
SOURCES := $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cc))

OBJS := $(patsubst %.cc, %.so, $(SOURCES))

PREFIX=/usr/local/bsod
LIBDIR=$(PREFIX)/plugins

#all: $(OUT)

.PHONY: all clean distclean install depend

all: $(OBJS)

distclean: clean

clean:
	$(RM) *~ $(OBJS)

%.o: %.c
	$(CC) -c -fPIC $<
	
%.so: %.o
	g++ -fPIC -shared -o $@ $^
#$(CC) --shared -Wl,-export-all-symbols $^ -o $@
#ld -x --shared $< -o $@

install:
	for dir in $(DIRS); do mkdir -p $(LIBDIR)/$$dir; done
	for obj in $(OBJS); do install -m 644 $$obj $(LIBDIR)/$$(echo $$obj | cut -d "/" -f 1) ; done
	
# vim: noet ts=8 sw=8
