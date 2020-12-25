# RPM spec file for XFCE-DATA

Summary:	A Powerfull X Environment, with Toolbar and Window Manager
Name:		xfce-common
Version:	2.4.0
Release:	1
Copyright:	Free
Group:		X11/Applications
Source:		xfce-common-2.4.0.tar.gz
URL:		http://www.xfce.org
Vendor:		Olivier Fourdan <fourdan@csi.com>
Packager:	Gary Thomas <g.thomas@opengroup.org>, Olivier Fourdan <fourdan@csi.com>
Provides:       xfce-common

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
rm -fr /var/XFCE
make install; 
make install.man

%post

%files
/var/XFCE
