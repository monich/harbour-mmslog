Name:       harbour-mmslog
Summary:    MMS Logger
Version:    1.0.9
Release:    1
Group:      Applications/System
License:    BSD
Vendor:     meego
URL:        http://github.com/monich/harbour-mmslog
Source0:    %{name}-%{version}.tar.bz2

Requires:   sailfishsilica-qt5
BuildRequires: pkgconfig(sailfishapp)
BuildRequires: pkgconfig(Qt5Quick)
BuildRequires: pkgconfig(Qt5Qml)
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5DBus)
BuildRequires: pkgconfig(mlite5)
BuildRequires: pkgconfig(gio-2.0)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(gio-unix-2.0)
BuildRequires: desktop-file-utils
BuildRequires: qt5-qttools-linguist

%description
Application for gathering MMS troubleshooting information.

%{!?qtc_qmake:%define qtc_qmake %qmake}
%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}
%{?qtc_builddir:%define _builddir %qtc_builddir}

%prep
%setup -q -n %{name}-%{version}

%build
%qtc_qmake5
%qtc_make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%qmake5_install

desktop-file-install --delete-original \
  --dir %{buildroot}%{_datadir}/applications \
   %{buildroot}%{_datadir}/applications/*.desktop

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_datadir}/%{name}/qml
%{_datadir}/%{name}/translations
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png

%changelog
* Fri Sep  9 2016 Slava Monich <slava.monich@jolla.com> 1.0.9
- Make font size configurable

* Mon Sep  6 2016 Slava Monich <slava.monich@jolla.com> 1.0.8
- Added app settings and build for openrepos

* Wed Jul 20 2016 Slava Monich <slava.monich@jolla.com> 1.0.7
- Added storage information to the tarball
- Fixed a few minor UI issues

* Sat Apr 30 2016 Slava Monich <slava.monich@jolla.com> 1.0.6
- Subdirectory inside the tarball
- Added 'Add account' button to the list of sharing methods
- Added 'Save to documents' function

* Tue Apr 12 2016 Slava Monich <slava.monich@jolla.com> 1.0.5
- Changed vendor to meego to fix upgrade problems

* Tue Apr 12 2016 Slava Monich <slava.monich@jolla.com> 1.0.4
- Added support for multiple modems
- Added hi-res icons

* Tue Jun 30 2015 Slava Monich <slava.monich@jolla.com> 1.0.3
- Added sailfish-release file to the tarball

* Wed Mar 25 2015 Slava Monich <slava.monich@jolla.com> 1.0.2
- Restart mms-engine if it fails to start

* Thu Jan 22 2015 Slava Monich <slava.monich@jolla.com> 1.0.1
- Added NetworkRegistration information to the tarball
- Limited number of lines in the log view
- Russian translation

* Wed Dec 24 2014 Slava Monich <slava.monich@jolla.com> 1.0.0
- Initial version
