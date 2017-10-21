
#install directory
#DOCDIR=/usr/share/man/man1/

EXTRA_DIST = $(html_man_files) what-is-dilithium.html server_options.txt crystal.png

all:
	@echo 'Type: make doxygen to create doxygen documentation for dilithium'

doxygen:dilithium.dox
	@echo 'Creating doxygen documentation for dilithium...'

if ENABLE_DOXYGEN
	$(DOXYGEN) dilithium.dox
else
	@echo 'configure was not run with the --enable-doxygen switch.'
	@echo 'The documentation cannot be built from the sources.'
endif

dist_man_MANS = dilithium.1

.1.in.1:
	@touch $@
	d=`LANG=C date -d $(DATE_VERSION) "+%B %-d, %Y"` ; \
	v="$(DOTTED_VERSION).$(DATE_VERSION)" ; \
	sed -e"s:[@]DATE@:$$d:" -e"s:[@]VERSION@:$$v:" < $< > $@

# Rules for converting man->html
html_man_files = $(dist_man_MANS:%.1=%.html)
SUFFIXES = .1.in .1 .html .dox

docsmandir=$(docdir)/man
docsman_DATA=$(html_man_files)

if ENABLE_GROFF_HTML
.1.html:
	rm -rf $@.tmp && \
	LC_NUMERIC=C $(GROFF) -man -T html  $< >$@.tmp && \
	mv -f $@.tmp $@
else
.1.html:
	@echo "No working groff available: cannot create $@"
	@! :
endif


MOSTLYCLEANFILES     = *.log core FILE *~
CLEANFILES           = *.log core FILE *~
DISTCLEANFILES       = *.log core FILE *~
MAINTAINERCLEANFILES = *.log core FILE *~ Makefile.in


