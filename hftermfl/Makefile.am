INCLUDES = -I$(top_srcdir)/include

EXTRA_DIST = hfterm.c hfterm.fd main.c msg.c

noinst_HEADERS = hft.h hfterm.h

if HAVEFORMS

bin_PROGRAMS = hfterm

hfterm_SOURCES = hfterm.c main.c msg.c
hfterm_LDADD = -lforms

endif
