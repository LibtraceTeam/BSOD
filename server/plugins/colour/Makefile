CFLAGS=-g -Wall -W
YFLAGS=-d -pcolour_
LFLAGS=-Pcolour_
CXXFLAGS=-g -fPIC -shared -Wall -W -pipe -pedantic
LDLIBS=-lfl -ltrace
LD=g++
OBJS=bpfcolour.so colours.so sipcolour.so

all: $(OBJS)

bpfcolour_lexer.o: bpfcolour_lexer.l bpfcolour_parser.o

bpfcolour.so: bpfcolour_lexer.o bpfcolour_parser.o bpfcolour.o

bpfcolour: bpfcolour_lexer.o bpfcolour_parser.o bpfcolour.o
	$(LD) -o $@ $^ $(LDFLAGS) $(LDLIBS)

%.so: %.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) \
	-o $@ $^

clean:
	rm -f *.o *.so core y.tab.h
