Summary:	Print RPM dump of specfile
Summary(pl.UTF-8):	Narzędzie wypisujące RPM-owy zrzut pliku spec
Name:		rpm-specdump
Version:	0.3
Release:	8
License:	GPL
Group:		Applications/System
Source0:	%{name}.c
BuildRequires:	rpm-devel
# commented out due to mixed env on builders
#%requires_eq	rpm-lib
BuildRoot:	%{tmpdir}/%{name}-%{version}-root-%(id -u -n)

%description
Print RPM dump of specfile.

%description -l pl.UTF-8
Narzędzie wypisujące RPM-owy zrzut (dump) pliku spec.

%prep
%setup -q -c -T
ln -s %{SOURCE0} rpm-specdump.c

cat <<'EOF' > Makefile
rpm-specdump: rpm-specdump.o
	%{__cc} %{rpmldflags} $< -o $@ -lrpm -lrpmdb -lrpmio -lrpmbuild

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
