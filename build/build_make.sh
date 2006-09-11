#!/bin/bash

export TERM=xterm

SCRIPT="$0"
COUNT=0
while [ -L "${SCRIPT}" ]
do
	SCRIPT=$(readlink ${SCRIPT})
	COUNT=$(expr ${COUNT} + 1)
	if [ ${COUNT} -gt 100 ]; then
		echo "Too many symbolic links"
		exit 1
	fi
done
BUILDDIR=$(dirname ${SCRIPT})

cd ${BUILDDIR}

case $1 in
	configure)
		cmake -DCMAKE_INSTALL_PREFIX=/usr "$@" ..
	;;
	make)
		cmake -DCMAKE_INSTALL_PREFIX=/usr "$@" ..
		make
	;;
	graph)
		cmake -DCMAKE_INSTALL_PREFIX=/usr --graphviz=${BUILDDIR}/wengophone.dot "$@" ..
		dot -Tpng -o${BUILDDIR}/wengophone.png ${BUILDDIR}/wengophone.dot
		dot -Tsvg -o${BUILDDIR}/wengophone.svg ${BUILDDIR}/wengophone.dot
	;;
	*)
		echo "Usage: $(basename $0) (configure|make|graph)"
		echo
		echo "    configure - run cmake configure"
		echo "    make - run configure and build wengophone"
		echo "    graph - build a picture of the deps"
		echo
	;;
esac

