CXX=g++-3.0
CC=g++-3.0
INCLUDE = -I/usr/local/wand/include
#CPPFLAGS = -Wall -Wimplicit -Wformat -pipe -g -fPIC
CPPFLAGS = -g -fPIC -shared -Wall -Wimplicit -Wformat -pipe -Wpedantic
CPPFLAGS += $(INCLUDE)

DIRS := colour position direction
SOURCES := $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cc))

OBJS := $(patsubst %.cc, %.so, $(SOURCES))


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

	
# vim: noet ts=8 sw=8
