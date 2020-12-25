# RPM spec file for XFCE
%define		Version	2.4.0
%define		Name	xfce

Summary:	A Powerfull X Environment, with Toolbar and Window Manager
Name:		%{Name}
Version:	%{Version}
Release:	1rh5
Copyright:	Free
Group:		X11/Applications
Source:		%{name}-%{version}.tar.gz
URL:		http://www.xfce.org
Vendor:		Olivier Fourdan <fourdan@csi.com>
Packager:	Gary Thomas <g.thomas@opengroup.org>, Olivier Fourdan <fourdan@csi.com>
BuildRoot:	/tmp/%{name}-%{version}-root
Requires:	xfce-common

%description
XFCE is a set made of a Window Manager (XFwm), a toolbar/desktop (XFce),
a backdrop manager (XFbd) a sound manager (xfsound), a pager (xfpager) and
a user friendly interface for mouse configration (xfmouse).

%prep
%setup
xmkmf -a

%build
make

%install
make install 
make install.man

%clean
rm -rf $RPM_BUILD_ROOT

%post

%files
%doc COPYRIGHT INSTALL FAQ MANUAL NEW NOTE.TO.CONTRIBUTORS README xfce.lsm
/usr/X11R6/lib/X11/app-defaults/XFce-en
/usr/X11R6/lib/X11/app-defaults/XFce-fr
/usr/X11R6/lib/X11/app-defaults/XFce-de
/usr/X11R6/lib/X11/app-defaults/XFce-it
/usr/X11R6/lib/X11/app-defaults/XFce-es
/usr/X11R6/lib/X11/app-defaults/XFce-hu
/usr/X11R6/lib/X11/app-defaults/XFce-sk
/usr/X11R6/lib/X11/app-defaults/XFce-pt
/usr/X11R6/lib/X11/app-defaults/XFce
/usr/X11R6/man/man1/xfce.1x
/usr/X11R6/man/man1/xfwm.1x
/usr/X11R6/man/man1/xfsound.1x
/usr/X11R6/man/man1/xfpager.1x
/usr/X11R6/man/man1/xfmouse.1x
/usr/X11R6/bin/xfbd
/usr/X11R6/bin/xfce
/usr/X11R6/bin/xfwm
/usr/X11R6/bin/xfsound
/usr/X11R6/bin/xfpager
/usr/X11R6/bin/xfmouse
/usr/X11R6/bin/xfplay
/usr/X11R6/bin/xfce_setup
/usr/X11R6/bin/xfce_remove
/usr/X11R6/bin/XFCE
/var/XFCE/XFce2.logo
/var/XFCE/doc/INSTALL
/var/XFCE/doc/MANUAL
/var/XFCE/doc/README
/var/XFCE/doc/COPYRIGHT
/var/XFCE/doc/FAQ


%changelog
