## Process this file with automake to produce Makefile.in

INCLUDES = \
	-I$(top_srcdir)/../include \
	-I$(top_srcdir)/intl \
	@GTK_CFLAGS@

bin_PROGRAMS = hfterm

hfterm_SOURCES = \
	main.c \
	support.c \
	interface.c \
	spectrum.c \
	msg.c

noinst_HEADERS = \
	support.h \
	interface.h \
	callbacks.h \
	spectrum.h \
	hft.h

EXTRA_DIST = vsnprintf.c

#dnl hfterm_LDADD = @GTK_LIBS@ $(INTLLIBS)
hfterm_LDADD = @GTK_LIBS@ $(OSLIBS)

hfterm_DEPENDENCIES = $(OSLIBS)
