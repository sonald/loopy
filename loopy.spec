Name: loopy
Version: 0.5.4
Release: 2
Summary: a themable media player powered by phonon
License: RedFlag License
Vendor:	RedFlag Linux
Url: http://www.redflag-linux.com
Group:	Applications/Multimedia
Packager:	packagerroot
Requires:	qt4 phonon kdelibs
BuildRequires: phonon-devel qt-devel kdelibs-devel cmake
BuildRoot : /var/tmp/%{name}-%{version}-root
Source:	%{name}-%{version}.tar.bz2

%description
a themable media player powered by kde phonon framework. newest backends needed
to support subtitles.

%prep
%setup -q

%build
cmake -DCMAKE_INSTALL_PREFIX=/usr
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%{_bindir}/loopy

%dir %{_kde4_appsdir}/loopy/themes/blackglass-light
%{_kde4_appsdir}/loopy/themes/blackglass-light/style.qss
%{_kde4_appsdir}/loopy/themes/blackglass-light/normal-icon.png
%{_kde4_appsdir}/loopy/themes/blackglass-light/close-icon.png

%dir %{_kde4_appsdir}/loopy/themes/blackglass-fat
%{_kde4_appsdir}/loopy/themes/blackglass-fat/style.qss
%{_kde4_appsdir}/loopy/themes/blackglass-fat/normal-icon.png
%{_kde4_appsdir}/loopy/themes/blackglass-fat/close-icon.png
%{_kde4_appsdir}/loopy/themes/blackglass-overkill
%{_kde4_appsdir}/loopy/themes/blackglass-overkill/style.qss
%{_kde4_appsdir}/loopy/themes/blackglass-overkill/draw-donut.png
%{_kde4_appsdir}/loopy/themes/blackglass-overkill/normal-icon.png
%{_kde4_appsdir}/loopy/themes/blackglass-overkill/draw-circle.png
%{_kde4_appsdir}/loopy/themes/blackglass-overkill/close-icon.png

%dir %{_kde4_appsdir}/loopy/themes/graygradients
%{_kde4_appsdir}/loopy/themes/graygradients/style.qss
%{_kde4_appsdir}/loopy/themes/graygradients/draw-donut.png
%{_kde4_appsdir}/loopy/themes/graygradients/normal-icon.png
%{_kde4_appsdir}/loopy/themes/graygradients/draw-circle.png
%{_kde4_appsdir}/loopy/themes/graygradients/close-icon.png

/usr/share/applications/kde4/loopy.desktop
/usr/share/applications/kde4/loopy_enqueue.desktop
%{_kde4_appsdir}/solid/actions/loopy_play_dvd.desktop
%{_kde4_appsdir}/loopy/loopyui.rc
/usr/share/config.kcfg/loopy.kcfg

# translations
/usr/share/locale/pt_BR/LC_MESSAGES/loopy.mo
/usr/share/locale/cs/LC_MESSAGES/loopy.mo
/usr/share/locale/zh_CN/LC_MESSAGES/loopy.mo
/usr/share/locale/de/LC_MESSAGES/loopy.mo
/usr/share/locale/hu/LC_MESSAGES/loopy.mo

%changelog
* Tue Sep 06 2011 sycao <sycao@redflag-linux.com> 0.5.4-2
- 0.5.4 fixes and zh_CN translation

* Tue Aug 08 2011 sycao <sycao@redflag-linux.com> 0.5.4-1
- initial package
