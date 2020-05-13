#!/bin/sh
CURDIR=`readlink -f $(dirname $0)`
libtoolize --copy
aclocal && autoheader && automake --add-missing && autoconf
cd $CURDIR/libgps_mtk3339 && ./autogen.sh && cd $CURDIR
cd $CURDIR/libssd1306 && ./autogen.sh && cd $CURDIR

