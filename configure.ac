#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])

AC_INIT(dilithium, 1.0.0)

AC_CONFIG_SRCDIR([version.h.in])
AC_CONFIG_AUX_DIR([build-tools])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

# Honor aclocal flags
ACLOCAL="$ACLOCAL $ACLOCAL_FLAGS"

AM_INIT_AUTOMAKE([nostdinc -Woverride -Wsyntax -Wunsupported])

AC_GNU_SOURCE # This enable extensions on GNU/Linux platforms

#####################################################################
# Do version number magic
#####################################################################

#AX_GIT_VERSION([20130623])

# This is used for keeping the ChangeLog files up-to-date
#AC_SUBST([CHANGELOG_BASE], [1.0.0-20130623])


#####################################################################
# Check for compiling & Program tools
#####################################################################

# Checks for programs.
AC_PROG_AWK
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_MAKE_SET
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MKDIR_P
AX_PROG_GROFF

#####################################################################
# Setup Linker flags...
#####################################################################
  AC_PROG_RANLIB
  #AC_DISABLE_STATIC
  AC_PROG_LIBTOOL

  # Have linker produce read-only relocations, if it knows how
  AC_MSG_CHECKING([linker tolerates -z relro])
  dilithium_LDFLAGS="$LDFLAGS"
  LDFLAGS="-Wl,-z,relro -ljpeg -lXrender -lGL -lGLU $LDFLAGS"

#####################################################################
# If automake 1.11 shave the output to look nice

#m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

#####################################################################
# Check for Standard Header files
#####################################################################

AC_PATH_X
AC_CHECK_HEADERS([fcntl.h inttypes.h paths.h shadow.h stdlib.h string.h])
AC_CHECK_HEADERS([sys/file.h sys/ioctl.h sys/time.h syslog.h unistd.h utmp.h])

#####################################################################
# Checks for typedefs, structures, and compiler characteristics.
#####################################################################

AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_UID_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

#####################################################################
# Checks for library & library functions.
#####################################################################

# This Macro checks libgcrypto and sets compiler and linker flags
AX_LIB_CRYPTO
AC_CHECK_LIB([crypt], [crypt])

# More Generic Library functions
AC_FUNC_CHOWN
AC_FUNC_FORK
AC_FUNC_GETGROUPS
AC_FUNC_MALLOC

# Checks for library functions.
AC_CHECK_FUNCS([alarm atexit dup2 endpwent gethostname getspnam memset])
AC_CHECK_FUNCS([setenv strerror sysinfo])

#####################################################################
# Optional things
#####################################################################

AX_GETPW_R_POSIX

# API documentation
AX_OPTION_DOXYGEN

#####################################################################
# Generate output
#####################################################################

AC_CONFIG_FILES([Makefile
                 docs/Makefile
                 etc/Makefile
                 data/Makefile
                 src/Makefile
                 src/xlogin/Makefile
])

AC_OUTPUT
echo "
Configuration:      ${PACKAGE} Ver ${VERSION} ${DATE_VERSION}
                 -------------------------------------
                        Compiler:     ${CC}
                        CPPFLAGS:     ${CPPFLAGS}
                          CFLAGS:     ${CFLAGS}
                         LDFLAGS:     ${LDFLAGS}

                     Destination:     ${prefix}
"
