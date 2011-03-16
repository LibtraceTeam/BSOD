AC_INIT([bsod-server],[2.0.1],[contact@wand.net.nz])

AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([bsod_server.cc])
AM_CONFIG_HEADER(config.h)

# Make sure we use the relatively silent automake output
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_PREFIX_DEFAULT(/usr/local/bsod)

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

#AM_PROG_LEX
#AC_PROG_YACC

AC_LANG(C++)
AC_PROG_LIBTOOL

build_lpicolour=1

AC_CONFIG_FILES([Makefile images/Makefile plugins/Makefile 
		plugins/direction/Makefile plugins/position/Makefile
		plugins/colour/Makefile])

AC_SEARCH_LIBS([trace_read_packet], [trace],,libtrace_found=0)
AC_SEARCH_LIBS([cfg_init],[confuse],,conf_found=0)
AC_SEARCH_LIBS([wand_event_init],[wandevent],,event_found=0)
AC_SEARCH_LIBS([lpi_init_library],[protoident],,lpi_found=0)

depend_error=0
if test "$libtrace_found" = 0; then
	AC_MSG_NOTICE(Required library libtrace not found!)
	depend_error=1
fi

if test "$conf_found" = 0; then
	AC_MSG_NOTICE(Required library libconfuse not found!)
	depend_error=1
fi

if test "$event_found" = 0; then
	AC_MSG_NOTICE(Required library libwandevent not found!)
	depend_error=1
fi

if test "$lpi_found" = 0; then
	AC_MSG_NOTICE(Libprotoident not found - will not build lpicolour module)
	build_lpicolour=0
fi


if test "$depend_error" = 1; then
	AC_MSG_ERROR(Critical libraries are missing - please install the missing libraries listed above and rerun ./configure)
fi

AM_CONDITIONAL([BUILD_LPI_COLOUR], [test "$build_lpicolour" = 1])

AC_SUBST([ADD_LIBS])
AC_SUBST([ADD_LDFLAGS])
AC_SUBST([ADD_INCLS])
AC_SUBST([LTLIBOBJS])

AC_OUTPUT

