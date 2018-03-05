ifeq (/usr/src/linux/.config,$(wildcard /usr/src/linux/.config))
include /usr/src/linux/.config
endif

KCFLAGS	= -D__KERNEL__ -I/usr/src/linux/include -Wall -Wstrict-prototypes -O2 -fomit-frame-pointer 
KCFLAGS	:= $(KCFLAGS) -fno-strength-reduce -m486 -malign-loops=2 -malign-jumps=2 -malign-functions=2
KCFLAGS	:= $(KCFLAGS) -DCONFIG_HFMODEM_WSS -DCONFIG_HFMODEM_SBC # -DCONFIG_HFMODEM_FLOAT
ifeq ($(CONFIG_M586),y)
KCFLAGS	:= $(KCFLAGS) -DCPU=586
endif
ifeq ($(CONFIG_M486),y)
KCFLAGS	:= $(KCFLAGS) -DCPU=486
endif
ifeq ($(CONFIG_M386),y)
KCFLAGS	:= $(KCFLAGS) -DCPU=386
endif
ifeq ($(CONFIG_MODULES),y)
KCFLAGS	:= $(KCFLAGS) -DMODULE -DEXPORT_SYMTAB
ifeq ($(CONFIG_MODVERSIONS),y)
KCFLAGS	:= $(KCFLAGS) -DMODVERSIONS -include /usr/src/linux/include/linux/modversions.h
endif
endif

SRC		=refclock.c sbc.c wss.c modem.c main.c

all:		hfmodem.o

hfmodem.o:	$(SRC:.c=.o)
		$(RM) -f $@
		$(LD) -m elf_i386  -r -o $@ $^

%.o:		%.c
		$(CC) $(KCFLAGS) -o $@ -c $<

%.s:		%.c
		$(CC) $(KCFLAGS) -o $@ -S $<

gentbl:		gentbl.c
		$(CC) -Wall -Wstrict-prototypes -O2 -o $@ $< -lm

tables.h:	gentbl
		./gentbl > $@

clean:
		$(RM) -f core *.o *~ .depend gentbl tables.h

depend dep:	tables.h
		$(CPP) -M $(CFLAGS) $(SRC) > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
