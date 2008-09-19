#!/bin/bash

src_dir=$1
shift
man1_path=$1
shift

if echo "$@" | grep -q -e "--help" ; then
    echo "This script tries to create Man Pages from $src_dir to $man1_path"
    exit -1 
fi

echo "Creating Man Pages from $src_dir to $man1_path"

mkdir -p "$man1_path"

# Perl Binaries
find $src_dir -name "*.pl" | grep -v -e '\#' -e '~' |\
    while read src_fn ; do 
    filename="`basename $src_fn`"
    filename=${filename%.pl}

    # extract manpage from perl program
    man1_fn="$man1_path/${filename}.1"
    if perldoc "$src_fn" >/dev/null 2>&1 ; then
	echo "Create Man Page '$man1_fn' from pod '$src_fn'"
	pod2man $src_fn >"$man1_fn"
    else
	if grep -q -e "--man" "$src_fn"; then
	    echo "Create Man Page '$man1_fn' with '$src_fn'  --man"
	    perl $src_fn --man >"$man1_fn"
	else
	    if grep -q -e "--help" "$src_fn"; then
		echo "Create Man Page '$man1_fn' with '$src_fn'  --help"
		perl $src_fn --help >"$man1_fn"
	    else
		echo "No idea how to create Man Page for $src_fn"
	    fi
	fi
    fi
done

# Shell Scripts
find $src_dir -name "*.sh" | grep -v -e '\#' -e '~' |\
    while read src_fn ; do 
    filename="`basename $src_fn`"
    filename=${filename%.sh}

    # extract manpage from perl program
    man1_fn="$man1_path/${filename}.1"
    if grep -q -e "--man" "$src_fn"; then
	echo "Create Man Page '$man1_fn' with '$src_fn'  --man"
	perl $src_fn --man >"$man1_fn"
    else
	if grep -q -e "--help" "$src_fn"; then
	    echo "Create Man Page '$man1_fn' with '$src_fn'  --help"
	    perl $src_fn --help >"$man1_fn"
	else
	    echo "No idea how to create Man Page for $src_fn"
	fi
    fi
done

echo "Finished Creating Man Pages."
