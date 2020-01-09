#! /bin/sh
subdirs=". backend gui kresources"
$EXTRACTRC `find $subdirs -name \*.kcfg` >> rc.cpp || exit 11
$EXTRACTRC `find $subdirs -name \*.rc` >> rc.cpp || exit 12
$EXTRACTRC `find $subdirs -name \*.ui` >> rc.cpp || exit 13
$XGETTEXT `find $subdirs -name \*.cpp` -o $podir/kbugbuster.pot
