#!/bin/sh
if echo "$@" | grep -q -e "--help"; then
    echo "fetch man page and convert to HTML"
    echo 
    echo "As far as aI can see this script is really old and deprecated (Joerg)"
    exit 1
fi

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

# copy to webspace
#cp gpsdrive_manual-en.[th]* /home/groups/g/gp/gpsdrive/htdocs/
