# RPM spec file for XFCE

Summary:	A Powerfull X Environment, with Toolbar and Window Manager
Name:		xfce
Version:	2.0.1
Release:	4a
Copyright:	Free
Group:		X11/Applications
Source:		xfce-2.0.1.tar.gz
URL:		http://www.linux-kheops.com/pub/xfce, http://tsikora.tiac.net/xfce
Vendor:		Olivier Fourdan <fourdan@csi.com>
Packager:	Gary Thomas <g.thomas@opengroup.org>, Olivier Fourdan <fourdan@csi.com>

%description
XFCE is a set made of a Window Manager (XFwm), a toolbar/desktop (XFce) and
a backdrop manager (XFbd).

%prep
%setup
xmkmf
make Makefiles

%build
make Makefiles
make

%install
rm -fr /var/XFCE
make install
make install.man

%post

%files
%doc COPYRIGHT INSTALL MANUAL NEW NOTE.TO.CONTRIBUTORS README
%doc goodies/sample.* xfce.lsm
/usr/X11R6/lib/X11/app-defaults/XFce-en
/usr/X11R6/lib/X11/app-defaults/XFce-fr
/usr/X11R6/lib/X11/app-defaults/XFce-de
/usr/X11R6/lib/X11/app-defaults/XFce-it
/usr/X11R6/lib/X11/app-defaults/XFce-es
/usr/X11R6/lib/X11/app-defaults/XFce-hu
/usr/X11R6/lib/X11/app-defaults/XFce
/usr/X11R6/man/man1/xfce.1x
/usr/X11R6/man/man1/xfwm.1x
/usr/X11R6/bin/xfbd
/usr/X11R6/bin/xfce
/usr/X11R6/bin/xfwm
/usr/X11R6/bin/xfce_setup
/usr/X11R6/bin/xfce_remove
/usr/X11R6/bin/XFCE
/var/XFCE
