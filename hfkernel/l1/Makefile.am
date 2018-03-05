INCLUDES = -I$(top_srcdir)/hfkernel \
	-I$(top_srcdir)/hfkernel/l1 \
	-I$(top_srcdir)/hfkernel/fsk \
	-I$(top_srcdir)/hfkernel/mt63 \
	-I$(top_srcdir)/hfkernel/cw

EXTRA_DIST = alsa.c alsa.h 
noinst_HEADERS = oss.h alsa.h
noinst_LIBRARIES = $(OSS) 

if HAVEOSS
OSS = libl1.a
else
OSS = 
endif

if HAVEALSA
ALSA = alsa.c
ALSALIBS = -lasound
else
ALSALIBS =
ALSA = 
endif

libl1_a_SOURCES = \
	refclock.c \
	oss.c \
	$(ALSA)

