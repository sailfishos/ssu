Name: ssu
Version: 0.23
Release: 1
Summary: SSU enabler for RND
Group: System/Base
License: GPLv2
Source0: %{name}-%{version}.tar.gz
URL: https://github.com/nemomobile/ssu
BuildRequires: pkgconfig(QtCore)
BuildRequires: pkgconfig(libzypp)
BuildRequires: pkgconfig(QtSystemInfo)
BuildRequires: pkgconfig(qt-boostable)
BuildRequires: pkgconfig(libsystemd-journal)
Requires(pre): shadow-utils
Requires(postun): shadow-utils
Requires: ssu-vendor-data

%description
%{summary}.

%files
%defattr(-,root,root,-)
%{_libdir}/zypp/plugins/urlresolver/*
%{_bindir}/rndssu
%{_libdir}/*.so.*
%dir %{_sysconfdir}/zypp/credentials.d


%package vendor-data-example
Summary: Sample vendor configuration data
Group: System/Base

%description vendor-data-example
%{summary}. A vendor (including Nemo) is supposed to put those configuration on device.

%files vendor-data-example
%defattr(-,root,root,-)
%attr(0664, root, ssu) %config(noreplace) %{_sysconfdir}/ssu/ssu.ini
%{_datadir}/ssu/*.ini


%package rnd-ui
Summary: Shiny user interface for %{name}
Group: System/Base

%description rnd-ui
%{summary}. With ponies!

%files rnd-ui
%defattr(-,root,root,-)
%{_bindir}/rndregisterui
%{_datadir}/applications/*.desktop


%package devel
Summary: Development files for %{name}
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}

%description devel
%{summary}.

%files devel
%defattr(-,root,root,-)
%{_libdir}/*.so
%{_includedir}/ssu.h


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


%prep
%setup -q -n %{name}-%{version}


%build
qmake DEFINES+='TARGET_ARCH=\\\"\"%{_target_cpu}\"\\\"' -recursive
make %{?_smp_mflags}


%install
make INSTALL_ROOT=%{buildroot} install
mkdir -p %{buildroot}/%{_sysconfdir}/zypp/credentials.d

%pre
groupadd -rf ssu
USER=$(getent passwd 1000 | cut -d: -f1)
if [ -n "$USER" ]; then
  usermod -a -G ssu $USER
fi
if [ -f /etc/ssu/ssu.ini ]; then
  chgrp ssu /etc/ssu/ssu.ini
  chmod 664 /etc/ssu/ssu.ini
fi

%postun
/sbin/ldconfig
if [ "$1" == 0 ]; then
  getent group ssu >/dev/null && groupdel ssu
fi

%post -p /sbin/ldconfig
