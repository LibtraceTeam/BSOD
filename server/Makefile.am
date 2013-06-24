SUBDIRS=plugins images

bin_PROGRAMS=bsodserver

bsodserver_SOURCES=Blacklist.cpp \
	bsod_server.cc \
	daemons.cpp \
	debug.cpp \
	packets.cc \
	RTTMap.cpp \
	socket.cc \
	Blacklist.h \
	bsod_server.h \
	daemons.h \
	debug.h \
	packets.h \
	RTTMap.h \
	socket.h \
	lru \
	exporter.h
	exporter.cc

bsodserver_LDADD=@ADD_LIBS@

EXTRA_DIST=bsod_server.conf

install-data-hook:
	mkdir -p $(prefix)/blist
