.PHONY: all clean

CXXFLAGS+=-I. -I/usr/include/SDL
CXXFLAGS+=-DLINUX -D_REENTRANT
CXXFLAGS+=-Wall -W -g -O2 -Wno-non-template-friend
LDLIBS+=-lIL -lILU -lGL -lGLU -lz -lSDL_net -lconfig

SOURCES= action.cpp actor.cpp bezier.cpp camera.cpp entity.cpp \
 entity_manager.cpp font.cpp gui.cpp levelloader.cpp loading_screen.cpp \
 main.cpp matrix.cpp md2.cpp md3.cpp menuitem.cpp misc.cpp \
 octree.cpp partflow.cpp partvis.cpp player.cpp polygon.cpp quaternion.cpp \
 reporter.cpp sdl_net_driver.cpp texture_manager.cpp vector.cpp vfs.cpp \
 world.cpp xml_parse.cpp 

SOURCES+= gl/gl_display_manager.cpp
SOURCES+= linux/linux_system_driver.cpp

OBJECTS=$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))

all: $(OBJECTS)
	g++ $(LDFLAGS) $(CXXFLAGS) $(OBJECTS) -o bsod_client $(LDLIBS)

clean:
	rm -f $(OBJECTS) bsod_client
	$(MAKE) -C lib clean
