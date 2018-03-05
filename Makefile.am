SUBDIRS = scripts hfkernel hfterm dcf77 doc test util
EXTRA_DIST = depcomp hf.spec

install-exec-hook:
	echo -e "\033[0G\033[32mhf installation done! Start me with 'hf'!\033[m"

rpm: $(PACKAGE)-$(VERSION).tar.gz hf.spec 
	cp -v $(PACKAGE)-$(VERSION).tar.gz /usr/src/packages/SOURCES
	rpmbuild -ba hf.spec
