SUBDIRS = man hf-examplefiles

DOCUMENTS = \
DE-HF-HOWTO.txt HF-HOWTO.txt \
AUTHORS \
LIESMICH README \
INSTALL ChangeLog \
chan_formulas.tex chan_formulas.ps \
dcf77.txt \
pactor.ps pactor.txt pactor_statediag.fig \
P-MB-list.txt \
mt63.txt \
english.txt \
lfconv.jpg 

docdir = $(datadir)/hf
doc_DATA = $(DOCUMENTS)

EXTRA_DIST = $(DOCUMENTS)

susedocdir = /usr/share/doc/packages
install-data-local:
	@$(NORMAL_INSTALL)

	if test -d $(DESTDIR)$(susedocdir) ; then \
	    $(mkinstalldirs) $(DESTDIR)$(susedocdir)/hf; \
	    $(INSTALL_DATA) $(DOCUMENTS) $(DESTDIR)$(susedocdir)/hf; \
	fi 
