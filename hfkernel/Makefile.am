INCLUDES = \
	-I$(top_srcdir)/hfkernel/fsk \
	-I$(top_srcdir)/hfkernel/l1 \
	-I$(top_srcdir)/hfkernel/mt63 \
	-I$(top_srcdir)/hfkernel/cw \
	-I$(top_srcdir) -I.

SUBDIRS = fsk l1 mt63 cw

EXTRA_DIST = vsnprintf.c 

if HAVEALSA
ALSALIBS = -lasound
else
ALSALIBS =
endif

MYLIBS = ./l1/libl1.a ./fsk/libfsk.a ./mt63/libmt63.a ./cw/libcw.a
noinst_HEADERS = main.h msg.h 
bin_PROGRAMS = hfkernel 
hfkernel_SOURCES = main.c msg.c
hfkernel_DEPENDANCIES =	 $(noinst_HEADERS) $(hfkernel_SOURCES) $(MYLIBS)
hfkernel_LDADD = $(MYLIBS) \
	$(OSLIBS) $(SOCKLIBS) $(THRLIBS) $(SCHEDLIBS) $(ALSALIBS) -lstdc++

install-exec-hook:
	chmod 4755 $(DESTDIR)$(bindir)/hfkernel 
	chmod 4755 $(DESTDIR)$(bindir)/elbug