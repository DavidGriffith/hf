INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/os -I$(top_srcdir)/l1 -I$(top_srcdir)/proto

noinst_HEADERS = \
	os_linux.h

EXTRA_DIST = vsnprintf.c

if HAVEOSS

OSS = hfkernel_user hfkernel_kernel

else

OSS = 

endif

bin_PROGRAMS = $(OSS) hfkernel_link

hfkernel_link_SOURCES = main.c msg.c
hfkernel_link_LDADD = ../l1/user/libl1linked.a ../proto/libproto.a $(OSLIBS) $(SOCKLIBS) $(THRLIBS) $(SCHEDLIBS)

hfkernel_user_SOURCES = main.c msg.c
hfkernel_user_LDADD = ../l1/user/libl1user.a ../proto/libproto.a $(OSLIBS) $(SOCKLIBS) $(THRLIBS) $(SCHEDLIBS)

hfkernel_kernel_SOURCES = main.c msg.c
hfkernel_kernel_LDADD = ../l1/kernel/libl1kernel.a ../proto/libproto.a $(OSLIBS) $(SOCKLIBS) $(THRLIBS) $(SCHEDLIBS)

hfkernel_link_DEPENDENCIES = $(OSLIBS)
hfkernel_kernel_DEPENDENCIES = $(OSLIBS)
hfkernel_user_DEPENDENCIES = $(OSLIBS)
