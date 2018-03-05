INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/l1 -I$(top_srcdir)/l1/user

if HAVEOSS

OSS = libl1user.a

else

OSS = 

endif

noinst_LIBRARIES = $(OSS) libl1linked.a

libl1linked_a_SOURCES = \
	fsk.c \
	link.c

libl1user_a_SOURCES = \
	fsk.c \
	oss.c \
	refclock.c

noinst_HEADERS = \
	l1user.h \
	util.h

EXTRA_DIST = oss.c refclock.c
