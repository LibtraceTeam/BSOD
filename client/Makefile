######################################
#                                    #
#          Generic makefile          #
#                                    #
#           by George Foot           #
# email: george.foot@merton.ox.ac.uk #
#                                    #
#   Copyright (c) 1997 George Foot   #
#        All rights reserved.        #
#                                    #
#     No warranty, no liability;     #
#   you use this at your own risk.   #
#                                    #
#     You are free to modify and     #
#   distribute this without giving   #
#   credit to the original author.   #
#                                    #
######################################

### Customising
#
# Adjust the following if necessary; EXECUTABLE is the target
# executable's filename, and LIBS is a list of libraries to link in
# (e.g. alleg, stdcx, iostr, etc). You can override these on make's
# command line of course, if you prefer to do it that way.

# TODO: use sdl-config
# notes: pthread is needed for openal

EXECUTABLE := bsod-client
LIBS := IL ILU stlport_gcc GL GLU SDL NL lua  enet z
LINK_FLAGS := -Lexternal/DevIL/lib -Lexternal/HawkNL/lib -Lexternal/OPCODE \
	-Lexternal/lua/lib -Lexternal/OpenAL/linux/src -Lexternal/ode/lib \
    -Lexternal/enet -Lexternal/GL

# Now alter any implicit rules' variables if you like, e.g.:
CFLAGS := -g -Wall `linux/copts`
#-O3 -m486 -g -mathlon-xp
CXXFLAGS := $(CFLAGS)

# The next bit checks to see whether rm is in your djgpp bin
# directory; if not it uses del instead, but this can cause (harmless)
# `File not found' error messages. If you are not using DOS at all,
# set the variable to something which will unquestioningly remove
# files.

RM-F := rm -f

# You shouldn't need to change anything below this point.

SOURCE := $(wildcard *.cpp) $(wildcard gl/*.cpp) $(wildcard linux/*.cpp)
OBJS := $(patsubst %.cpp,%.o,$(SOURCE))
DEPS := $(patsubst %.o,%.d,$(foreach o,$(OBJS),$(notdir $o)))
MISSING_DEPS := $(filter-out $(wildcard $(DEPS)),$(DEPS))
MISSING_DEPS_SOURCES := $(wildcard $(patsubst %.d,%.c,$(MISSING_DEPS)) \
                                   $(patsubst %.d,%.cpp,$(MISSING_DEPS)))
CPPFLAGS += -MD

.PHONY : everything deps objs clean veryclean rebuild

everything : $(EXECUTABLE)

deps : $(DEPS)

objs : $(OBJS)

clean :
	@$(RM-F) $(OBJS)
	@$(RM-F) $(DEPS)

veryclean: clean
	@$(RM-F) $(EXECUTABLE)

rebuild: veryclean everything

ifneq ($(MISSING_DEPS),)
$(MISSING_DEPS) :
	@$(RM-F) $(patsubst %.d,%.o,$@)
endif

-include $(DEPS)

$(EXECUTABLE) : $(OBJS)
	g++ -o $(EXECUTABLE) $(OBJS) $(LINK_FLAGS) $(addprefix -l,$(LIBS))
