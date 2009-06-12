This is the Win32 port of gpsdrive.

[FIXME: It should be migrated into the contrib/ dir]

This port is in a PRE-ALPHA STATE and should be seen as a "proof of concept"
only, for now with limited features and bugs/crashes all the time!

Features that are currently not working on the Win32 port:
- gpsd (so only gpsdrive GPS emulation mode for now)
- mysql -> poi, streets, wlan, kismet
- festival voice output

There are no binaries provided yet.


************** "Developer only" information following **************
I've tried to keep the number of changes to the existing source files as low as possible. Most changes required for Win32 are hidden in the Win32 specific config.c/.h files and the newly created os_specific.c/.h.


*** C-Compiler (IDE) ***
http://msdn.microsoft.com/vstudio/express/visualc/download/
Microsoft Visual C++ 2005 Express Edition (the normal 2005 editions should work also)

*** Platform SDK ***
http://www.microsoft.com/downloads/details.aspx?familyid=0BAF2B35-C656-4969-ACE8-E4C0C0716ADB&displaylang=en
Microsoft Platform SDK (header files, libs, ...)

*** MySQL ***
http://dev.mysql.com/downloads/mysql/5.0.html#win32
You'll need the "Windows ZIP/Setup.EXE (x86)" pack, as the Essentials don't contain the required libs/header files.

*** libraries ***
Native Win32 library ports of:
http://www.zlatkovic.com/pub/libxml/
- libxml
libxml2-2.6.28.win32.zip

http://ftp.gnome.org/pub/GNOME/binaries/win32/
- libiconv: 
libiconv-1.9.1.bin.woe32.zip
- glib: 
glib-2.2.3-20040116.zip 
glib-dev-2.2.3-20040116.zip
- gtk2
atk-1.18.0.zip
atk-dev-1.18.0.zip
cairo-1.4.2.zip
cairo-dev-1.4.2.zip
gib-2.12.12.zip
glib-dev-2.12.12.zip
gtk+-2.10.12.zip
gtk+-dev-2.10.12.zip
libpng-1.2.8-bin.zip
pango-1.16.4.zip
pango-dev-1.16.4.zip


*** basics ***
Don't define variables and typedefs inside the function body (MSVC doesn't like ANSI C 99 here). I'm not sure if this can be enabled with command line options somehow.

Put #include "config.h" as the first directive into the .c files.

*** portable functional equivalents ***
I've replaced some "UNIX only" by their ANSI / GTK+ (at least since GTK version 2.2) replacements:
- extern int errno -> #include <errno.h>
- bzero() -> memset()
- bcopy() -> memcpy()
- usleep() -> g_usleep()
- struct timeval, gettimeofday() -> GTimeVal, g_get_current_time()
- DIR, struct dirent, opendir, ... -> GDir, 
- rindex() -> strrchr()
- #include <config.h> -> #include "config.h" (and move that to the start of the file)

*** sockets ***
While the socket functionality is almost the same, there are some differences that needs to be dealt with:
- read(), write() -> recv(), send() (compatible, set additional flag parameter to 0)
- close() -> socket_close() (as defined in os_specific.c/.h)
- setting nonblocking -> socket_nonblocking() (as defined in os_specific.c/.h)

*** problematic, but optional ***
I've used #ifdef _WIN32, to temporary remove some things that's optional and more difficult to solve, to get "the thing" up and running:
- mysql (see below)
- getuid() (detection of running as root)
- SIGUSR1,SIGUSR2 (no equivalent on Win32! - maybe use dbus instead in the future)
- setlocale(LC_MESSAGES) (I don't have a clue on this, simply left it out)
- X11: get_window_sizing() (better use gtk instead, it's already on the todo)
- unit_test() (a lot of typedefs inside the function, which won't work on Win32)
- struct termios (opennmea) (is this still used anyway?)


*** to do ***
Some things I (ULFL) might spend some time in the near future:
- DATADIR needs cleanup
- gpsd win32 binary, using cygwin to compile the official gpsd sources (http://www.renderlab.net/projects/wrt54g/gpsdonwindows.html)
- replace X11 remains in gui.c with gtk code -> window_get_geometry() from Wireshark gui_utils.c
- Use official Win32 "Application Data" folder instead of .gpsdrive
- clarify contents of datadir 
- NSIS based installer
- implement mysql or replace with sqlite? (how to deal with libmysql.dll?)
- festival compile with cygwin 
(http://www.cstr.ed.ac.uk/cgi-bin/cstr/lists.cgi?config=festival_faq&entry=installing_festival)


*** MySQL ***
Disable TCP Port 3306 in the Windows Security Center.

MySQL Installer
- Complete
- Configure the MySQL Server now

MySQL Server Instance Configuration Wizard
- Detailed Configuration
- Developer Machine
- Multifunctional Database
- Installation Path
- Decision Support (DSS) / OLAP
- Enable TCP/IP Networking / Strict Mode
- Standard Character Set
- Install as Windows Service
- Modify Security Settings (remember your password!) / No anonymous account

Command Line
We need to create an account for user=gast, password=gast
- go to mysql bin path (e.g. "C:\Program Files\MySQL\MySQL Server 5.0\bin")
- enter: mysql --user=root -p mysql
- enter password (remembered?)
- enter: GRANT ALL PRIVILEGES ON *.* TO 'monty'@'localhost'
- enter: IDENTIFIED BY 'some_pass' WITH GRANT OPTION;
- enter: quit
Beware, this creates a gast account with ALL privileges, don't use this on a production server!


export PERL5LIB=/<your path>/gpsdrive/share/perl5 ->
unshift(@INC,"/cygdrive/c/Dokumente und Einstellungen/ulfl/Eigene Dateien/svn.gpsdrive.cc/scripts/osm/perl_lib");

perl -MCPAN -e "install Bundle::DBI"
perl -MCPAN -e "install XML::Twig"
perl -MCPAN -e "install Date::Manip"
perl -MCPAN -e "install File::Slurp"
perl -MCPAN -e "install XML::Simple"
perl -MCPAN -e "install WWW::Mechanize"
