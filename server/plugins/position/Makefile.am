include ../Makefile.plugins

plugindir = $(prefix)/plugins/position
AM_CXXFLAGS+=-rdynamic -DDIRNAME=\"$(plugindir)\" $(INCLUDES)

plugin_LTLIBRARIES= multiplenet24.la network16.la networkxxyy.la radial.la \
		random.la

multiplenet24_la_SOURCES=multiplenet24.cc ../../debug.cpp ../../daemons.cpp
network16_la_SOURCES=network16.cc
networkxxyy_la_SOURCES=networkxxyy.cc
radial_la_SOURCES=radial.cc
random_la_SOURCES=random.cc

multiplenet24_la_LDFLAGS=-module -avoid-version @ADD_LDFLAGS@
network16_la_LDFLAGS=-module -avoid-version @ADD_LDFLAGS@
networkxxyy_la_LDFLAGS=-module -avoid-version @ADD_LDFLAGS@
radial_la_LDFLAGS=-module -avoid-version @ADD_LDFLAGS@
random_la_LDFLAGS=-module -avoid-version @ADD_LDFLAGS@

multiplenet24_la_LIBADD = @ADD_LIBS@
network16_la_LIBADD = @ADD_LIBS@
networkxxyy_la_LIBADD = @ADD_LIBS@
radial_la_LIBADD = @ADD_LIBS@
random_la_LIBADD = @ADD_LIBS@

EXTRA_DIST=position.h

