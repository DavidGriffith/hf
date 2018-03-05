man_MANS = dcf77gen.1 dcf77rx.1

EXTRA_DIST = $(man_MANS)

noinst_HEADERS = dcf77.h xgintf.h
noinst_LIBRARIES = libdcf.a
noinst_PROGRAMS = mktables
bin_PROGRAMS = dcf77rx dcf77gen

libdcf_a_SOURCES = \
	util.c \
	xgintf.c \
	audioin.c \
	audioout.c \
	calccorr.c \
	timedec.c \
	timeenc.c \
	dcfdemod.c \
	dcfdemodpn.c \
	dcfmod.c \
	hbgdemod.c \
	hbgmod.c

libdcf_a_LIBADD = costab.o pntab.o

mktables_SOURCES = mktables.c
mktables_LDADD = -lm

dcf77rx_SOURCES = dcf77rx.c
dcf77rx_LDADD = libdcf.a

dcf77gen_SOURCES = dcf77gen.c
dcf77gen_LDADD = libdcf.a

costab.c:	mktables$(EXEEXT)
	./mktables$(EXEEXT) 1 > costab.c

pntab.c:	mktables$(EXEEXT)
	./mktables$(EXEEXT) 2 > pntab.c
