Name:          hides-dkms
Version:       %{version}
Release:       %{release}
Summary:       DKMS for HiDes devices and ITE IT950x driver kernel modules
Group:         System Environment/Kernel
License:       GPL
Vendor:        ITE Technologies Inc.
Source0:       hides-dkms-%{version}.tgz
BuildRoot:     %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildArch:     noarch
Requires:      dkms
Requires:      elfutils-libelf-devel

%description
Provide the source code and DKMS setup for the kernel modules which are
required to use HiDes UT-100C USB Modulators. The driver is provided
by ITE Technologies Inc. for IT950x chipsets.

# Disable debuginfo package.
%global debug_package %{nil}

%prep
%setup -q -n hides-dkms-%{version}

%build

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/src $RPM_BUILD_ROOT%{_sysconfdir}/udev/rules.d $RPM_BUILD_ROOT%{_sysconfdir}/security/console.perms.d
cp -r hides-%{version} $RPM_BUILD_ROOT/usr/src
install -m 644 60-hides.rules $RPM_BUILD_ROOT%{_sysconfdir}/udev/rules.d
install -m 644 60-hides.perms $RPM_BUILD_ROOT%{_sysconfdir}/security/console.perms.d

%post
[[ $(/usr/sbin/dkms status | grep hides | grep "%{version}" | wc -l) -eq 0 ]] && /usr/sbin/dkms add -m hides -v "%{version}"
/usr/sbin/dkms build -m hides -v "%{version}"
/usr/sbin/dkms install -m hides -v "%{version}" --force
exit 0

%preun
/usr/sbin/dkms remove -m hides -v "%{version}" --all
exit 0

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
/usr/src/hides-%{version}
%{_sysconfdir}/udev/rules.d/60-hides.rules
%{_sysconfdir}/security/console.perms.d/60-hides.perms
