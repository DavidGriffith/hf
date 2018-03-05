EXTRA_DIST = freqdev.m ggtest.m golay.m  \
	alsafdx.c alsalatency.c alsatestmake

if HAVEALSA
ALSA = alsafdx alsalatency
alsafdx: alsafdx.c
	gcc alsafdx.c -o alsafdx -lasound
alsalatency: alsalatency.c
	gcc alsalatency.c -o alsalatency -lasound
else
ALSA = 
endif

noinst_PROGRAMS = testcos testgettimeofday testoss testpoly $(ALSA)
bin_PROGRAMS = portecho

testcos_LDADD = -lm