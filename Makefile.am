SUBDIRS = src docs etc data

ACLOCAL_AMFLAGS = -I m4

# We need to ensure that `make distcheck' *doesn't* find a git
# repository!

DISTCHECK_CONFIGURE_FLAGS = \
		GIT="$(false)" 

BUILT_SOURCES = version.h

EXTRA_DIST =    COPYING \
		autogen.sh \
		config.h \
		version.h version.h.in

doxygen:

# call 'make doxygen' for dilithium
if ENABLE_DOXYGEN
	(cd docs; $(MAKE) doxygen) || exit 1;
else
	@echo 'configure was not run with the --enable-doxygen switch.'
	@echo 'The documentation cannot be built from the sources.'
endif
