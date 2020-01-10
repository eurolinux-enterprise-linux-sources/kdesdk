Name:    kdesdk
Summary: The KDE Software Development Kit (SDK)
Version: 4.10.5
Release: 6%{?dist}

License: GPLv2+ and GFDL
URL:     http://www.kde.org/
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/%{version}/src/%{name}-%{version}.tar.xz

Patch0: kdesdk-4.10.5-remove-env-shebang.patch
Patch1: kdesdk-kminspector-multilib.patch
Patch2: kdesdk-4.10.5-fix-crash-with-invalid-selection.patch

%if 0%{?fedora} || 0%{?rhel} > 6
BuildRequires: antlr-static antlr-tool
%else
BuildRequires: antlr jpackage-utils
BuildRequires: java >= 1.6
%endif
BuildRequires: apr-devel
# for libiberty (used by kmtrace for cp_demangle)
BuildRequires: binutils-devel binutils-static
BuildRequires: boost-devel
BuildRequires: flex
BuildRequires: gettext-devel
BuildRequires: kde-baseapps-devel >= %{version}
BuildRequires: kdepimlibs-devel >= %{version}
# kstartperf
BuildRequires: libtool-ltdl-devel
BuildRequires: pkgconfig(hunspell)
BuildRequires: pkgconfig(libical)
BuildRequires: pkgconfig(libstreamanalyzer) pkgconfig(libstreams)
BuildRequires: pkgconfig(libxslt)
BuildRequires: pkgconfig(qca2)
BuildRequires: subversion-devel

Requires: %{name}-cervisia = %{version}-%{release}
Requires: %{name}-dolphin-plugins = %{version}-%{release}
Requires: %{name}-kapptemplate = %{version}-%{release}
Requires: %{name}-kcachegrind = %{version}-%{release}
Requires: %{name}-kioslave = %{version}-%{release}
Requires: %{name}-kmtrace = %{version}-%{release}
Requires: %{name}-kompare = %{version}-%{release}
Requires: %{name}-kpartloader = %{version}-%{release}
Requires: %{name}-kstartperf = %{version}-%{release}
Requires: %{name}-kuiviewer = %{version}-%{release}
Requires: %{name}-okteta = %{version}-%{release}
Requires: %{name}-poxml = %{version}-%{release}
Requires: %{name}-scripts = %{version}-%{release}
Requires: %{name}-strigi-analyzer = %{version}-%{release}
Requires: %{name}-thumbnailers = %{version}-%{release}
Requires: %{name}-umbrello = %{version}-%{release}
# for upgrade path, when kate was split out 
Requires: kate

%description
A metapackage/collection of applications and tools used by developers, including:
* cervisia: a CVS frontend
* kate: advanced text editor
* kcachegrind: a browser for data produced by profiling tools (e.g. cachegrind)
* kompare: diff tool
* kuiviewer: displays designer's UI files
* lokalize: computer-aided translation system focusing on productivity and performance
* okteta: binary/hex editor
* umbrello: UML modeller and UML diagram tool

%package common
Summary: Common files for %{name}
# when split occurred
Conflicts: kdesdk < 4.6.95-10
BuildArch: noarch
%description common
%{summary}.

%package devel
Summary: Development files for %{name}
Requires: %{name}-kmtrace-devel = %{version}-%{release}
Requires: %{name}-kompare-devel = %{version}-%{release}
Requires: %{name}-okteta-devel = %{version}-%{release}
%description devel 
%{summary}.

%package cervisia
Summary: A SCM frontend
Provides: cervisia = %{version}-%{release}
Requires: %{name}-common = %{version}-%{release}
Requires: %{name}-kioslave = %{version}-%{release}
%description cervisia
%{summary}.

%package dolphin-plugins
Summary: Dolphin plugins
Requires: %{name}-common = %{version}-%{release}
%description dolphin-plugins 
%{summary}.

%package kapptemplate
Summary: KDE Template generator 
Provides: kapptemplate = %{version}-%{release}
Requires: %{name}-common = %{version}-%{release}
requires: %{name}-kapptemplate-template = %{version}-%{release}
%description kapptemplate
%{summary}.

%package kapptemplate-template
Summary: KDE Templates
Requires: %{name}-common = %{version}-%{release}
BuildArch: noarch
%description kapptemplate-template
%{summary}.

%package kcachegrind 
Summary: a browser for data provided by profiling tools (like cachegrind) 
Provides: kcachegrind = %{version}-%{release}
Requires: %{name}-common = %{version}-%{release}
%description kcachegrind 
%{summary}.

%package kioslave
Summary: KIO slaves from %{name} 
Requires: %{name}-common = %{version}-%{release}
%description kioslave
%{summary}, including:
kio_perldoc
kio_svn

%package kmtrace
Summary: Assist with malloc debugging using glibc's "mtrace" functionality
Provides: kmtrace = %{version}-%{release}
Requires: %{name}-common = %{version}-%{release}
Requires: %{name}-kmtrace-libs%{?_isa} = %{version}-%{release}
%description kmtrace 
%{summary}.

%package kmtrace-libs
Summary: Runtime libraries for kmtrace
# when split occurred
Obsoletes: kdesdk-libs < 4.6.95-10
Requires: %{name}-kmtrace = %{version}-%{release}
%description kmtrace-libs
%{summary}.

%package kmtrace-devel
Summary: Developer files for kmtrace
Requires: %{name}-kmtrace-libs%{?_isa} = %{version}-%{release}
%description kmtrace-devel
%{summary}.

%package kompare 
Summary: Diff tool 
Provides: kompare = %{version}-%{release}
# when split occurred
Obsoletes: kdesdk-libs < 4.6.95-10
Requires: %{name}-common = %{version}-%{release}
Requires: %{name}-kompare-libs%{?_isa} = %{version}-%{release}
Requires: kate-part%{?_kde4_version: >= %{_kde4_version}}
Requires: diffutils
%description kompare 
%{summary}.

%package kompare-libs
Summary: Runtime libraries for kompare 
Requires: %{name}-kompare = %{version}-%{release}
%description kompare-libs
%{summary}.

%package kompare-devel
Summary: Developer files for kompare 
Provides: kompare-devel = %{version}-%{release}
Requires: %{name}-kompare-libs%{?_isa} = %{version}-%{release}
%description kompare-devel
%{summary}.

%package kpartloader
Summary: KPart loader 
Requires: %{name}-common = %{version}-%{release}
%description kpartloader
%{summary}.

%package kstartperf
Summary: Startup time measurement tool for KDE applications
Requires: %{name}-common = %{version}-%{release}
%description kstartperf
%{summary}.

%package kuiviewer 
Summary: Displays designer UI files 
Requires: %{name}-common = %{version}-%{release}
%description kuiviewer 
%{summary}.

%package lokalize 
Summary: Computer-aided translation system
Provides: lokalize = %{version}-%{release}
Requires: %{name}-common = %{version}-%{release}
Requires: %{name}-strigi-analyzer = %{version}-%{release}
Requires: kross(python)
%description lokalize 
%{summary}, focusing on productivity and performance.

%package okteta 
Summary: Binary/Hex editor 
# okteta moved kdeutils -> kdesdk
Conflicts: kdeutils < 6:4.5.80
Provides: okteta = %{version}-%{release}
Requires: %{name}-common = %{version}-%{release}
Requires: %{name}-okteta-libs%{?_isa} = %{version}-%{release}
%description okteta 
%{summary}.

%package okteta-libs
Summary: Runtime libraries for okteta 
Requires: %{name}-okteta = %{version}-%{release}
%description okteta-libs
%{summary}.

%package okteta-devel
Summary: Developer files for okteta
Provides: okteta-devel = %{version}-%{release}
Requires: %{name}-okteta-libs%{?_isa} = %{version}-%{release}
%description okteta-devel
%{summary}.

%package scripts
Summary: KDE SDK scripts 
Requires: %{name}-common = %{version}-%{release}
# optimizegraphics
Requires: advancecomp
Requires: optipng
BuildArch: noarch
%description scripts 
%{summary}.

%package strigi-analyzer 
Summary: Strigi anayzers from %{name} 
Requires: %{name}-common = %{version}-%{release}
%description strigi-analyzer 
%{summary}, including: diff, po, ts.

%package poxml 
Summary: Text utilities from %{name}
Requires: %{name}-common = %{version}-%{release}
Obsoletes: kdesdk-utils < 4.6.95-10
Provides:  kdesdk-utils = %{version}-%{release}
%{?_qt4_version:Requires: qt4%{?_isa} >= %{_qt4_version}}
%description poxml 
%{summary}, including
po2xml
split2po
swappo
xml2pot

%package thumbnailers 
Summary: Thumbnailers for KDE
Obsoletes: kde-thumbnailer-po <= 2.0 
Provides: kde-thumbnailer-po = %{version}-%{release}
Requires: %{name}-common = %{version}-%{release}
%description thumbnailers 
%{summary}, including gnu gettext po translation files and
gettext translation templates.

%package umbrello
Summary: UML modeller and UML diagram tool
Provides: umbrello = %{version}-%{release}
Requires: %{name}-common = %{version}-%{release}
%description umbrello
%{summary}.


%prep
%setup -q -n kdesdk-%{version}

%patch0 -p1 -b .remove-env-shebang
%patch1 -p1 -b .multilib
%patch2 -p1 -b .fix-crash-with-invalid-selection

%build
mkdir -p %{_target_platform}
pushd %{_target_platform}
%{cmake_kde4} \
  ..
popd

make %{?_smp_mflags} -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

%find_lang cervisia --with-kde --without-mo
%find_lang kapptemplate --with-kde --without-mo
%find_lang kcachegrind --with-kde --without-mo
%find_lang kompare --with-kde --without-mo
%find_lang lokalize --with-kde --without-mo
%find_lang okteta --with-kde --without-mo
%find_lang umbrello --with-kde --without-mo

# unpackaged files
# This one fits better into krazy2 (it requires krazy2), and the version in
# kdesdk does not understand lib64.
rm -f %{buildroot}%{_kde4_bindir}/krazy-licensecheck

# fix documentation multilib conflict
for f in cervisia kcachegrind okteta umbrello ; do
  if [ -f %{buildroot}%{_kde4_docdir}/HTML/en/$f/index.cache.bz2 ] ; then
    bunzip2 %{buildroot}%{_kde4_docdir}/HTML/en/$f/index.cache.bz2
    sed -i -e 's!<a name="id[a-z]*[0-9]*"></a>!!g' %{buildroot}%{_kde4_docdir}/HTML/en/$f/index.cache
    sed -i -e 's!.html#id[a-z]*[0-9]*"!.html"!g' %{buildroot}%{_kde4_docdir}/HTML/en/$f/index.cache
    bzip2 -9 %{buildroot}%{_kde4_docdir}/HTML/en/$f/index.cache
  fi
done

%files

%files common
%doc okteta/COPYING*

%files devel
%{_kde4_includedir}/kprofilemethod.h

%post cervisia
touch --no-create %{_kde4_iconsdir}/hicolor &> /dev/null ||:

%posttrans cervisia
gtk-update-icon-cache %{_kde4_iconsdir}/hicolor &> /dev/null ||:
update-desktop-database -q &> /dev/null ||:

%postun cervisia
if [ $1 -eq 0 ] ; then
touch --no-create %{_kde4_iconsdir}/hicolor &> /dev/null ||:
gtk-update-icon-cache %{_kde4_iconsdir}/hicolor &> /dev/null ||:
update-desktop-database -q &> /dev/null ||:
fi

%files cervisia -f cervisia.lang
%doc cervisia/README
%{_kde4_bindir}/cervisia
%{_kde4_appsdir}/cervisia/
%{_kde4_appsdir}/cervisiapart/
%{_kde4_iconsdir}/hicolor/*/apps/cervisia.*
%{_kde4_libdir}/kde4/cervisiapart.so
%{_kde4_libdir}/libkdeinit4_cervisia.so
%{_kde4_datadir}/applications/kde4/cervisia.desktop
%{_kde4_datadir}/config.kcfg/cervisiapart.kcfg
%{_kde4_datadir}/dbus-1/interfaces/org.kde.cervisia*.xml
%{_kde4_iconsdir}/hicolor/*/actions/*cervisia.*
%{_kde4_datadir}/kde4/services/cervisiapart.desktop
%{_mandir}/man1/cervisia*
%{_kde4_bindir}/cvsaskpass
%{_kde4_bindir}/cvsservice
%{_kde4_libdir}/libkdeinit4_cvsaskpass.so
%{_kde4_libdir}/libkdeinit4_cvsservice.so
%{_kde4_datadir}/kde4/services/cvsservice.desktop

%files dolphin-plugins
%{_kde4_libdir}/kde4/fileviewgitplugin.so
%{_kde4_libdir}/kde4/fileviewsvnplugin.so
%{_kde4_libdir}/kde4/fileviewbazaarplugin.so
%{_kde4_libdir}/kde4/fileviewhgplugin.so
%{_kde4_datadir}/config.kcfg/fileviewgitpluginsettings.kcfg
%{_kde4_datadir}/config.kcfg/fileviewsvnpluginsettings.kcfg
%{_kde4_datadir}/config.kcfg/fileviewhgpluginsettings.kcfg
%{_kde4_datadir}/kde4/services/fileviewgitplugin.desktop
%{_kde4_datadir}/kde4/services/fileviewsvnplugin.desktop
%{_kde4_datadir}/kde4/services/fileviewbazaarplugin.desktop
%{_kde4_datadir}/kde4/services/fileviewhgplugin.desktop

%post kapptemplate
touch --no-create %{_kde4_iconsdir}/hicolor &> /dev/null ||:

%posttrans kapptemplate
gtk-update-icon-cache %{_kde4_iconsdir}/hicolor &> /dev/null ||:

%postun kapptemplate
if [ $1 -eq 0 ] ; then
touch --no-create %{_kde4_iconsdir}/hicolor &> /dev/null ||:
gtk-update-icon-cache %{_kde4_iconsdir}/hicolor &> /dev/null ||:
fi

%files kapptemplate -f kapptemplate.lang
%{_kde4_bindir}/kapptemplate
%{_kde4_datadir}/applications/kde4/kapptemplate.desktop
%{_kde4_datadir}/config.kcfg/kapptemplate.*
%{_kde4_iconsdir}/hicolor/*/apps/kapptemplate.*

%files kapptemplate-template
%{_kde4_appsdir}/kdevappwizard/

%post kcachegrind
touch --no-create %{_kde4_iconsdir}/hicolor &> /dev/null ||:

%posttrans kcachegrind
gtk-update-icon-cache %{_kde4_iconsdir}/hicolor &> /dev/null ||:
update-desktop-database -q &> /dev/null ||:

%postun kcachegrind
if [ $1 -eq 0 ] ; then
touch --no-create %{_kde4_iconsdir}/hicolor &> /dev/null ||:
gtk-update-icon-cache %{_kde4_iconsdir}/hicolor &> /dev/null ||:
update-desktop-database -q &> /dev/null ||:
fi

%files kcachegrind -f kcachegrind.lang
%doc kcachegrind/README
%{_kde4_bindir}/kcachegrind
%{_kde4_bindir}/dprof2calltree
%{_kde4_bindir}/hotshot2calltree
%{_kde4_bindir}/memprof2calltree
%{_kde4_bindir}/op2calltree
%{_kde4_bindir}/pprof2calltree
%{_kde4_appsdir}/kcachegrind/
%{_kde4_datadir}/applications/kde4/kcachegrind.desktop
%{_kde4_iconsdir}/hicolor/*/apps/kcachegrind.*

%post kioslave
touch --no-create %{_kde4_iconsdir}/hicolor &> /dev/null ||:

%posttrans kioslave
gtk-update-icon-cache %{_kde4_iconsdir}/hicolor &> /dev/null ||:

%postun kioslave
if [ $1 -eq 0 ] ; then
touch --no-create %{_kde4_iconsdir}/hicolor &> /dev/null ||:
gtk-update-icon-cache %{_kde4_iconsdir}/hicolor &> /dev/null ||:
fi

%files kioslave
%{_kde4_libdir}/kde4/kio_perldoc.so
%{_kde4_appsdir}/kio_perldoc/pod2html.pl
%{_kde4_datadir}/kde4/services/perldoc.protocol
%{_kde4_bindir}/kio_svn_helper
%{_kde4_libdir}/kde4/kio_svn.so
%{_kde4_datadir}/kde4/services/svn*.protocol
%{_kde4_iconsdir}/hicolor/*/actions/*kiosvn.*
%{_kde4_datadir}/kde4/services/ServiceMenus/subversion.desktop
%{_kde4_datadir}/kde4/services/ServiceMenus/subversion_toplevel.desktop
%{_kde4_libdir}/kde4/kded_ksvnd.so
%{_kde4_datadir}/dbus-1/interfaces/org.kde.ksvnd.xml
%{_kde4_datadir}/kde4/services/kded/ksvnd.desktop

%files kmtrace
%doc kde-dev-utils/kmtrace/README
%{_kde4_bindir}/kmtrace
%{_kde4_bindir}/demangle
%{_kde4_bindir}/kmmatch
%{_kde4_bindir}/kminspector
%{_kde4_appsdir}/kmtrace/

%files kmtrace-libs
%{_kde4_libdir}/libktrace.so.4*

%files kmtrace-devel
%{_kde4_includedir}/ktrace.h
%{_kde4_libdir}/libktrace.so

%post kompare
touch --no-create %{_kde4_iconsdir}/hicolor &> /dev/null ||:

%posttrans kompare
gtk-update-icon-cache %{_kde4_iconsdir}/hicolor &> /dev/null ||:
update-desktop-database -q &> /dev/null ||:

%postun kompare
if [ $1 -eq 0 ] ; then
touch --no-create %{_kde4_iconsdir}/hicolor &> /dev/null ||:
gtk-update-icon-cache %{_kde4_iconsdir}/hicolor &> /dev/null ||:
update-desktop-database -q &> /dev/null ||:
fi

%files kompare -f kompare.lang
%doc kompare/README
%{_kde4_bindir}/kompare
%{_kde4_appsdir}/kompare/
%{_kde4_libdir}/libkomparedialogpages.so
%{_kde4_libdir}/libkomparediff2.so
%{_kde4_datadir}/kde4/servicetypes/kompare*.desktop
%{_kde4_libdir}/kde4/komparenavtreepart.so
%{_kde4_libdir}/kde4/komparepart.so
%{_kde4_datadir}/applications/kde4/kompare.desktop
%{_kde4_iconsdir}/hicolor/*/apps/kompare.*
%{_kde4_datadir}/kde4/services/komparenavtreepart.desktop
%{_kde4_datadir}/kde4/services/komparepart.desktop

%post kompare-libs -p /sbin/ldconfig
%postun kompare-libs -p /sbin/ldconfig

%files kompare-libs
%{_kde4_libdir}/libkompare*.so.*

%files kompare-devel
/usr/include/kde4/kompare/kompareinterface.h
%{_kde4_libdir}/libkompareinterface.so

%files kpartloader
%{_kde4_bindir}/kpartloader
%{_kde4_appsdir}/kpartloader/kpartloaderui.rc

%files kstartperf
#doc kstartperf/README
%{_kde4_bindir}/kstartperf
%{_kde4_libdir}/kde4/kstartperf.so

%post kuiviewer
touch --no-create %{_kde4_iconsdir}/hicolor &> /dev/null ||:

%posttrans kuiviewer
gtk-update-icon-cache %{_kde4_iconsdir}/hicolor &> /dev/null ||:

%postun kuiviewer
if [ $1 -eq 0 ] ; then
touch --no-create %{_kde4_iconsdir}/hicolor &> /dev/null ||:
gtk-update-icon-cache %{_kde4_iconsdir}/hicolor &> /dev/null ||:
fi

%files kuiviewer
%{_kde4_bindir}/kuiviewer
%{_kde4_appsdir}/kuiviewer/
%{_kde4_appsdir}/kuiviewerpart/
%{_kde4_libdir}/kde4/kuiviewerpart.so
%{_kde4_datadir}/applications/kde4/kuiviewer.desktop
%{_kde4_iconsdir}/hicolor/*/apps/kuiviewer.*
%{_kde4_datadir}/kde4/services/kuiviewer_part.desktop
%{_kde4_libdir}/kde4/quithumbnail.so
%{_kde4_datadir}/kde4/services/designerthumbnail.desktop

%post lokalize
touch --no-create %{_kde4_iconsdir}/hicolor &> /dev/null ||:

%posttrans lokalize
gtk-update-icon-cache %{_kde4_iconsdir}/hicolor &> /dev/null ||:
update-desktop-database -q &> /dev/null ||:

%postun lokalize
if [ $1 -eq 0 ] ; then
touch --no-create %{_kde4_iconsdir}/hicolor &> /dev/null ||:
gtk-update-icon-cache %{_kde4_iconsdir}/hicolor &> /dev/null ||:
update-desktop-database -q &> /dev/null ||:
fi

%files lokalize -f lokalize.lang
%{_kde4_bindir}/lokalize
%{_kde4_appsdir}/lokalize/
%{_kde4_iconsdir}/hicolor/*/apps/lokalize.*
%{_kde4_iconsdir}/hicolor/*/actions/approved.*
%{_kde4_iconsdir}/*color/*/actions/*msgid*.*
%{_kde4_iconsdir}/*color/*/actions/catalogmanager.*
%{_kde4_iconsdir}/*color/*/actions/diff.*
%{_kde4_iconsdir}/*color/*/actions/insert_*.*
%{_kde4_iconsdir}/*color/*/actions/next*.*
%{_kde4_iconsdir}/*color/*/actions/prev*.*
%{_kde4_iconsdir}/*color/*/actions/search*.*
%{_kde4_iconsdir}/*color/*/actions/transsearch.*
%{_kde4_datadir}/config.kcfg/lokalize.kcfg
%{_kde4_datadir}/applications/kde4/lokalize.desktop

%post okteta
touch --no-create %{_kde4_iconsdir}/hicolor &> /dev/null ||:

%posttrans okteta
gtk-update-icon-cache %{_kde4_iconsdir}/hicolor &> /dev/null ||:
update-mime-database %{_kde4_datadir}/mime >& /dev/null ||:

%postun okteta
if [ $1 -eq 0 ] ; then
touch --no-create %{_kde4_iconsdir}/hicolor &> /dev/null ||:
gtk-update-icon-cache %{_kde4_iconsdir}/hicolor &> /dev/null ||:
update-mime-database %{_kde4_datadir}/mime >& /dev/null ||:
fi

%files okteta -f okteta.lang
%{_kde4_bindir}/okteta
%{_kde4_bindir}/struct2osd.sh
%{_kde4_appsdir}/okteta/
%{_kde4_appsdir}/oktetapart/
%{_kde4_datadir}/mime/packages/okteta.xml
%{_kde4_libdir}/kde4/oktetapart.so
%{_kde4_datadir}/applications/kde4/okteta.desktop
%{_kde4_datadir}/config/okteta-structures.knsrc
%{_kde4_iconsdir}/hicolor/*/apps/okteta.*
%{_kde4_datadir}/kde4/services/oktetapart.desktop
%{_kde4_libdir}/kde4/libkbytearrayedit.so
%{_kde4_datadir}/kde4/services/kbytearrayedit.desktop
%{_kde4_datadir}/config.kcfg/structviewpreferences.kcfg

%post okteta-libs -p /sbin/ldconfig
%postun okteta-libs -p /sbin/ldconfig

%files okteta-libs
%{_kde4_libdir}/libkasten*.so.*
%{_kde4_libdir}/libokteta*.so.*
%{_kde4_libdir}/kde4/plugins/designer/oktetadesignerplugin.so

%files okteta-devel
%{_kde4_includedir}/KDE/Okteta*/
%{_kde4_includedir}/okteta*/
%{_kde4_libdir}/libokteta*.so
%{_kde4_includedir}/KDE/Kasten*/
%{_kde4_includedir}/kasten*/
%{_kde4_libdir}/libkasten*.so

%files poxml 
%{_kde4_bindir}/po2xml
%{_kde4_bindir}/split2po
%{_kde4_bindir}/swappo
%{_kde4_bindir}/xml2pot
%{_mandir}/man1/po2xml*
%{_mandir}/man1/split2po*
%{_mandir}/man1/swappo*
%{_mandir}/man1/xml2pot*

%files scripts
%doc kde-dev-scripts/README
%{_kde4_bindir}/svnrevertlast
%{_kde4_bindir}/fixuifiles
%{_kde4_bindir}/cvscheck
%{_kde4_bindir}/extend_dmalloc
%{_kde4_bindir}/extractattr
%{_kde4_bindir}/noncvslist
%{_kde4_bindir}/pruneemptydirs
%{_kde4_bindir}/cvsrevertlast
%{_kde4_bindir}/create_makefile
%{_kde4_bindir}/colorsvn
%{_kde4_bindir}/cvslastchange
%{_kde4_bindir}/svngettags
%{_kde4_bindir}/create_svnignore
%{_kde4_bindir}/svnchangesince
%{_kde4_bindir}/build-progress.sh
%{_kde4_bindir}/package_crystalsvg
%{_kde4_bindir}/svnbackport
%{_kde4_bindir}/svnlastlog
%{_kde4_bindir}/cxxmetric
%{_kde4_bindir}/kdemangen.pl
%{_kde4_bindir}/cvsforwardport
%{_kde4_bindir}/includemocs
%{_kde4_bindir}/svnlastchange
%{_kde4_bindir}/wcgrep
%{_kde4_bindir}/qtdoc
%{_kde4_bindir}/nonsvnlist
%{_kde4_bindir}/svnforwardport
%{_kde4_bindir}/create_cvsignore
%{_kde4_bindir}/svnintegrate
%{_kde4_bindir}/kdekillall
%{_kde4_bindir}/create_makefiles
%{_kde4_bindir}/cvsbackport
%{_kde4_bindir}/fixkdeincludes
%{_kde4_bindir}/kde-systemsettings-tree.py
%{_kde4_bindir}/zonetab2pot.py
%{_kde4_bindir}/kde_generate_export_header
%{_kde4_bindir}/cvs-clean
%{_kde4_bindir}/kdelnk2desktop.py
%{_kde4_bindir}/findmissingcrystal
%{_kde4_bindir}/adddebug
%{_kde4_bindir}/cvsversion
%{_kde4_bindir}/cheatmake
%{_kde4_bindir}/cvsblame
%{_kde4_bindir}/optimizegraphics
%{_kde4_bindir}/cvsaddcurrentdir
%{_kde4_bindir}/fix-include.sh
%{_kde4_bindir}/kdedoc
%{_kde4_bindir}/svn-clean
%{_kde4_bindir}/png2mng.pl
%{_kde4_bindir}/extractrc
%{_kde4_bindir}/makeobj
%{_kde4_bindir}/cvslastlog
%{_kde4_bindir}/svnversions
%{_mandir}/man1/adddebug.1.gz
%{_mandir}/man1/cheatmake.1.gz
%{_mandir}/man1/create_cvsignore.1.gz
%{_mandir}/man1/create_makefile.1.gz
%{_mandir}/man1/create_makefiles.1.gz
%{_mandir}/man1/cvscheck.1.gz
%{_mandir}/man1/cvslastchange.1.gz
%{_mandir}/man1/cvslastlog.1.gz
%{_mandir}/man1/cvsrevertlast.1.gz
%{_mandir}/man1/cxxmetric.1.gz
%{_mandir}/man1/demangle.1.gz
%{_mandir}/man1/extend_dmalloc.1.gz
%{_mandir}/man1/extractrc.1.gz
%{_mandir}/man1/fixincludes.1.gz
%{_mandir}/man1/pruneemptydirs.1.gz
%{_mandir}/man1/qtdoc.1.gz
%{_mandir}/man1/reportview.1.gz
%{_mandir}/man1/transxx.1.gz
%{_mandir}/man1/zonetab2pot.py.1.gz

%files strigi-analyzer
%{_kde4_libdir}/strigi/strigita_ts.so
%{_kde4_libdir}/strigi/strigita_xlf.so
%{_kde4_libdir}/strigi/strigila_po.so
%{_kde4_libdir}/strigi/strigila_diff.so
%{_kde4_datadir}/strigi/fieldproperties/strigi_translation.fieldproperties

%files thumbnailers
%{_kde4_libdir}/kde4/pothumbnail.so
%{_kde4_datadir}/kde4/services/pothumbnail.desktop
%{_kde4_datadir}/config.kcfg/pocreatorsettings.kcfg

%post umbrello
touch --no-create %{_kde4_iconsdir}/hicolor &> /dev/null ||:

%posttrans umbrello
gtk-update-icon-cache %{_kde4_iconsdir}/hicolor &> /dev/null ||:
update-desktop-database -q &> /dev/null ||:

%postun umbrello
if [ $1 -eq 0 ] ; then
touch --no-create %{_kde4_iconsdir}/hicolor &> /dev/null ||:
gtk-update-icon-cache %{_kde4_iconsdir}/hicolor &> /dev/null ||:
update-desktop-database -q &> /dev/null ||:
fi

%files umbrello -f umbrello.lang
%doc umbrello/README
%{_kde4_bindir}/umbrello
%{_kde4_appsdir}/umbrello/
%{_kde4_iconsdir}/hicolor/*/apps/umbrello.*
%{_kde4_iconsdir}/hicolor/*/mimetypes/application-x-uml.*
%{_kde4_datadir}/applications/kde4/umbrello.desktop


%changelog
* Thu Jul 24 2014 Jan Grulich <jgrulich@redhat.com> - 4.10.5-6
- Fix crash on find or replace with an invalid selection
  Resolves: bz#1064811

* Wed Mar 05 2014 Than Ngo <than@redhat.com> - 4.10.5-5
- fix several multilib issues

* Fri Jan 24 2014 Daniel Mach <dmach@redhat.com> - 4.10.5-4
- Mass rebuild 2014-01-24

* Mon Jan 13 2014 Jan Grulich <jgrulich@redhat.com> - 4.10.5-3
- Do not use shebang with env (#987073)

* Fri Dec 27 2013 Daniel Mach <dmach@redhat.com> - 4.10.5-2
- Mass rebuild 2013-12-27

* Sun Jun 30 2013 Than Ngo <than@redhat.com> - 4.10.5-1
- 4.10.5

* Sat Jun 01 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.4-1
- 4.10.4

* Mon May 06 2013 Than Ngo <than@redhat.com> - 4.10.3-1
- 4.10.3

* Thu Apr 04 2013 Rex Dieter <rdieter@fedoraproject.org> 4.10.2-2
- move Obsoletes: kdesdk-libs to (sub)pkgs that actually provide replacements

* Mon Apr 01 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.2-1
- 4.10.2

* Sat Mar 02 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.1-1
- 4.10.1

* Fri Feb 01 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.10.0-1
- 4.10.0

* Tue Jan 22 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.9.98-1
- 4.9.98

* Fri Jan 04 2013 Rex Dieter <rdieter@fedoraproject.org> - 4.9.97-1
- 4.9.97

* Thu Dec 20 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.9.95-1
- 4.9.95

* Tue Dec 04 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.9.90-1
- 4.9.90

* Mon Dec 03 2012 Than Ngo <than@redhat.com> - 4.9.4-1
- 4.9.4

* Sat Nov 03 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.9.3-1
- 4.9.3

* Sat Sep 29 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.9.2-1
- 4.9.2
- fix kdevelop kde-simple template

* Mon Sep 03 2012 Than Ngo <than@redhat.com> - 4.9.1-1
- 4.9.1

* Thu Jul 26 2012 Lukas Tinkl <ltinkl@redhat.com> - 4.9.0-1
- 4.9.0

* Thu Jul 19 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.8.97-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Thu Jul 12 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.8.97-1
- 4.8.97

* Wed Jun 27 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.95-1
- 4.8.95

* Mon Jun 11 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.90-2
- respin

* Sun Jun 10 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.8.90-1
- 4.8.90

* Sat May 26 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.80-1
- 4.8.80

* Wed May 23 2012 Than Ngo <than@redhat.com> - 4.8.3-3
- readd requirement on kross-python

* Tue May 22 2012 Than Ngo <than@redhat.com> - 4.8.3-2
- drop needless requirement on kross-python

* Mon Apr 30 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.3-1
- 4.8.3

* Mon Apr 02 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.2-2
- respin

* Fri Mar 30 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.8.2-1
- 4.8.2

* Mon Mar 05 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.1-1
- 4.8.1
- remove extractqml

* Tue Feb 28 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.8.0-3
- Rebuilt for c++ ABI breakage

* Tue Jan 31 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.0-2
- move Java deps to antlr

* Fri Jan 20 2012 Jaroslav Reznik <jreznik@redhat.com> - 4.8.0-1
- 4.8.0
- bump required Java version for antlr with correct epoch

* Wed Jan 04 2012 Rex Dieter <rdieter@fedoraproject.org> - 4.7.97-1
- 4.7.97

* Wed Dec 21 2011 Radek Novacek <rnovacek@redhat.com> - 4.7.95-1
- 4.7.95

* Sun Dec 04 2011 Rex Dieter <rdieter@fedoraproject.org> - 4.7.90-1
- 4.7.90

* Fri Dec 02 2011 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.7.80-3
- -kompare: Requires: diffutils (kde#287748)

* Wed Nov 30 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.80-2
- lokalize: Requires: -strigi-analyzer (kde#287777)

* Fri Nov 25 2011 Radek Novacek <rnovacek@redhat.com> 4.7.80-1
- 4.7.80 (beta 1)
- Drop patch for fixing virtual inheritance of QObject (fixed upstream)
- Drop patch for fixing big endian check/logic to not fail the build (fixed upstream)
- libkomparepart.so renamed to komparepart.so (same for komparenavtreepart.so)

* Sat Oct 29 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.3-1
- 4.7.3
- pkgconfig-style deps

* Sat Oct 08 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.2-2
- -kompare: Requires: kate-part

* Tue Oct 04 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.2-1
- 4.7.2

* Tue Sep 06 2011 Than Ngo <than@redhat.com> - 4.7.1-1
- 4.7.1

* Mon Aug 08 2011 Rex Dieter <rdieter@fedoraproject.org> 4.7.0-3
- *really* (and unconditionally) include umbrello
- fix build on big endian/s390

* Wed Aug 03 2011 Jaroslav Reznik <jreznik@redhat.com> 4.7.0-2
- fix umbrello FTBS (#725077)

* Tue Jul 26 2011 Jaroslav Reznik <jreznik@redhat.com> 4.7.0-1
- 4.7.0
- omit FTBFS umbrello for now (#725077)

* Mon Jul 25 2011 Rex Dieter <rdieter@fedoraproject.org> 4.6.95-12
- Requires: kate (for upgrade path)

* Sun Jul 24 2011 Rex Dieter <rdieter@fedoraproject.org> 4.6.95-11
- add Provides: for subpkgs (sans kdesdk- prefix)

* Fri Jul 22 2011 Rex Dieter <rdieter@fedoraproject.org> 4.6.95-10
- drop kate
- split packaging (#725076)

* Fri Jul 22 2011 Rex Dieter <rdieter@fedoraproject.org> 4.6.95-3
- drop needless -utils scriptlet

* Fri Jul 22 2011 Rex Dieter <rdieter@fedoraproject.org> 4.6.95-2
- Provides: okteta(-devel)

* Fri Jul 08 2011 Rex Dieter <rdieter@fedoraproject.org> 4.6.95-1
- 4.6.95
- drop old kaider references
- -devel: Provides: kate-devel

* Mon Jun 27 2011 Than Ngo <than@redhat.com> - 4.6.90-1
- 4.6.90 (rc1)

* Wed Jun 15 2011 Jaroslav Reznik <jreznik@redhat.com> 4.6.80-1
- 4.6.80 (beta1)

* Thu May 26 2011 Rex Dieter <rdieter@fedoraproject.org> 4.6.3-3
- rebuid (hunspell)

* Tue May 03 2011 Rex Dieter <rdieter@fedoraproject.org> 4.6.3-2
- Unowned  /usr/share/kde4/apps/{cervisiapart,kcachegrind,kuiviewer,kuiviewerpart,lokalize} dirs (#645067)

* Thu Apr 28 2011 Rex Dieter <rdieter@fedoraproject.org> 4.6.3-1
- 4.6.3

* Wed Apr 06 2011 Than Ngo <than@redhat.com> - 4.6.2-1
- 4.6.2

* Sun Feb 27 2011 Rex Dieter <rdieter@fedoraproject.org> 4.6.1-1
- 4.6.1

* Mon Feb 07 2011 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.6.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_15_Mass_Rebuild

* Wed Feb 02 2011 Rex Dieter <rdieter@fedoraproject.org> 4.6.0-2
- -devel: Obsoletes: kdeutils-devel 

* Fri Jan 21 2011 Jaroslav Reznik <jreznik@redhat.com> 4.6.0-1
- 4.6.0

* Thu Jan 06 2011 Jaroslav Reznik <jreznik@redhat.com> 4.5.95-1
- 4.5.95 (4.6rc2)

* Wed Dec 22 2010 Rex Dieter <rdieter@fedoraproject.org> 4.5.90-1
- 4.5.90 (4.6rc1)

* Wed Dec 08 2010 Thomas Janssen <thomasj@fedoraproject.org> 4.5.85-2
- respun upstream tarballs

* Sat Dec 04 2010 Thomas Janssen <thomasj@fedoraproject.org> 4.5.85-1
- 4.5.85 (4.6beta2)

* Sun Nov 21 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.5.80-1
- 4.5.80 (4.6beta1)
- drop kbugbuster, add okteta

* Fri Nov 05 2010 Kevin Kofler <Kevin@tigcc.ticalc.org> - 4.5.3-3
- fix Kompare "malformed diff" false positives (kde#249976)

* Fri Nov 05 2010 Thomas Janssen <thomasj@fedoraproject.org> 4.5.3-2
- rebuild for new libxml2

* Sun Oct 31 2010 Than Ngo <than@redhat.com> - 4.5.3-1
- 4.5.3

* Sat Oct 02 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.5.2-1
- 4.5.2

* Sat Aug 28 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.5.1-1
- 4.5.1

* Tue Aug 03 2010 Than Ngo <than@redhat.com> - 4.5.0-1
- 4.5.0

* Sun Jul 25 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.4.95-1
- 4.5 RC3 (4.4.95)

* Thu Jul 08 2010 Kevin Kofler <Kevin@tigcc.ticalc.org> - 4.4.92-2
- upstream fix for Kompare failing to open temp file when saving (kde#242192)

* Wed Jul 07 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.4.92-1
- 4.5 RC2 (4.4.92)

* Fri Jun 25 2010 Jaroslav Reznik <jreznik@redhat.com> - 4.4.90-1
- 4.5 RC1 (4.4.90)

* Mon Jun 07 2010 Jaroslav Reznik <jreznik@redhat.com> - 4.4.85-1
- 4.5 Beta 2 (4.4.85)
- Requires: advancecomp, optipng (for optimizegraphics) 

* Fri May 21 2010 Jaroslav Reznik <jreznik@redhat.com> - 4.4.80-1
- 4.5 Beta 1 (4.4.80)

* Fri Apr 30 2010 Jaroslav Reznik <jreznik@redhat.com> - 4.4.3-1
- 4.4.3

* Thu Apr 08 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.4.2-2
- -devel: omit (dup'd) libkdeinit4_* stuff
- -utils: add shlib scriptlet

* Mon Mar 29 2010 Lukas Tinkl <ltinkl@redhat.com> - 4.4.2-1
- 4.4.2

* Sat Feb 27 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.4.1-1
- 4.4.1

* Fri Feb 26 2010 Kevin Kofler <Kevin@tigcc.ticalc.org> - 4.4.0-4
- also remove hardcoded setCheckSpellingEnabled(true) calls (kde#228587)

* Fri Feb 26 2010 Kevin Kofler <Kevin@tigcc.ticalc.org> - 4.4.0-3
- fix Cervisia not to unconditionally enable auto spell checking (kde#228587)
- BR binutils-static (F13+)

* Wed Feb 24 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.4.0-2
- -utils: Requires: qt4 dep
- -libs: Requires: %%name ...

* Fri Feb 05 2010 Than Ngo <than@redhat.com> - 4.4.0-1
- 4.4.0

* Sun Jan 31 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.3.98-1
- KDE 4.3.98 (4.4rc3)

* Thu Jan 21 2010 Lukas Tinkl <ltinkl@redhat.com> - 4.3.95-1
- KDE 4.3.95 (4.4rc2)

* Sat Jan 16 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.3.90-2
- rebuild (boost)

* Wed Jan 06 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.3.90-1
- kde-4.3.90 (4.4rc1)

* Fri Dec 18 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.85-1
- kde-4.3.85 (4.4beta2)

* Tue Dec  1 2009 Lukáš Tinkl <ltinkl@redhat.com> - 4.3.80-1
- KDE 4.4 beta1 (4.3.80)

* Tue Nov 24 2009 Ben Boeckel <MathStuf@gmail.com> - 4.3.75-0.2.svn1048496
- Add BR on hunspell for lokalize

* Sun Nov 22 2009 Ben Boeckel <MathStuf@gmail.com> - 4.3.75-0.1.svn1048496
- Update to 4.3.75 snapshot

* Sat Oct 31 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.3-1
- 4.3.3

* Sun Oct 11 2009 Kevin Kofler <Kevin@tigcc.ticalc.org> - 4.3.2-2
- Fix Kompare diff parsing regression (due to a typo in a bugfix)

* Mon Oct 05 2009 Than Ngo <than@redhat.com> - 4.3.2-1
- 4.3.2

* Thu Sep 24 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.1-2
- Requires: kross(python) (#523076)

* Fri Aug 28 2009 Than Ngo <than@redhat.com> - 4.3.1-1
- 4.3.1

* Thu Jul 30 2009 Than Ngo <than@redhat.com> - 4.3.0-1
- 4.3.0

* Wed Jul 22 2009 Than Ngo <than@redhat.com> - 4.2.98-1
- 4.3rc3

* Sun Jul 12 2009 Than Ngo <than@redhat.com> - 4.2.96-1
- 4.3rc2

* Fri Jun 26 2009 Than Ngo <than@redhat.com> - 4.2.95-1
- 4.3rc1

* Thu Jun 04 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.2.90-1
- KDE-4.3 beta2 (4.2.90)

* Thu May 14 2009 Lukáš Tinkl <ltinkl@redhat.com> - 4.2.85-1
- KDE 4.3 beta 1

* Wed Apr 01 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.2.2-2
- optimize scriptlets

* Tue Mar 31 2009 Lukáš Tinkl <ltinkl@redhat.com> - 4.2.2-1
- KDE 4.2.2

* Fri Feb 27 2009 Than Ngo <than@redhat.com> - 4.2.1-1
- 4.2.1
- blockquote patch (#487624)

* Wed Feb 25 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.2.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_11_Mass_Rebuild

* Thu Feb 19 2009 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.2.0-2
- fix Kompare crash with Qt 4.5 (kde#182792)
- fix build with GCC 4.4

* Thu Jan 22 2009 Than Ngo <than@redhat.com> - 4.2.0-1
- 4.2.0

* Mon Jan 19 2009 Than Ngo <than@redhat.com> - 4.1.96-4
- apply patch to fix  build against Boost 1.37.0 

* Tue Jan 13 2009 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.1.96-3
- F11+: add workaround to fix build against Boost 1.37.0

* Fri Jan 09 2009 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.1.96-2
- don't ship krazy-licensecheck, should be in krazy2

* Wed Jan 07 2009 Than Ngo <than@redhat.com> - 4.1.96-1
- 4.2rc1

* Fri Dec 12 2008 Than Ngo <than@redhat.com> 4.1.85-1
- 4.2beta2

* Mon Dec 01 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.1.80-3
- BR plasma-devel instead of kdebase-workspace-devel
- don't require kdebase-workspace
- rebase Lokalize quit menu patch
- BR libical-devel

* Thu Nov 20 2008 Than Ngo <than@redhat.com> 4.1.80-2
- merged

* Thu Nov 20 2008 Lorenzo Villani <lvillani@binaryhelix.net> 4.1.80-1
- 4.1.80
- BR cmake >= 2.6.2
- make install/fast
- kdesdk-4.1.2-kdecore.patch upstreamed, dropped

* Wed Nov 12 2008 Than Ngo <than@redhat.com> 4.1.3-1
- 4.1.3

* Wed Oct 22 2008 Than Ngo <than@redhat.com> 4.1.2-4
- check if the document has been saved, if not ask the user
  to save the change or close without saving

* Wed Oct 22 2008 Rex Dieter <rdieter@fedoraproject.org> 4.1.2-3
- -utils should not depend on kdelibs etc (#467984)

* Mon Sep 29 2008 Rex Dieter <rdieter@fedoraproject.org> 4.1.2-2
- make VERBOSE=1
- respin against new(er) kde-filesystem

* Fri Sep 26 2008 Rex Dieter <rdieter@fedoraproject.org> 4.1.2-1
- 4.1.2

* Wed Sep 24 2008 Than Ngo <than@redhat.com> 4.1.1-2
- show quit in the menu

* Fri Aug 29 2008 Than Ngo <than@redhat.com> 4.1.1-1
- 4.1.1

* Wed Jul 23 2008 Than Ngo <than@redhat.com> 4.1.0-1
- 4.1.0

* Wed Jul 23 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.99-2
- fix -utils dep issues

* Fri Jul 18 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.99-1
- 4.0.99

* Fri Jul 11 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.98-1
- 4.0.98

* Sun Jul 06 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.85-1
- 4.0.85

* Fri Jun 27 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.84-1
- 4.0.84

* Sat Jun 21 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.0.83-2
- drop upstreamed rh#433399 patch

* Thu Jun 19 2008 Than Ngo <than@redhat.com> 4.0.83-1
- 4.0.83 (beta2)

* Sun Jun 15 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.82-1
- 4.0.82

* Mon May 26 2008 Than Ngo <than@redhat.com> 4.0.80-1
- 4.1 beta1

* Fri May 16 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.72-2
- %%description: +kate

* Tue May 13 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.0.72-1
- update to 4.0.72
- Obsoletes/Provides kaider (now part of kdesdk, renamed to lokalize)
- add lokalize to description and file list
- add BR strigi-devel for lokalize
- add BR boost-devel (now needed by Umbrello)

* Fri Apr 18 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.3-5
- Requires: kdesdk-utils

* Fri Apr 18 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.3-4
- utils: po2xml, xml2pot (#432443)

* Thu Apr 03 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.0.3-3
- rebuild (again) for the fixed %%{_kde4_buildtype}

* Mon Mar 31 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.0.3-2
- rebuild for NDEBUG and _kde4_libexecdir

* Fri Mar 28 2008 Than Ngo <than@redhat.com> 4.0.3-1
- 4.0.3

* Thu Feb 28 2008 Than Ngo <than@redhat.com> 4.0.2-1
- 4.0.2

* Wed Feb 27 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.1-2
- kate appears in Applications -> Other (#433399)

* Thu Jan 31 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.1-1
- kde-4.0.1

* Tue Jan 08 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.0.0-1
- update to 4.0.0
- drop upstreamed fix-kompare patch
- update file list (no more katesessionmenu.desktop)

* Wed Dec 19 2007 Kevin Kofler <Kevin@tigcc.ticalc.org> 3.97.0-8
- update Kompare from SVN (rev 750443)

* Sun Dec 16 2007 Kevin Kofler <Kevin@tigcc.ticalc.org> 3.97.0-7
- don't look for libkompare*part.so in %%{_kde4_libdir}
- don't list D-Bus interfaces twice in file list
- build Kompare documentation

* Sun Dec 16 2007 Kevin Kofler <Kevin@tigcc.ticalc.org> 3.97.0-6
- update Kompare from SVN (rev 748928)
- Kompare now installs unversioned (private) shared libs, package them properly

* Wed Dec 12 2007 Kevin Kofler <Kevin@tigcc.ticalc.org> 3.97.0-5
- rebuild for changed _kde4_includedir

* Tue Dec 11 2007 Kevin Kofler <Kevin@tigcc.ticalc.org> 3.97.0-4
- build Kompare's static convenience libraries with -fPIC

* Mon Dec 10 2007 Kevin Kofler <Kevin@tigcc.ticalc.org> 3.97.0-3
- updated fix-kompare patch (rev 6)

* Mon Dec 10 2007 Kevin Kofler <Kevin@tigcc.ticalc.org> 3.97.0-2
- updated fix-kompare patch (rev 5)

* Sat Dec 08 2007 Kevin Kofler <Kevin@tigcc.ticalc.org> 3.97.0-1
- update to 3.97.0 (KDE 4.0 RC2)

* Fri Dec 07 2007 Kevin Kofler <Kevin@tigcc.ticalc.org> 3.96.2-3
- separate -libs subpackage
- remove X11 BRs which are now required by kdelibs-devel

* Thu Dec 06 2007 Kevin Kofler <Kevin@tigcc.ticalc.org> 3.96.2-2
- drop kbabel from description (not actually there)
- reenable kompare, fix its build and porting bugs (kde#153463)
- add missing BR subversion-devel, add files for kio_svn to list
- add missing BR binutils-devel (for libiberty), add files for kmtrace to list

* Fri Nov 30 2007 Sebastian Vahl <fedora@deadbabylon.de> 3.96.2-1
- kde-3.96.2

* Sat Nov 24 2007 Sebastian Vahl <fedora@deadbabylon.de> 3.96.1-1
- kde-3.96.1

* Mon Nov 19 2007 Sebastian Vahl <fedora@deadbabylon.de> 3.96.0-2
- (Build)Require: kdebase-workspace(-devel) (for kate)
- re-enable kate
- BR: kde-filesystem >= 4

* Sun Nov 18 2007 Sebastian Vahl <fedora@deadbabylon.de> 3.96.0-1
- Initial version for Fedora
