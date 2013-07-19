## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS = 1.7 subdir-objects

VPATH = $(srcdir)

INC_LOCAL =  -I$(top_srcdir)/ -I$(top_srcdir)/include

LIBGCRYPT_LIBS= `libgcrypt-config --libs` -lcrypt
LIBJPEG_LIB = -ljpeg

AM_LDFLAGS = -version-info $(XLOGIN_SO_VERSION)    \
	     -export-dynamic                       \
	     $(no_undefined)                       \
	     -module

noinst_LIBRARIES = libxlogin.a

libxlogin_a_SOURCES = xjpeg.cc libxlogin.cc

libxlogin_a_CPPFLAGS = $(INC_LOCAL) -gtoggle

EXTRA_LIBRARIES = libxlogin.a

#libxlogin_a_LDFLAGS  =  $(LIBGCRYPT_LIBS) $(LIBJPEG_LIB)

MOSTLYCLEANFILES     = *.log core FILE *~
CLEANFILES           = *.log core FILE *~
DISTCLEANFILES       = *.log core FILE *~
MAINTAINERCLEANFILES = *.log core FILE *~ Makefile.in

# Unfortunately, in order to test libtool thoroughly, we need access
# to its private directory.
objdir = `sed -n -e 's/^objdir=\"\(.*\)\"$$/\1/p' ./libtool`

