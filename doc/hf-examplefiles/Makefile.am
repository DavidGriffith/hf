EXAMPLEFILES = ./*

EXTRA_DIST = \
$(EXAMPLEFILES)

exampledir = $(datadir)/hf/hf-examplefiles

install-data-local:
	@$(NORMAL_INSTALL)

install:
	$(mkinstalldirs) $(DESTDIR)$(exampledir);
	$(INSTALL_DATA) $(EXAMPLEFILES) $(DESTDIR)$(exampledir);

