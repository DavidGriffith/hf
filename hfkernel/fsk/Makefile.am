INCLUDES = -I$(top_srcdir)/hfkernel \
	-I$(top_srcdir)/hfkernel/fsk \
	-I$(top_srcdir)/hfkernel/l1 \
	-I$(top_srcdir)/hfkernel/mt63 \
	-I$(top_srcdir)/hfkernel/cw

noinst_LIBRARIES = libfsk.a

noinst_PROGRAMS = mkgtortables

noinst_HEADERS = \
	fskl1.h \
	fskutil.h \
	fskinlines.h \
	amtor.h \
	gtor.h \
	pactor.h \
	rtty.h \
	standby.h 

gtor.c: gtor_tables.h 
mkgtortables_SOURCES = mkgtortables.c 
mkgtortables$(EXEEXT): mkgtortables.c 
gtor_tables.h:	mkgtortables$(EXEEXT)
	./mkgtortables$(EXEEXT) > gtor_tables.h

libfsk_a_SOURCES = \
	fskl1.c \
	fskutil.c \
	amtor.c \
	gtor.c \
	pactor.c \
	rtty.c \
	standby.c 
