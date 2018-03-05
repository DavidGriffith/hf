INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/proto -I$(top_srcdir)/l1

noinst_LIBRARIES = libproto.a

noinst_PROGRAMS = mkgtortables

noinst_HEADERS = \
	amtor.h \
	gtor.h \
	pactor.h \
	rtty.h \
	standby.h \
	util.h

mkgtortables_SOURCES = mkgtortables.c

gtor_tables.h:	mkgtortables
	./mkgtortables > gtor_tables.h

libproto_a_SOURCES = \
	amtor.c \
	gtor.c \
	pactor.c \
	rtty.c \
	standby.c \
	util.c

