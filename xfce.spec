%define 	name	xfce
%define 	version	3.4.4
%define		release	1
%define		prefix 	/usr/X11R6
%define		datadir /usr/share
%define		confdir /etc/X11

Summary:	A Powerfull X Environment, with Toolbar and Window Manager
Name:		%{name}
Version: 	%{version}
Release: 	%{release}
URL: 		http://www.xfce.org
Copyright:	GPL
Group: 		User Interface/Desktops
Source:		http://www.xfce.org/archive/%{name}-%{version}.tar.gz
Buildroot: 	/var/tmp/%{name}-root
Requires: 	xscreensaver, gtk+ >= 1.2.3

%description
XFce is a fast, lightweight desktop
environment for Linux and various Unices...

%prep
%setup -q -n %{name}-%{version}

%build
./configure --prefix=%{prefix} --datadir=%{datadir} --sysconfdir=%{confdir} --disable-dt
make

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/share/icons
mkdir -p $RPM_BUILD_ROOT/var

(
cd $RPM_BUILD_ROOT%{datadir}/xfce
ln -sf ../icons more-icons
cd $RPM_BUILD_ROOT/var
ln -sf ..%{datadir}/xfce XFCE
)

chmod 0755 $RPM_BUILD_ROOT%{confdir}/xfce/{xsession,xinitrc,xinitrc.mwm,Xclients}

strip -s $RPM_BUILD_ROOT%{prefix}/bin/* || :
gzip $RPM_BUILD_ROOT%{prefix}/man/man1/* || :

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc INSTALL ChangeLog AUTHORS COPYING README.UPGRADE-3.2.0
%{prefix}/bin/*
%{datadir}/xfce/*
%{prefix}/man/man1/*
%{confdir}/xfce/*

%changelog
* Mon Apr 17 2000 Olivier Fourdan <fourdan@xfce.org>
- Update with FHS compliancy
* Thu Dec 16 1999 Tim Powers <timp@redhat.com>
- updated to 3.2.2
- general cleanups to make it a bit easier to maintain
- configure to build section
- quiet setup
- no more %pre/preun or %post/postun sections, not needed (done in %install instead)
- no more posinstall messages being displayed
- cleaned up %files section
- gzip man pages

* Sat Oct 23 1999 P. Reich -- fixed spec file to build and install 
- correctly in BUILD_ROOT.
- Added preun to remove links in xfce-datadir.
- Moved DATADIR to /usr/share
- Added link to "more-icons" in /usr/share/icons. 
