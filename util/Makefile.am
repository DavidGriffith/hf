INCLUDES = -I$(top_srcdir)/hfkernel \
	-I$(top_srcdir)/hfkernel/l1 \
	-I$(top_srcdir)/hfkernel/fsk \
	-I$(top_srcdir)/hfkernel/mt63

EXTRA_DIST = reffreq.c

if HAVEOSS
OSS = reffreq
reffreq_SOURCES = reffreq.c
reffreq_LDADD	= -lm
else
OSS = 
endif

bin_PROGRAMS = paccalc channel $(OSS)
noinst_PROGRAMS = testgaussgen filtbench chantab amtorsymsort pactorcrc
paccalc_SOURCES = paccalc.c
channel_SOURCES = channel.c
channel_LDADD = $(THRLIBS) $(SOCKLIBS) $(SCHEDLIBS) -lm
testgaussgen_SOURCES = testgaussgen.c
filtbench_SOURCES = filtbench.c
filtbench_LDADD	= -lm
pactorcrc_SOURCES = pactorcrc.c
pactorcrc_LDADD = ../hfkernel/fsk/libfsk.a
chantab_SOURCES = chantab.c
chantab_LDADD 	= -lm
amtorsymsort_SOURCES = amtorsymsort.c
paccalc_LDADD 	= -lm

channel.c: chan_tables.h

chan_tables.h:	chantab
	./chantab > chan_tables.h

