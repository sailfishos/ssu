Name: ssu
Version: 0.33
Release: 1
Summary: SSU enabler for RND
Group: System/Base
License: GPLv2
Source0: %{name}-%{version}.tar.gz
URL: https://github.com/nemomobile/ssu
BuildRequires: pkgconfig(boardname)
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5DBus)
BuildRequires: pkgconfig(Qt5Network)
BuildRequires: pkgconfig(Qt5Xml)
BuildRequires: pkgconfig(Qt5Test)
BuildRequires: pkgconfig(Qt5SystemInfo)
BuildRequires: pkgconfig(libzypp)
BuildRequires: pkgconfig(libsystemd-journal)
BuildRequires: pkgconfig(libshadowutils)
BuildRequires: pkgconfig(connman-qt5)
BuildRequires: oneshot
BuildRequires: doxygen
Requires(pre): shadow-utils
Requires(pre): /usr/bin/groupadd-user
Requires(postun): shadow-utils
Requires: ssu-vendor-data

%description
%{summary}.

%files
%defattr(-,root,root,-)
%{_libdir}/zypp/plugins/urlresolver/*
%{_bindir}/rndssu
%{_bindir}/ssu
%{_libdir}/*.so.*
%dir %{_sysconfdir}/zypp/credentials.d
# ssu itself does not use the package-update triggers, but provides
# them for the vendor data packages to use
%attr(0755, -, -) %{_oneshotdir}/*
%{_bindir}/ssud
%{_datadir}/dbus-1/system-services/*.service
%{_sysconfdir}/dbus-1/system.d/*.conf

%package vendor-data-example
Summary: Sample vendor configuration data
Group: System/Base
Provides: ssu-vendor-data

%description vendor-data-example
%{summary}. A vendor (including Nemo) is supposed to put those configuration on device.

%files vendor-data-example
%defattr(-,root,root,-)
%attr(0664, root, ssu) %config(noreplace) %{_sysconfdir}/ssu/ssu.ini
%{_datadir}/ssu/*.ini


%package ks
Summary: Kickstart generator using %{name} data
Group: System/Base
# required for QA to pick up new macros
Provides: rpm-macros

%description ks
%{summary}. With ponies!

%files ks
%defattr(-,root,root,-)
%config %{_sysconfdir}/rpm/macros.ssuks
%{_bindir}/ssuks


%package devel
Summary: Development files for %{name}
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}

%description devel
%{summary}.

%files devel
%defattr(-,root,root,-)
%{_libdir}/*.so
%{_includedir}/ssu*.h


%package tests
Summary: Unit tests for %{name}
Group: Development/Libraries
Requires: testrunner-lite

%description tests
%{summary}.

%files tests
%defattr(-,root,root,-)
/opt/tests/%{name}


%package tools
Summary: Tools for %{name}
Group: Development/Libraries
Requires: rpm

%description tools
%{summary}.

%files tools
%defattr(-,root,root,-)
%attr(0755, -, -) /usr/sbin/ssu-repos.sh


%package doc
Summary: Documentation for %{name}
Group: Documentation

%description doc
%{summary}.

%files doc
%defattr(-,root,root,-)
%{_docdir}/%{name}


%prep
%setup -q -n %{name}-%{version}


%build
mkdir -p build && cd build
%qmake5 DEFINES+='TARGET_ARCH=\\\"\"%{_target_cpu}\"\\\"' -recursive ..
make %{?_smp_mflags}
cd .. && doxygen doc/Doxyfile


%install
cd build && make INSTALL_ROOT=%{buildroot} install
mkdir -p %{buildroot}/%{_sysconfdir}/zypp/credentials.d
ln -s %{_bindir}/ssu %{buildroot}/%{_bindir}/rndssu
mkdir -p %{buildroot}/%{_docdir}/%{name}
cd .. && cp -R doc/html/* %{buildroot}/%{_docdir}/%{name}/


%pre
groupadd -rf ssu
groupadd-user ssu
if [ -f /etc/ssu/ssu.ini ]; then
  chgrp ssu /etc/ssu/ssu.ini
  chmod 664 /etc/ssu/ssu.ini
fi

%postun
/sbin/ldconfig
if [ "$1" == 0 ]; then
  getent group ssu >/dev/null && groupdel ssu
fi

%post
/sbin/ldconfig
