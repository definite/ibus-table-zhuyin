Name:       ibus-table-zhuyin
Version:    1.0.7.20090414
Release:    1%{?dist}
Summary:    Plain Zhuyin table for IBus
License:    LGPLv2+
Group:      System Environment/Libraries
URL:        http://github.com/definite/ibus-table-zhuyin/tree/master
Source0:    %{name}-%{version}-Source.tar.gz
Source1:

BuildRoot:  %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  ibus-table >= 1.1
BuildRequires:  cmake >= 2.4
BuildRequires:  libchewing >= 0.3.2
Requires:  ibus-table >= 1.1
Requires(post):  ibus-table >= 1.1

%description
The Chewing engine for IBus platform. It provides Chinese input method from
libchewing.

%prep
%setup -q -n %{name}-%{version}-Source

%build
%cmake -DCMAKE_INSTALL_PREFIX=%{_usr}
make VERBOSE=1 C_DEFINES="$RPM_OPT_FLAGS" %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

%post
export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
gconftool-2 --makefile-install-rule \
%{_sysconfdir}/gconf/schemas/%{name}.schemas > /dev/null || :

%preun
if [ "$1" -eq 0 ] ; then
export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
gconftool-2 --makefile-uninstall-rule \
%{_sysconfdir}/gconf/schemas/%{name}.schemas > /dev/null || :
fi

%clean
rm -rf $RPM_BUILD_ROOT

%files -f %{name}.lang
%defattr(-,root,root,-)
%doc AUTHORS README ChangeLog NEWS COPYING
%{_libexecdir}/ibus-engine-chewing
%{_datadir}/%{name}
%{_datadir}/ibus/component/chewing.xml
%config(noreplace) %{_sysconfdir}/gconf/schemas/%{name}.schemas

%changelog
* Mon Apr 20 2009 Ding-Yi Chen <dchen at redhat.com> - 1.0.7.20090414-1






