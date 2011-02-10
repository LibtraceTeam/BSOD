include ../Makefile.plugins

plugindir = $(prefix)/plugins/colour
AM_CXXFLAGS+=-rdynamic -DDIRNAME=\"$(plugindir)\" $(INCLUDES)

plugin_LTLIBRARIES= bpfcolour.la colours.la sipcolour.la

if BUILD_LPI_COLOUR
plugin_LTLIBRARIES+= lpicolour.la

lpicolour_la_SOURCES=lpicolour.cc
lpicolour_la_LDFLAGS=-module -avoid-version @ADD_LDFLAGS@
lpicolour_la_LIBADD = @ADD_LIBS@
endif

bpfcolour_la_SOURCES=bpfcolour.cc bpfcolour.h
#		bpfcolour_lexer.l bpfcolour_parser.y bpfcolour.h
colours_la_SOURCES=colours.cc colours.h
sipcolour_la_SOURCES=sipcolour.cc

bpfcolour_la_LDFLAGS=-module -avoid-version @ADD_LDFLAGS@
colours_la_LDFLAGS=-module -avoid-version @ADD_LDFLAGS@
sipcolour_la_LDFLAGS=-module -avoid-version @ADD_LDFLAGS@

bpfcolour_la_LIBADD = @ADD_LIBS@
colours_la_LIBADD = @ADD_LIBS@
sipcolour_la_LIBADD = @ADD_LIBS@


