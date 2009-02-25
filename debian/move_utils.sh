#!/bin/bash

# move perl modules since they are in the 
# gpsdrive-utils package

destdir=debian/gpsdrive
destdir_utils=debian/gpsdrive-utils
mkdir -p $destdir_utils

mkdir -p "${destdir_utils}/usr/"
mv -f ${destdir}/usr/local ${destdir_utils}/usr/
mkdir -p ${destdir_utils}/usr/bin
mv -f ${destdir}/usr/bin/*.pl ${destdir_utils}/usr/bin
mv -f ${destdir}/usr/bin/*.py ${destdir_utils}/usr/bin

mkdir -p ${destdir_utils}/usr/share/man/man1/
for file in ${destdir_utils}/usr/bin/*; do
    filename=`basename $file`
    filename=${filename%.sh}
    filename=${filename%.pl}
    filename=${filename%.py}
    mv -f ${destdir}/usr/share/man/man1/$filename.1  ${destdir_utils}/usr/share/man/man1/
done