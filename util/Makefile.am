INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/proto -I$(top_srcdir)/l1

EXTRA_DIST = reffreq.c

if HAVEOSS
OSS = reffreq
reffreq_SOURCES = reffreq.c
else
OSS = 
endif

bin_PROGRAMS = channel $(OSS)
noinst_PROGRAMS = testgaussgen filtbench pactorcrc chantab amtorsymsort

channel_SOURCES = channel.c
channel_LDADD = $(THRLIBS) $(SOCKLIBS) $(SCHEDLIBS)
testgaussgen_SOURCES = testgaussgen.c
filtbench_SOURCES = filtbench.c
pactorcrc_SOURCES = pactorcrc.c
pactorcrc_LDADD = ../proto/libproto.a
chantab_SOURCES = chantab.c
amtorsymsort_SOURCES = amtorsymsort.c

chan_tables.h:	chantab
		./chantab > chan_tables.h
