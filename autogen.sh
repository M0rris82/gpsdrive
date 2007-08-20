#!/bin/sh
# Script to automatically prepare config files using Automake and friends


# set command line params
ACLOCAL_CMD="aclocal -I m4 --force"
AUTOMAKE_CMD="automake --add-missing"
AUTOCONF_CMD="autoconf"

#check Automake version
AUTOMAKE_VER=`automake --version | head -n 1 | sed -e 's/^.* //'`
AUTOMAKE_VER_MAJOR=`echo "$AUTOMAKE_VER" | cut -f1 -d.`
AUTOMAKE_VER_MINOR=`echo "$AUTOMAKE_VER" | cut -f2 -d.`
AUTOMAKE_VER_REV=`echo "$AUTOMAKE_VER" | cut -f3 -d.`

# use minor rev as test to see if test worked before integer compares (crummy hack)
if [ -n "$AUTOMAKE_VER_REV" ] && \
    [ "$AUTOMAKE_VER_MAJOR" -le 1 ] && [ "$AUTOMAKE_VER_MINOR" -lt 9 ] ; then
   echo "ERROR: requires automake 1.9.6 or newer." 1>&2
   echo " Found version $AUTOMAKE_VER_MAJOR.$AUTOMAKE_VER_MINOR.$AUTOMAKE_VER_REV" 1>&2
   exit 1
fi
# if the test logic is broken, failover is try anyway,


echo "$ACLOCAL_CMD ..." && \
$ACLOCAL_CMD && \
echo "$AUTOMAKE_CMD ..." && \
$AUTOMAKE_CMD && \
echo "$AUTOCONF_CMD ..." && \
$AUTOCONF_CMD && \
echo "Finished! Now you can start your './configure'."


if [ $? -ne 0 ] ; then
   echo "An error occurred." 1>&2
   exit 1
fi
