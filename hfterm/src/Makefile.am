## Process this file with automake to produce Makefile.in

INCLUDES = \
	-I$(top_srcdir)/hfkernel \
	-I$(top_srcdir)/hfkernel/l1 \
	-I$(top_srcdir)/hfkernel/fsk \
	-I$(top_srcdir)/hfkernel/mt63 \
	-I$(top_srcdir)/hfkernel/cw \
	-I$(top_srcdir)/intl \
	@GTK_CFLAGS@

bin_PROGRAMS = hfterm

hfterm_SOURCES = \
	callbacks.c \
	logcallbacks.c \
	main.c \
	log.c \
	support.c \
	spectrum.c \
	msg.c \
	maingui.c \
	specgui.c \
	pargui.c \
	monigui.c \
	fixgui.c \
	loggui.c \
	mapgui.c \
	helpgui.c \
	params.c \
	rxtx.c \
	mailbox.c

noinst_HEADERS = \
	support.h \
	gui.h \
	callbacks.h \
	spectrum.h \
	hft.h \
	log.h \
	rxtx.h \
	mailbox.h

EXTRA_DIST = vsnprintf.c

#dnl hfterm_LDADD = @GTK_LIBS@ $(INTLLIBS)
hfterm_LDADD = @GTK_LIBS@ $(OSLIBS)

hfterm_DEPENDENCIES = $(OSLIBS) $(noinst_HEADERS)

