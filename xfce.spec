# RPM spec file for XFce

%define		Version	3.2.1
%define		Name	xfce
%define		Release	1

%define         Buildroot /var/tmp/%{Name}-root
%define		Bindir /usr/X11R6
%define		Datadir /usr/share

Summary:	A Powerfull X Environment, with Toolbar and Window Manager
Name:		%{Name}
Version:	%{Version}
Release:	%{Release}
Buildroot:      %{Buildroot}
Copyright:	GPL
Group:		X11/Window Managers
Source:		%{Name}-%{version}.tar.gz
URL:		http://www.xfce.org
Vendor:		Olivier Fourdan <fourdan@xfce.org>
Packager:       G. Thomas <g.thomas@opengroup.org>,O. Fourdan <fourdan@xfce.org>,P. Reich <reichp@ameritech.net>
Requires:	xscreensaver

%description
XFce is a fast, lightweight desktop
environment for Linux and various Unices...

%prep
%setup
./configure --prefix=%{Bindir} --datadir=%{Datadir} --disable-dt

%build
make

%install
make install DESTDIR=$RPM_BUILD_ROOT
strip -s %{Buildroot}%{Bindir}/bin/xfce
strip -s %{Buildroot}%{Bindir}/bin/xfwm
strip -s %{Buildroot}%{Bindir}/bin/xftree
strip -s %{Buildroot}%{Bindir}/bin/xfsound
strip -s %{Buildroot}%{Bindir}/bin/xfbd
strip -s %{Buildroot}%{Bindir}/bin/xfpager
strip -s %{Buildroot}%{Bindir}/bin/xfgnome
strip -s %{Buildroot}%{Bindir}/bin/xfmouse
strip -s %{Buildroot}%{Bindir}/bin/xfclock

%clean
rm -rf $RPM_BUILD_ROOT

%post
cd %{Datadir}/xfce; chmod 0755 sample.xsession sample.xinitrc sample.Xclients
if [ -d /usr/share/icons ]; then ln -sf /usr/share/icons %{Datadir}/xfce/more-icons; fi
echo
echo CONGRATULATIONS ! You\'ve got XFce setup on your computer !
echo
echo Now execute \"xfce_setup\" to make XFce your default desktop environment,
echo \(even if you\'re upgrading from a previous version, this is also recommended\),
echo and restart your X session to enjoy XFce Desktop environment.
echo
cat /usr/doc/%{Name}-%{Version}/README.UPGRADE-3.2.0
if [ -d /var/XFCE ]; then
  echo Previous /var/XFCE directory found
  if [ -h /var/XFCE ]; then
    echo Saved from previous upgrade !
    echo Nothing to do.
  else
    echo The previous directory /var/XFCE is being renamed as /var/XFCE.old
    mv -f /var/XFCE /var/XFCE.old
    echo and replaced by a symbolic link to /usr/share/xfce
    ln -sf /usr/share/xfce /var/XFCE
    echo That should be enough to keep your previous configurations unchanged.
  fi
fi


%files
%doc README README.UPGRADE-3.2.0 INSTALL ChangeLog
%{Bindir}/bin/xfce
%{Bindir}/bin/xfwm
%{Bindir}/bin/xftree
%{Bindir}/bin/xfsound
%{Bindir}/bin/xfbd
%{Bindir}/bin/xfpager
%{Bindir}/bin/xfgnome
%{Bindir}/bin/xfmouse
%{Bindir}/bin/xfclock
%{Bindir}/bin/xfplay
%{Bindir}/bin/xfhelp
%{Bindir}/bin/xfprint
%{Bindir}/bin/xftrash
%{Bindir}/bin/xfterm
%{Bindir}/bin/xfce_setup
%{Bindir}/bin/xfce_remove
%{Bindir}/bin/startxfce
%{Bindir}/man/man1/xfbd.1
%{Bindir}/man/man1/xfce.1
%{Bindir}/man/man1/xfclock.1
%{Bindir}/man/man1/xfmouse.1
%{Bindir}/man/man1/xfpager.1
%{Bindir}/man/man1/xfgnome.1
%{Bindir}/man/man1/xfsound.1
%{Bindir}/man/man1/xftree.1
%{Bindir}/man/man1/xfwm.1
%{Bindir}/man/man1/xfhelp.1
%{Bindir}/man/man1/xfprint.1
%{Bindir}/man/man1/xftrash.1
%{Bindir}/man/man1/xfterm.1
%{Datadir}/xfce

%preun

%postun
rm -rf %{Datadir}/xfce

%changelog

* Sat Oct 23 1999 P. Reich -- fixed spec file to build and install 
- correctly in BUILD_ROOT.
- Added preun to remove links in xfce-datadir.
- Moved DATADIR to /usr/share
- Added link to "more-icons" in /usr/share/icons. 
