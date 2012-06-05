#!/bin/sh

# move perl modules since they are in the 
# gpsdrive-utils package

destdir=debian/gpsdrive
destdir_utils=debian/gpsdrive-utils
mkdir -p "$destdir_utils"

# Move Perl Libs to Utils Package
for file in `find "${destdir}"/usr/share/perl5 -type f` ; do
    dst_name="${destdir_utils}/${file#${destdir}}"
    dir=`dirname $dst_name`
    mkdir -p $dir
    mv -f "${file}" "${dst_name}"
done
for dir in `find "${destdir}"/usr/share/perl5 -type d | sort -r ` ; do
    rmdir $dir
done

mkdir -p "${destdir_utils}/usr/"
mkdir -p "${destdir_utils}/usr/bin"
mv -f "${destdir}"/usr/bin/*.sh "${destdir_utils}/usr/bin"
mv -f "${destdir}"/usr/bin/*.pl "${destdir_utils}/usr/bin"
mv -f "${destdir}"/usr/bin/*.py "${destdir_utils}/usr/bin"
#mv -f "${destdir}"/usr/bin/geo* "${destdir_utils}/usr/bin"

mkdir -p "${destdir_utils}/usr/share/man/man1/"
for file in "${destdir_utils}"/usr/bin/*; do
    filename=`basename $file`
    filename="${filename%.sh}"
    filename="${filename%.pl}"
    filename="${filename%.py}"
    test -e "${destdir}/usr/share/man/man1/$filename.1" || continue
    mv -f "${destdir}/usr/share/man/man1/$filename.1" "${destdir_utils}/usr/share/man/man1/"
done
