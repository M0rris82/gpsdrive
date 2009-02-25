#!/bin/sh
if echo "$@" | grep -q -e "--help"; then
    echo "Fetch man page and convert to HTML."
    echo
    echo "Mostly this script exists to update the online version of the man page"
    echo "  at http://gpsdrive.sourceforge.net/gpsdrive_manual-en.html#index"
# not sure if SF has enabled cron jobs yet from their shell access. If so it could run once a day.
    echo
    exit 1
fi

# written by HB, Nov 2008. (c) 2008 the GpsDrive Development Team

cd working

# checkout
svn export http://gpsdrive.svn.sourceforge.net/svnroot/gpsdrive/trunk/man/gpsdrive.1

# convert to text file
man ./gpsdrive.1 | col -b | grep -v '^XXX' > gpsdrive_manual-en.txt

# convert to HTML
man2html gpsdrive.1 | \
  sed -e 's+^Content-type: text/html$+<!-- Content-type: text/html -->+' \
      -e 's+^Section: User Commands.*<BR>\(.*\)+<B>Jump to \1</B>+' \
      -e 's+<A HREF=".*/cgi-bin/man/man2html">Return to Main Contents</A>+\n+' \
      -e 's+<A HREF=".*/cgi-bin/man/man2html">man2html</A>+man2html+' \
      -e 's+<BODY>+<BODY bgcolor="#F0F0F0">+' \
  > gpsdrive_manual-en.html

# copy to SF webspace
#cp gpsdrive_manual-en.[th]* /home/groups/g/gp/gpsdrive/htdocs/
