INCLUDES = \
	-I$(top_srcdir)/hfkernel \
	-I$(top_srcdir)/hfkernel/l1 \
	-I$(top_srcdir)/hfkernel/fsk \
	-I$(top_srcdir)/hfkernel/mt63 \
	-I$(top_srcdir)/hfkernel/cw 

FILES = COPYING Readme Makefile *.cc *.c *.h *.dat *.alf *.a
VERSION = 0.5

noinst_LIBRARIES = libmt63.a
bin_PROGRAMS = mt63rx mt63tx mt63trx ratecal1 peakrms morsecod addnoise 

libmt63_a_SOURCES = \
	dsp.cc mt63.cc sound.cc stdinr.cc *.h *.dat mt63hf.cc \
	mt63hflink.c

ratecal1_SOURCES =	ratecal1.cc        dsp.cc sound.cc stdinr.cc
mt63tx_SOURCES = 	mt63tx.cc  mt63.cc dsp.cc sound.cc stdinr.cc
mt63rx_SOURCES =	mt63rx.cc  mt63.cc dsp.cc sound.cc stdinr.cc term.cc
mt63trx_SOURCES = 	mt63trx.cc mt63.cc dsp.cc sound.cc stdinr.cc term.cc

mt63rx_LDADD	= -lncurses
mt63tx_LDADD	= -lncurses
mt63trx_LDADD	= -lncurses
peakrms_LDADD	= -lm
addnoise_LDADD	= -lm
#libmt63_a_LDADD = -lg++

arch: $(FILES) 
	test ! -d mt63lx-$(VERSION) || rm -rf mt63lx-$(VERSION)
	mkdir mt63lx-$(VERSION)
	cp $(FILES) mt63lx-$(VERSION)
	tar zcvvf mt63lx-$(VERSION).tar.gz mt63lx-$(VERSION)
	rm -rf mt63lx-$(VERSION)

clean:
	rm -f *.a *.o $(bin_PROGRAMS)

