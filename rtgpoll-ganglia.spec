%define dist .gu
Name:           rtgpoll-ganglia
Version:        1.0
Release:        1%{?dist}
Summary:        SNMP Poller for Ganglia
Group:          System Environment/Base
License:        GNU GPL
URL:            https://github.com/satterly/rtgpoll-ganglia
Source0: 	%{name}-%{version}.tar.gz
BuildRoot:      %{_builddir}/%{name}-%{version}-rpmroot
Requires:	net-snmp-utils
BuildRequires:  net-snmp-devel
Requires:	libganglia >= 3.1.7
BuildRequires:  ganglia-devel >= 3.1.7

%description
RTG SNMP poller with Ganglia integration.

%prep
%setup

%build
autoconf
./configure --prefix=/usr
make

%install
make install DESTDIR=$RPM_BUILD_ROOT
install -dm 755 $RPM_BUILD_ROOT/etc/ganglia/rtgpoll/
install -pm 0644 %{_builddir}/%{name}-%{version}/rtg.conf $RPM_BUILD_ROOT/etc/ganglia/rtgpoll/
install -pm 0644 %{_builddir}/%{name}-%{version}/target.conf $RPM_BUILD_ROOT/etc/ganglia/rtgpoll/
install -dm 755 $RPM_BUILD_ROOT/etc/init.d/
install -pm 0755 %{_builddir}/%{name}-%{version}/rtgpoll-ganglia.init $RPM_BUILD_ROOT/etc/init.d/rtgpoll-ganglia

%clean
rm -rf $RPM_BUILD_ROOT

%post
/sbin/chkconfig --add rtgpoll-ganglia

%preun
if [ "$1" = 0 ] ; then
  /etc/init.d/rtgpoll-ganglia stop >/dev/null 2>&1
  /sbin/chkconfig --del rtgpoll-ganglia
fi

%postun
if [ "$1" -ge 1 ] ; then
  service rtgpoll-ganglia restart >/dev/null 2>&1
fi

%files
%defattr(-,root,root,-)
/usr/bin/rtgpoll
/etc/init.d/rtgpoll-ganglia
%config(noreplace) /etc/ganglia/rtgpoll/rtg.conf
%config(noreplace) /etc/ganglia/rtgpoll/target.conf
/usr/include/common.h
/usr/include/rtg.h

%changelog
* Thu Oct 06 2011 Nick Satterly <nick.satterly@guardian.co.uk> - 1.0-1
- bumped version to 1.0
- added preun and postun steps
* Tue Sep 06 2011 Nick Satterly <nick.satterly@guardian.co.uk> - 0.9-1
- Modified libganglia require to use new package name
- moved executable to /usr/bin instead of /usr/local/rtg/bin.
* Fri Apr 08 2011 Nick Satterly <nick.satterly@guardian.co.uk> - 0.8-1
- Initial build.

