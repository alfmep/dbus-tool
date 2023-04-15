#!/bin/sh

if [ -f Makefile ]; then
    make distclean
fi

rm -rf Makefile.in \
    aclocal.m4 \
    autom4te.cache \
    config.guess \
    config.sub \
    config.log \
    configure \
    depcomp \
    install-sh \
    ltmain.sh \
    m4 \
    missing \
    ar-lib \
    compile \
    src/Makefile.in \
    src/dbus-tool.1
