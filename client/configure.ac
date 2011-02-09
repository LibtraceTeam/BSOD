AC_INIT([bsod-client],[2.0.0],[contact@wand.net.nz])

AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/main.cpp])
AM_CONFIG_HEADER(config.h)

# Make sure we use the relatively silent automake output
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_PREFIX_DEFAULT(/usr/local/)

# Function that checks if the C++ compiler actually works - there's a bit of
# oversight in autoconf that will set the C++ compiler to g++ if no compiler
# is found, even if g++ is not present! So we need an extra test to make sure
# that the compiler works :(

AC_DEFUN([rw_PROG_CXX_WORKS],
[AC_REQUIRE([AC_PROG_CXX])dnl
AC_CACHE_CHECK([whether the C++ compiler works],
	[rw_cv_prog_cxx_works],
	[AC_LANG_PUSH([C++])
	AC_LINK_IFELSE([AC_LANG_PROGRAM([], [])],
		[rw_cv_prog_cxx_works=yes],
		[rw_cv_prog_cxx_works=no])
	AC_LANG_POP([C++])])
])


AC_PROG_CC
AC_PROG_CXX
rw_PROG_CXX_WORKS
AC_PROG_INSTALL

AC_LANG(C++)
AC_PROG_LIBTOOL

build_tools=yes

AC_CONFIG_FILES([Makefile src/Makefile src/include/Makefile runtime/Makefile
		runtime/data/Makefile runtime/data/gui/Makefile 
		runtime/data/shaders/Makefile])

AC_SEARCH_LIBS([SDL_Init], [SDL],,SDL_found=0)
AC_SEARCH_LIBS([SDLNet_Init], [SDL_net],,SDLNet_found=0)
AC_SEARCH_LIBS([TTF_Init],[SDL_ttf],,SDLttf_found=0)
AC_SEARCH_LIBS([ilGenImages],[IL],,devil_found=0)
AC_SEARCH_LIBS([iluGetInteger],[ILU],,ilu_found=0)
AC_SEARCH_LIBS([ilutGetInteger],[ILUT],,ilut_found=0)
AC_SEARCH_LIBS([glewInit],[GLEW],,glew_found=0)
AC_SEARCH_LIBS([cfg_init],[confuse],,conf_found=0)

# Hackish, but it works 
LIBS="$LIBS -lCEGUIBase -lCEGUIOpenGLRenderer -lCEGUIXercesParser"
CPPFLAGS="$CPPFLAGS -I/usr/include/CEGUI"

AC_MSG_CHECKING(for libCEGUI)
AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <CEGUI.h>]],
	[CEGUI::WindowManager::getSingleton();])],cegui_found=1,cegui_found=0)

if test "$cegui_found" = 0; then
	AC_MSG_RESULT(No)
else
	AC_MSG_RESULT(Yes)
fi

depend_error=0

if test "$SDL_found" = 0; then
	AC_MSG_NOTICE(Required library libSDL not found!)
	depend_error=1
fi

if test "$SDLNet_found" = 0; then
	AC_MSG_NOTICE(Required library libSDL_net not found!)
	depend_error=1
fi

if test "$SDLttf_found" = 0; then
	AC_MSG_NOTICE(Required library libSDL_ttf not found!)
	depend_error=1
fi

if test "$devil_found" = 0; then
	AC_MSG_NOTICE(Required library IL not found!)
	depend_error=1
fi

if test "$ilu_found" = 0; then
	AC_MSG_NOTICE(Required library ILU not found!)
	depend_error=1
fi

if test "$ilut_found" = 0; then
	AC_MSG_NOTICE(Required library ILUT not found!)
	depend_error=1
fi

if test "$glew_found" = 0; then
	AC_MSG_NOTICE(Required library libGLEW not found!)
	depend_error=1
fi

if test "$conf_found" = 0; then
	AC_MSG_NOTICE(Required library libconfuse not found!)
	depend_error=1
fi

if test "$cegui_found" = 0; then
	AC_MSG_NOTICE(Required library libCEGUI not found!)
	depend_error=1
fi

if test "$depend_error" = 1; then
	AC_MSG_ERROR(Critical libraries are missing - please install the missing libraries listed above and rerun ./configure)
fi

AC_SUBST([ADD_LIBS])
AC_SUBST([ADD_LDFLAGS])
AC_SUBST([ADD_INCLS])
AC_SUBST([LTLIBOBJS])

AC_OUTPUT

