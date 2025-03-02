Summary:	Print RPM dump of specfile
Summary(pl.UTF-8):	Narzędzie wypisujące RPM-owy zrzut pliku spec
Name:		rpm-specdump
Version:	0.8
Release:	3
License:	GPL
Group:		Applications/System
Source0:	%{name}.c
Source1:	Makefile
BuildRequires:	rpm-devel
%requires_ge	rpm-lib
BuildRoot:	%{tmpdir}/%{name}-%{version}-root-%(id -u -n)

%description
Print RPM dump of specfile.

%description -l pl.UTF-8
Narzędzie wypisujące RPM-owy zrzut (dump) pliku spec.

%prep
%setup -qcT
ln -s %{SOURCE0} .
ln -s %{SOURCE1} .

%build
%{__make} \
	CC="%{__cc}" \
	RPMLDFLAGS="%{rpmldflags}" \
	RPMCFLAGS="%{rpmcflags}"

%install
rm -rf $RPM_BUILD_ROOT
install -d $RPM_BUILD_ROOT%{_bindir}
install -p %{name} $RPM_BUILD_ROOT%{_bindir}

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(644,root,root,755)
%attr(755,root,root) %{_bindir}/rpm-specdump
