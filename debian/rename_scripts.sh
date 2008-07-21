#!/bin/bash

src_dir=$1
shift

if echo "$@" | grep -q -e "--help" ; then
    echo "This script tries to rename all scripts to have no extention"
    exit -1 
fi

# Perl Binaries
find $src_dir -name "*.pl" | grep -v -e '\#' -e '~' |\
    while read src_fn ; do 
    filename="`basename $src_fn`"
    filename=${filename%.pl}

    mv "$src_fn" "$filename"
done

# Shell Scripts
find $src_dir -name "*.sh" | grep -v -e '\#' -e '~' |\
    while read src_fn ; do 
    filename="`basename $src_fn`"
    filename=${filename%.sh}

    mv "$src_fn" "$filename"
done

# Python Scripts
find $src_dir -name "*.py" | grep -v -e '\#' -e '~' |\
    while read src_fn ; do 
    filename="`basename $src_fn`"
    filename=${filename%.py}

    mv "$src_fn" "$filename"
done
