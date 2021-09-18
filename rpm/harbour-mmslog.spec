Name:       harbour-mmslog
Summary:    MMS Logger
Version:    1.0.19
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

%{!?qtc_qmake:%define qtc_qmake %qmake}
%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}
%{?qtc_builddir:%define _builddir %qtc_builddir}

%description
Application for gathering MMS troubleshooting information.

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
%global privileges_dir %{_datarootdir}/mapplauncherd/privileges.d
%dir %{privileges_dir}
%{privileges_dir}/%{name}
%{_bindir}/%{name}
%{_datadir}/%{name}/qml
%{_datadir}/%{name}/translations
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png

%changelog
* Sun Aug 15 2021 Slava Monich <slava.monich@jolla.com> 1.0.19
- Copy log text to clipboard on long tap

* Thu Aug  5 2021 Slava Monich <slava.monich@jolla.com> 1.0.18
- Disabled in-app sharing for Sailfish OS >= 4.2
- Tweaked setting page to support latest jolla-settings features
- Freshened up app icon

* Sat Dec 19 2020 Slava Monich <slava.monich@jolla.com> 1.0.17
- Resolved another issue with killing mms-engine on latest Sailfish OS
- Fixed E-mail sharing on latest Sailfish OS

* Sat May 25 2019 Slava Monich <slava.monich@jolla.com> 1.0.16
- Fixed a problem with killing mms-engine in Sailfish OS 3.0.3

* Mon Jun 18 2018 Slava Monich <slava.monich@jolla.com> 1.0.15
- Freshened up the list of sharing methods
- Make the log files owned by nemo:nemo
- Added Dutch translations

* Wed Nov  1 2017 Slava Monich <slava.monich@jolla.com> 1.0.14
- Run mms logger as nemo:privileged

* Sat Feb 11 2017 Slava Monich <slava.monich@jolla.com> 1.0.13
- Added settings page to the harbour app

* Sat Jan 28 2017 Slava Monich <slava.monich@jolla.com> 1.0.12
- Fixed a bug in ofono properties dump

* Tue Jan 24 2017 Slava Monich <slava.monich@jolla.com> 1.0.11
- Added Swedish translations

* Mon Oct 17 2016 Slava Monich <slava.monich@jolla.com> 1.0.10
- Save ofono logs if they are available

* Fri Sep  9 2016 Slava Monich <slava.monich@jolla.com> 1.0.9
- Make font size configurable

* Tue Sep  6 2016 Slava Monich <slava.monich@jolla.com> 1.0.8
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
