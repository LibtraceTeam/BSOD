AC_INIT([bsod-client],[2.0.2],[contact@wand.net.nz])

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

AC_CONFIG_FILES([Makefile src/Makefile src/include/Makefile runtime/Makefile
		runtime/data/Makefile runtime/data/gui/Makefile 
		runtime/data/shaders/Makefile])

AC_CANONICAL_HOST

AC_SEARCH_LIBS([SDL_Init], [SDL],,SDL_found=0)
AC_SEARCH_LIBS([SDLNet_Init], [SDL_net],,SDLNet_found=0)
AC_SEARCH_LIBS([TTF_Init],[SDL_ttf],,SDLttf_found=0)
AC_SEARCH_LIBS([png_read_image],[png],,png_found=0)
AC_SEARCH_LIBS([glewInit],[GLEW],,glew_found=0)
AC_SEARCH_LIBS([glClear],[GL],,gl_found=0)
AC_SEARCH_LIBS([gluOrtho2D],[GLU],,glu_found=0)

if test "$host_vendor" == "apple"; then
	LIBS="$LIBS -lintl -liconv -lc"
fi

AC_SEARCH_LIBS([cfg_init],[confuse],,conf_found=0)

AC_ARG_WITH(cgl,
		AS_HELP_STRING(--with-cgl, enable ClusterGL support),
[
	if test "x$withval" = xyes
	then 
		enable_cgl=yes
	elif test "x$withval" = xno
	then
		enable_cgl=no
	else
		enable_cgl=yes
	fi
],[
	enable_cgl=no
])


# Hackish, but it works 
AC_MSG_CHECKING(for libCEGUI)
if test "$host_vendor" == "apple"; then
	CPPFLAGS="$CPPFLAGS"
	LDFLAGS="$LDFLAGS -framework CoreFoundation -framework AppKit -framework OpenGL -framework CEGUIBase -framework CEGUIOpenGLRenderer"
	AC_LINK_IFELSE([AC_LANG_PROGRAM([[
		#include <CoreFoundation/CoreFoundation.h>
		#include <CEGUIBase/CEGUI.h>
		]],
		[CEGUI::WindowManager::getSingleton();])],
		cegui_found=1,cegui_found=0)

else
	LIBS="$LIBS -lCEGUIBase -lCEGUIOpenGLRenderer -lCEGUIXercesParser"
	CPPFLAGS="$CPPFLAGS -I/usr/include/CEGUI"
	AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <CEGUI/CEGUI.h>]],
		[CEGUI::WindowManager::getSingleton();])],
		cegui_found=1,cegui_found=0)
fi

if test "$cegui_found" = 0; then
	AC_MSG_RESULT(No)
else
	AC_MSG_RESULT(Yes)
	AC_MSG_CHECKING(libCEGUI version)

	if test "$host_vendor" == "apple"; then
		AC_LINK_IFELSE([AC_LANG_PROGRAM([
		[#include <CoreFoundation/CoreFoundation.h>
		#include <CEGUIBase/CEGUI.h>
		#include <CEGUIBase/RendererModules/OpenGL/CEGUIOpenGLRenderer.h>]],
		[CEGUI::WindowManager::getSingleton();])],newcegui_found=1,
		newcegui_found=0)
		
	else	
		AC_LINK_IFELSE([AC_LANG_PROGRAM([
		[#include <CEGUI/CEGUI.h>
		#include <CEGUI/RendererModules/OpenGL/CEGUIOpenGLRenderer.h>]],
		[CEGUI::WindowManager::getSingleton();])],newcegui_found=1,
		newcegui_found=0)
	fi

	if test "$newcegui_found" = 1; then
		AC_MSG_RESULT(0.7)
	else
		AC_MSG_RESULT(0.6 or previous)
	fi
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

if test "$png_found" = 0; then
	AC_MSG_NOTICE(Required library libpng not found!)
	depend_error=1
fi

if test "$glew_found" = 0; then
	AC_MSG_NOTICE(Required library libGLEW not found!)
	depend_error=1
fi

if test "$gl_found" = 0; then
	AC_MSG_NOTICE(Required library libGL not found!)
	depend_error=1
fi

if test "$glu_found" = 0; then
	AC_MSG_NOTICE(Required library libGLU not found!)
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

if test "$newcegui_found" = 1; then
	AC_MSG_NOTICE(Using CEGUI 0.7)
	AC_DEFINE(HAVE_CEGUI_07, 1, [Set to 1 if you have CEGUI 0.7 or later])
else
	AC_MSG_NOTICE(Not using CEGUI 0.7)
	AC_DEFINE(HAVE_CEGUI_07, 0, [Set to 1 if you have CEGUI 0.7 or later])
fi

if test "x$enable_cgl" = xyes; then
	AC_MSG_NOTICE(Enabled Cluster GL Compatibility)
	AC_DEFINE(ENABLE_CGL_COMPAT, 1, [Set to 1 if Cluster GL enabled])
fi

AC_SUBST([ADD_LIBS])
AC_SUBST([ADD_LDFLAGS])
AC_SUBST([ADD_INCLS])
AC_SUBST([LTLIBOBJS])
AC_SUBST([HAVE_CEGUI_07])

AC_OUTPUT

