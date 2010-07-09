#!/bin/bash
#unset http_proxy
export WGETRC=/dev/null
WGET_QUIET="-nv"
wget $WGET_QUIET -N -p -r --level=2 --no-directories http://translationproject.org/latest/gpsdrive/ --include-directories=latest/gpsdrive
svn add *.po 2>&1 | grep -v -e 'is already under version control'
svn diff | grep -e revisio -e working
