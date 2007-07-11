#!/bin/sh
echo "aclocal -I m4 --force ..." && \
aclocal -I m4 --force && \
echo "automake --add-missing ..." && \
automake --add-missing && \
echo "autoconf ..." && \
autoconf && \
echo "\nFinished! Now you can start your './configure'."
