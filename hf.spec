# hf.spec
#
%define name1	hf
%define name    hf
%define version 0.8
%define release 0
%define section	Applications/Communications
%define title	hf
%define summary "Historic Ham Radio digimodes, MT63, CW-Elbug"
%define bindir  /usr/local/bin
%define mandir  /usr/local/man

Summary:        %{summary}
Name:           %{name}
Version:        %{version}
Release:        %{release}
License:        GPL
Group:          Communications
URL:            http://hfterm.sf.net
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-buildroot
Requires:       gtk 

%description 
Ham radio Shortwave CW-Elbug RTTY AMTOR GTOR PACTOR1 MT63 program. 
With TCP/IP interface, can be linked to the program F6FBB
or to any other mailbox program.
Can be used as automatic mailbox or for remote control.
Contains 3 calibration tools: 
dxf77rx, which uses the signals of the European longwave radio clock beacon DCF77, 
reffreq, which can use any Audio reference frequency from 20-22000 Hz, 
ratecal1, which utilizes any pulsed signal. 

%prep
%setup -q

%build
./configure 
make

%install
make  DESTDIR=%buildroot install
chmod 4755 %buildroot%{bindir}/hfkernel

%clean
rm -rf %buildroot

%files
/usr/local/share/hf/*
/usr/local/etc/calibrations
%{bindir}/*
%{mandir}/* 

## maybe for SuSE you have to uncomment the following line
# /usr/share/doc/packages/hf

## maybe for some other distros the following is right
#I am a bit confused
# this is /usr/bin etc.
#%_bindir/*
#%_mandir/* 

%config

%changelog
* Thu Feb 15 2005 see ChangeLog in the package
- much ....