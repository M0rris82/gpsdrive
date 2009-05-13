#!/bin/sh

src_dir=$1
shift

if echo "$@" | grep -q -e "--help" ; then
    echo "This script tries to rename all scripts to have no extention"
    exit -1 
fi

# Perl Binaries
find $src_dir -name "*.pl" | grep -v -e '\#' -e '~' |\
    while read src_fn ; do 
    #filename="`basename $src_fn`"
    filename=${src_fn%.pl}

    echo "mv \"$src_fn\" \"$filename\""
    mv "$src_fn" "$filename"
done

# Shell Scripts
find $src_dir -name "*.sh" | grep -v -e '\#' -e '~' |\
    while read src_fn ; do 
    #filename="`basename $src_fn`"
    filename=${src_fn%.sh}

    echo "mv \"$src_fn\" \"$filename\""
    mv "$src_fn" "$filename"
done

# Python Scripts
find $src_dir -name "*.py" | grep -v -e '\#' -e '~' |\
    while read src_fn ; do 
    #filename="`basename $src_fn`"
    filename=${src_fn%.py}

    echo "mv \"$src_fn\" \"$filename\""
    mv "$src_fn" "$filename"
done
