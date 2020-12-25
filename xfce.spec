# RPM spec file for XFce
%define		Version	3.0.2
%define		Name	xfce
%define		Datadir	/var
%define		Basedir	/usr/X11R6

Summary:	A Powerfull X Environment, with Toolbar and Window Manager
Name:		%{Name}
Version:	%{Version}
Release:	1
Copyright:	GPL
Group:		X11/Applications
Source:		%{name}-%{version}.tar.gz
URL:		http://www.xfce.org
Vendor:		Olivier Fourdan <fourdan@csi.com>
Packager:	Gary Thomas <g.thomas@opengroup.org>, Olivier Fourdan <fourdan@csi.com>

%description
XFce is a lightweight and powerfull desktop environment for Linux and various UNIX
flavour.

%prep
%setup
./configure -prefix=%{Basedir} --datadir=%{Datadir} --disable-dt

%build
make

%install
make install-strip

%clean
rm -rf $RPM_BUILD_ROOT

%post
echo
echo CONGRATULATIONS ! You\'ve got XFce setup on your computer !
echo
echo Now execute \"xfce_setup\" to make XFce your default desktop environment,
echo \(even if you\'re upgrading from a previous version, this is also recommended\),
echo and restart your X session to enjoy XFce Desktop environment.
echo

%files
%doc README INSTALL ChangeLog
%{Basedir}/bin/xfce
%{Basedir}/bin/xfwm
%{Basedir}/bin/xftree
%{Basedir}/bin/xfsound
%{Basedir}/bin/xfbd
%{Basedir}/bin/xfpager
%{Basedir}/bin/xfmouse
%{Basedir}/bin/xfclock
%{Basedir}/bin/xfplay
%{Basedir}/bin/xfhelp
%{Basedir}/bin/xfce_setup
%{Basedir}/bin/xfce_remove
%{Basedir}/man/man1/xfbd.1
%{Basedir}/man/man1/xfce.1
%{Basedir}/man/man1/xfclock.1
%{Basedir}/man/man1/xfmouse.1
%{Basedir}/man/man1/xfpager.1
%{Basedir}/man/man1/xfsound.1
%{Basedir}/man/man1/xftree.1
%{Basedir}/man/man1/xfwm.1
%{Datadir}/XFCE

%changelog
