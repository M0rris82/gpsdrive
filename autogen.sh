#!/bin/sh
# Script to automatically prepare config files using Automake and friends

ACLOCAL_CMD="aclocal -I m4 --force"
AUTOMAKE_CMD="automake --add-missing"
AUTOCONF_CMD="autoconf"

echo "$ACLOCAL_CMD ..." && \
$ACLOCAL_CMD && \
echo "$AUTOMAKE_CMD ..." && \
$AUTOMAKE_CMD && \
echo "$AUTOCONF_CMD ..." && \
$AUTOCONF_CMD && \
echo "\nFinished! Now you can start your './configure'."
