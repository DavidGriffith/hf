#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(hf, 0.8, safaridoktor@compuserve.de)
AM_INIT_AUTOMAKE(hf, 0.8) 
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([asound], [set_hwparams])
AC_CHECK_LIB([ncurses], [initscr])

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h libintl.h limits.h netinet/in.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h syslog.h termios.h unistd.h])
AC_CHECK_HEADERS(getopt.h sys/ioctl.h syslog.h errno.h sys/io.h)
AC_CHECK_HEADERS(sys/audioio.h stropts.h sys/conf.h sys/soundcard.h)
AC_CHECK_HEADERS(alsa/asoundlib.h)
AC_CHECK_HEADERS(forms.h pthread.h sched.h)
AC_CHECK_HEADERS(sys/kd.h sys/vtkd.h sys/kbio.h)

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_HEADER_STDBOOL

AC_MSG_CHECKING(for bittypes)
bittypes=no
AC_TRY_COMPILE([#include <sys/types.h>], 
     [ int8_t a; u_int8_t b; int16_t c; u_int16_t d; int32_t e; u_int32_t f; ],
     AC_DEFINE(HAVE_BITTYPES) bittypes=yes)
AC_MSG_RESULT($bittypes)

if test x${bittypes} = xyes; then
    AC_DEFINE([HAVE_BITTYPES], [],[bittypes=yes])
fi

# there seems to be a new syntax of ac_define
# as told in the error given by autoheader
# AC_DEFINE([HAVE_BITTYPES], [],[bittypes=yes])

# Checks for library functions.
AC_PROG_RANLIB
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MKTIME
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([floor getpagesize gettimeofday memchr memmove memset munmap pow select socket sqrt strcasecmp strdup strerror strstr strtol strtoul])

AM_CONDITIONAL(HAVEOSS, test x${ac_cv_header_sys_soundcard_h} = xyes)
AM_CONDITIONAL(HAVEALSA, test x${ac_cv_header_alsa_asoundlib_h} = xyes)

AM_PATH_GTK([1.2.0],[],
            [AC_MSG_ERROR(Cannot find GTK: Is gtk-config in path?)])

# Checks for directories, needed needed for hfterm's help and example fixtexts
# Note: this will be wrong if you give configure an option. Then hfterm 
# will not find the examplefiles. I could not find a solution yet.
AC_DEFINE_UNQUOTED(PREFIX, "$ac_default_prefix", [/usr/bin or /usr/local/bin])
   
AC_CONFIG_FILES([Makefile
                 dcf77/Makefile
                 doc/Makefile
                 doc/hf-examplefiles/Makefile
                 doc/man/Makefile
                 doc/man/de/Makefile
                 hfkernel/Makefile
                 hfkernel/fsk/Makefile
                 hfkernel/l1/Makefile
                 hfkernel/mt63/Makefile
                 hfkernel/cw/Makefile
                 hfterm/Makefile
                 hfterm/src/Makefile
                 scripts/Makefile
                 test/Makefile
                 util/Makefile])
AC_OUTPUT
