SUBDIRS=include

AM_CFLAGS=-I./include
AM_CXXFLAGS=-I./include

bin_PROGRAMS=bsodclient

bsodclient_SOURCES=config.cpp \
	flow.cpp \
	flowmanager.cpp \
	font.cpp \
	gl_util.cpp \
	gui.cpp \
	keys.cpp \
	main.cpp \
	misc.cpp \
	particles.cpp \
	ps_classic.cpp \
	ps_shaders.cpp \
	ps_texture.cpp \
	ps_vtf.cpp \
	render.cpp \
	shaders.cpp \
	socket.cpp \
	texture.cpp \
	update.cpp

bsodclient_LDADD= @ADD_LIBS@

all-local: bsodclient$(EXEEXT)
	cp bsodclient$(EXEEXT) ../runtime	
