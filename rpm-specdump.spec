Summary:	Print RPM dump of specfile
Name:		rpm-specdump
Version:	0.1
Release:	3
License:	GPL
Group:		Applications/System
Source0:	%{name}.c
BuildRequires:	rpm-devel
%requires_eq	rpm-lib
BuildRoot:	%{tmpdir}/%{name}-%{version}-root-%(id -u -n)

%description
Print RPM dump of specfile.

%prep
%setup -q -c -T
ln -s %{SOURCE0} rpm-specdump.c

cat <<'EOF' > Makefile
rpm-specdump: rpm-specdump.o
	%{__cc} %{rpmldflags} $< -o $@ -lrpm -lrpmbuild

rpm-specdump.o: rpm-specdump.c
	%{__cc} %{rpmcflags} -Wall -W -I/usr/include/rpm -Wall -c $< -o $@
EOF

%build
%{__make}

%install
rm -rf $RPM_BUILD_ROOT
install -d $RPM_BUILD_ROOT%{_bindir}
install %{name} $RPM_BUILD_ROOT%{_bindir}

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(644,root,root,755)
%attr(755,root,root) %{_bindir}/rpm-specdump
