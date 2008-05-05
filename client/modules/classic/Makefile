OUTPUT_FOLDER=../../runtime/modules

#Output filename
NAME= classic.so

#config
CXXFLAGS= -fPIC -pg -O3 -I. -I../ -I../../src/include
LDFLAGS= -shared

#files
SOURCES := $(wildcard *.cpp) ../../src/*.cpp
OBJS := $(patsubst %.cpp, %.o, $(SOURCES)) 

$(NAME): $(OBJS)
	$(CXX) -o $(OUTPUT_FOLDER)/$@ $^ $(CXXFLAGS)  $(LDFLAGS)

clean:
	rm -f *.o $(OUTPUT_FOLDER)/$(NAME)
