################################################################################
#
# spec file for humbug-collector package
#
# Copyright  (c) 2011  Humbug Telecom Labs Ltd.
# 
# Igor Ratgauzer <igor@humbuglabs.org>
#
################################################################################

Name:           humbug-collector
Version:        0.8
Release:        1%{?dist}
Summary:        The Humbug analytics collection agent
Group:          Applications/Communications

License:        GPL
Packager:       Igor Ratgauzer <igor@humbuglabs.org>
Vendor:         Humbug Telecom Labs Ltd.
URL:            http://www.humbuglabs.org

Source:         humbug-collector-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}
BuildRequires:	gcc, newt

%description
The Humbug Collector connects your PBX to Humbug Analytics 
for in-depth reporting, alerting and protection from fraud.

%prep
%setup -q

%build
make clean all

%install
%{__rm} -rf %{buildroot}
%{__install} -Dp -m0644 humbug.conf %{buildroot}/etc/humbug/humbug.conf
%{__install} -Dp -m0755 %{name} %{buildroot}%{_bindir}/%{name}
%{__install} -Dp -m0755 humbug-setup %{buildroot}%{_bindir}/humbug-setup
%{__install} -Dp -m0755 init.d/rc.redhat.humbug %{buildroot}/etc/init.d/%{name}
%{__install} -Dp -m0755 humbug-config.sh %{buildroot}/usr/bin/humbug-config.sh
%{__install} -Dp -m0755 humbug-collector-keeper.sh %{buildroot}/usr/bin/humbug-collector-keeper.sh
%{__install} -Dp -m0755 humbug-cron-config.sh %{buildroot}/usr/bin/humbug-cron-config.sh
%{__install} -d -m0755 %{buildroot}/var/run/humbug
%{__install} -d -m0755 %{buildroot}/var/log/humbug

%post
if [ "$1" = "1" ]; then
	/sbin/chkconfig --add %{name}
fi
/usr/bin/humbug-cron-config.sh install
echo 	"+-----------------------------------------------------+"
echo 	"| humbug-collector, v.0.8.0 (c) 2011 Humbug Labs Ltd. |"
echo 	"+-----------------------------------------------------+"
echo 	"|  To start the Humbug Agent follow the below steps:  |"
echo    "|                                                     |"
echo    "| 1. Run the configuration tool using the following:  |"
echo    "|    command: /usr/bin/humbug-config.sh               |"
echo 	"| 2. Edit the configuration file located at:          |"
echo    "|    /etc/humbug/humbug.conf                          |"
echo 	"|    or as another option, use the config tool:       |"
echo    "|    /usr/bin/humbug-setup                            |"
echo    "| 3. Start the humbug-collector agent:                |"
echo 	"|    /etc/init.d/humbug-collector start               |"
echo 	"|                                                     |"
echo 	"+-----------------------------------------------------+"
echo    "| Thank you from installing and participating in the  |"
echo    "|     Humbug Fraud Analysis Community Initiative.     |"
echo 	"+-----------------------------------------------------+"

%preun
if [ "$1" = "0" ]; then
	/sbin/service %{name} stop >/dev/null 2>&1
	/sbin/chkconfig --del %{name}
	/usr/bin/humbug-cron-config.sh uninstall
fi

%clean
%{__rm} -rf %{buildroot} $RPM_BUILD_DIR/%{name}-%{version}/

%files
%defattr(-,root,root)
%dir /etc/humbug
%config /etc/humbug/humbug.conf
/etc/init.d/humbug-collector
%{_bindir}/humbug-config.sh
%{_bindir}/humbug-collector-keeper.sh
%{_bindir}/humbug-cron-config.sh
%{_bindir}/humbug-collector
%{_bindir}/humbug-setup
%dir /var/run/humbug
%dir /var/log/humbug

%changelog
* Tue Feb 21 2012 Igor Ratgauzer <igor@humbuglabs.org> - 0.8-0
- Added blacklist
- Added business hours list
- Added timeranges list
* Mon Oct 10 2011 Igor Ratgauzer <igor@humbuglabs.org> - 0.4-0
- Added collector-keeper script
- Added cron-config script
- New daemonize function
- New log function
* Wed Jun 1 2011 Igor Ratgauzer <igor@humbuglabs.org> - 0.3.5-3
- Fix few small bugs
* Wed May 2 2011 Igor Ratgauzer <igor@humbuglabs.org> - 0.3.5-1
- First release
