Name:       openrepos-mmslog
Summary:    MMS Logger
Version:    1.0.22
Release:    1
Group:      Applications/System
License:    BSD
Vendor:     slava
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

%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}

%description
Application for gathering MMS troubleshooting information.

%prep
%setup -q -n %{name}-%{version}

%build
%qtc_qmake5 CONFIG+=openrepos CONFIG+=app_settings
%qtc_make %{?_smp_mflags}

%install
%qmake5_install

desktop-file-install --delete-original \
  --dir %{buildroot}%{_datadir}/applications \
   %{buildroot}%{_datadir}/applications/*.desktop

%files
%global privileges_dir %{_datarootdir}/mapplauncherd/privileges.d
%dir %{privileges_dir}
%{privileges_dir}/%{name}
%{_bindir}/%{name}
%{_datadir}/%{name}/qml
%{_datadir}/translations/%{name}*.qm
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
%{_datadir}/jolla-settings/entries/%{name}.json
