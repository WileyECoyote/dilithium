
LIBGCRYPT_CFLAGS=-c `pkg-config --cflags xau` `libgcrypt-config --cflags`
LIBGCRYPT_LIBS= `libgcrypt-config --libs` -lcrypt

AM_CFLAGS=-g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security
AM_CPPFLAGS=-D_FORTIFY_SOURCE=2

INC_LOCAL =  -I$(top_srcdir)/ -I$(top_srcdir)/include

LIB_XLOGIN = xlogin

SUBDIRS = $(LIB_XLOGIN)

bin_PROGRAMS = dilithium

dilithium_SOURCES = \
	common.cc \
	console.cc \
        daemon.cc \
	xauthxx.cc \
	dilithium.cc \
	spawner.cc \
	privileges.cc

dilithium_CPPFLAGS    = $(AM_CPPFLAGS) $(CPPFLAGS) $(INC_LOCAL) $(GCRYPT_CFLAGS)
dilithium_CFLAGS      = $(AM_CFLAGS) $(CFLAGS)

dilithium_LDFLAGS     = "-lX11" `pkg-config --libs xau` $(LIBGCRYPT_LIBS)

dilithium_LDADD	      = $(LIB_XLOGIN)/libxlogin.a

MOSTLYCLEANFILES      = *.log core FILE *~
CLEANFILES            = *.log core FILE *~
DISTCLEANFILES        = *.log core FILE *~
MAINTAINERCLEANFILES  = *.log core FILE *~ Makefile.in

