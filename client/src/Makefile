#------------------------------------------------------#
#			Configuration
#------------------------------------------------------#


#Output filename
NAME= bsod2


#files
SOURCES := $(wildcard *.cpp) 
OBJS := $(patsubst %.cpp, %.o, $(SOURCES)) 
DEPS := $(wildcard include/*.h) 

#compiler-type things
CXXFLAGS=-Iinclude `pkg-config --cflags CEGUI` -O3 -W -Wextra -g -pg
LDFLAGS=-L../lib/ -lGL -lGLU `sdl-config --cflags --libs` -lIL -lILU -lILUT -lSDL_ttf -lSDL_net -lconfuse -lGLEW `pkg-config --libs CEGUI-OPENGL` -lCEGUIXercesParser
#CGL libs
#-lCg -lCgGL 

#------------------------------------------------------#
#					Compile
#------------------------------------------------------#
all: $(OBJS) $(NAME)
  
$(NAME): $(OBJS) 
	$(CXX) -o ../runtime/$@ $^ $(CXXFLAGS) $(LDFLAGS)
	
#------------------------------------------------------#
#					Test
#------------------------------------------------------#
run-app:
	 @../runtime/$(NAME)

#------------------------------------------------------#
#					Clean
#------------------------------------------------------#
clean:
	rm -f *.o
	rm -f ../runtime/$(NAME)
