
#install directory
install_dir=/usr/share/fvwm-crystal/fvwm/wallpapers

install-data-local:all
	cp -u fvwm-crystal-bright2-1024x768.jpg $(install_dir)/
	cp -u fvwm-crystal-dark2-1024x768.jpg   $(install_dir)/
	cp -u dark-login-366x200.jpg $(install_dir)/

uninstall-local:
	rm -f $(install_dir)/fvwm-crystal-bright2-1024x768.jpg
	rm -f $(install_dir)/fvwm-crystal-dark2-1024x768.jpg
	rm -f $(install_dir)/fvwm-crystal-dark2-1024x768.jpg

MOSTLYCLEANFILES     = *.log core FILE *~
CLEANFILES           = *.log core FILE *~
DISTCLEANFILES       = *.log core FILE *~
MAINTAINERCLEANFILES = *.log core FILE *~ Makefile.in

