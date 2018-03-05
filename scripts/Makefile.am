configdir = /usr/local/etc
scripts = hf hftcp ratetry 
configfiles = calibrations

EXTRA_DIST = $(configfiles) $(scripts) testtest htmlup

install:
	$(mkinstalldirs) $(DESTDIR)$(configdir)
	$(INSTALL_DATA) $(configfiles) $(DESTDIR)$(configdir)
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	$(INSTALL) $(scripts) $(DESTDIR)$(bindir)
