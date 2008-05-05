OUTPUT_FOLDER=../../runtime/modules

#Output filename
NAME= earth.so

#config
CXXFLAGS= -fPIC
LDFLAGS= -shared

#files
SOURCES := $(wildcard *.cpp) ../../src/*.cpp
OBJS := $(patsubst %.cpp, %.o, $(SOURCES)) 
DEPS := $(wildcard include/*.h) 
CXXFLAGS := -I. -I../ -I../../src/include

$(NAME): $(OBJS)
	$(CXX) -o $(OUTPUT_FOLDER)/$@ $^ $(CXXFLAGS)  $(LDFLAGS)

clean:
	rm -f *.o $(OUTPUT_FOLDER)/$(NAME)
