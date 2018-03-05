#!/usr/bin/perl -wT

$ENV{PATH}="/usr/bin;/bin";
system "/sbin/rmmod hfmodem";
system "/sbin/insmod sound";
system "/sbin/rmmod sound";
system "/sbin/modprobe hfmodem";
system "/home/sailer/src/pactor/hfkernel_kernel -c /var/run/hfapp";
system "/sbin/rmmod hfmodem";
