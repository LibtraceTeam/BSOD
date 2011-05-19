include ../Makefile.plugins

plugindir = $(prefix)/plugins/direction
AM_CXXFLAGS+=-rdynamic -DDIRNAME=\"$(plugindir)\" $(INCLUDES)

plugin_LTLIBRARIES= destbpf.la destmac.la interface.la

destbpf_la_SOURCES=destbpf.cc 
destmac_la_SOURCES=destmac.cc ../../debug.cpp ../../daemons.cpp 
interface_la_SOURCES=interface.cc 

destbpf_la_LDFLAGS=-module -avoid-version @ADD_LDFLAGS@
destmac_la_LDFLAGS=-module -avoid-version @ADD_LDFLAGS@
interface_la_LDFLAGS=-module -avoid-version @ADD_LDFLAGS@

destbpf_la_LIBADD = @ADD_LIBS@
destmac_la_LIBADD = @ADD_LIBS@
interface_la_LIBADD = @ADD_LIBS@

EXTRA_DIST=direction.h

