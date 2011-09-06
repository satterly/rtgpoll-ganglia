Name:           rtgpoll-ganglia
Version:        0.9
Release:        1%{?dist}
Summary:        SNMP Poller for Ganglia
Group:          System Environment/Base
License:        GNU GPL
URL:            http://rtg.sourceforge.net/
Source0: 	%{name}-%{version}.tar.gz
BuildRoot:      %{_builddir}/%{name}-%{version}-rpmroot
Requires:	net-snmp-utils
Requires:	libganglia >= 3.1.7

%description
This is the RTG SNMP poller with Ganglia integration for graphing.

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

%files
%defattr(-,root,root,-)
/usr/bin/rtgpoll
/etc/init.d/rtgpoll-ganglia
%config(noreplace) /etc/ganglia/rtgpoll/rtg.conf
%config(noreplace) /etc/ganglia/rtgpoll/target.conf
/usr/include/common.h
/usr/include/rtg.h

%changelog
* Tue Sep 06 2011 Nick Satterly <nick.satterly@guardian.co.uk> - 0.9-1
- Modified libganglia require to use new package name
- moved executable to /usr/bin instead of /usr/local/rtg/bin.
* Fri Apr 08 2011 Nick Satterly <nick.satterly@guardian.co.uk> - 0.8-1
- Initial build.

