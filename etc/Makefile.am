
#install directory
sysconfdir=/etc

install-data-local:all
	cp -u dilithium.def           $(sysconfdir)/default/dilithium
	cp -u dilithium-deb-rc        $(sysconfdir)/init.d/dilithium
	cp -u cyrstal-display-manager $(sysconfdir)/X11/

uninstall-local:
	rm -f $(sysconfdir)/init.d/dilithium
	rm -f $(sysconfdir)/default/dilithium

MOSTLYCLEANFILES     = *.log core FILE *~
CLEANFILES           = *.log core FILE *~
DISTCLEANFILES       = *.log core FILE *~
MAINTAINERCLEANFILES = *.log core FILE *~ Makefile.in

