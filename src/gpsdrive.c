/***********************************************************************

Copyright (c) 2001-2004 Fritz Ganter <ganter@ganter.at>

Website: www.gpsdrive.de

Disclaimer: Please do not use for navigation. 

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    *********************************************************************

$Log$
Revision 1.11  2005/01/15 23:46:46  tweety
Added config option to disable/enable drawing of grid

Revision 1.10  2005/01/11 21:33:40  tweety
rename ->longitude ->lon

Revision 1.9  2005/01/11 18:11:51  tweety
improved draw raster

Revision 1.8  2005/01/11 01:37:06  tweety
implement first part of drawgrid

Revision 1.7  2005/01/11 00:47:05  tweety
added an 8th column to way.txt which means proximity
if this filed is filled a circle arround the
waypoint is displayed with radius proximity Meters

Revision 1.6  2004/12/27 10:21:26  tweety
Change map filename checks. After this change you can use directoryname and filename to
specify you map-files in map_koord.txt. So you can use subdirecories in you gpsdrive dir.

Revision 1.5  2004/12/25 19:21:51  commiter
splitting functions out of gpsdrive.c

Revision 1.4  2004/12/25 15:10:36  commiter
splitting functions out

Revision 1.3  2004/12/24 15:49:17  commiter
Split error_popup() into separate file

Revision 1.2  2004/12/24 15:44:15  commiter
Split quit_program into separate file

Revision 1.1.1.1  2004/12/23 16:03:24  commiter
Initial import, straight from 2.10pre2 tar.gz archive

Revision 1.526  2004/04/05 18:45:40  ganter
added patch for kismet hangs and Mac-OS mouse
patches provided by Ulrich Hecht from SUSE.

Revision 1.525  2004/03/03 20:46:47  ganter
fixed big bug in waypoint list if format not set to decimal display

Revision 1.524  2004/03/02 01:52:47  ganter
changed expedia tooltips
yes, this is the real 2.09pre1 :-)

Revision 1.523  2004/03/02 01:36:04  ganter
added German Expedia map server (expedia.de),
this should be used for european users
this is the real 2.09pre1 beta

Revision 1.522  2004/03/02 00:53:35  ganter
v2.09pre1
added new gpsfetchmap.pl (works again with Expedia)
added sound settings in settings menu
max serial device string is now 40 char

Revision 1.521  2004/02/24 16:59:14  ganter
fixed segfault if expedia could not resolved or reached.
upload new 2.08

Revision 1.520  2004/02/18 13:24:18  ganter
navigation

Revision 1.519  2004/02/16 23:15:20  ganter
activated navigation.c for teleatlas street maps, need some months of work to
get it functional.

Revision 1.518  2004/02/12 17:42:53  ganter
added -W switch for enable/disable WAAS/EGNOS (for SiRF II only?)

Revision 1.517  2004/02/11 21:59:22  ganter
max. number of sats is now MAXSATS

Revision 1.516  2004/02/11 21:01:53  ganter
added debug output for satellites

Revision 1.515  2004/02/11 20:27:40  ganter
increased max satellite number from 40 to 80

Revision 1.514  2004/02/11 11:25:45  ganter
added patch from Johnny Cache <johnycsh@hick.org>, dbname is now configurable
in gpsdriverc
additional search path for libmysql for cygwin

Revision 1.513  2004/02/09 23:34:30  ganter
fixed topomap bug

Revision 1.512  2004/02/09 17:06:39  ganter
fixed timeout behavior for direct serial connection

Revision 1.511  2004/02/08 17:16:25  ganter
replacing all strcat with g_strlcat to avoid buffer overflows

Revision 1.510  2004/02/08 16:35:10  ganter
replacing all sprintf with g_snprintf to avoid buffer overflows

Revision 1.509  2004/02/08 12:44:10  ganter
replacing all sprintf with g_snprintf to avoid buffer overflows

Revision 1.508  2004/02/07 17:46:09  ganter
...

Revision 1.507  2004/02/07 09:25:42  ganter
changed status string

Revision 1.506  2004/02/07 00:02:16  ganter
added "store timezone" button in settings menu

Revision 1.505  2004/02/06 17:11:14  ganter
disabled mapblast server

Revision 1.504  2004/02/06 16:41:40  ganter
added -E parameter, which prints out the NMEA messages received

Revision 1.503  2004/02/06 14:55:54  ganter
added support for user-defined icons
create the directory: $HOME/.gpsdrive/icons
place your icons (type must be png) into this directory, with the name of
the waypoint type, filename must be lowercase
i.e. for waypoint type "HOTEL" the file must have the name "hotel.png"

Revision 1.502  2004/02/05 19:47:31  ganter
replacing strcpy with g_strlcpy to avoid bufferoverflows
USB receiver does not send sentences in direct serial mode,
so I first send a "\n" to it

Revision 1.501  2004/02/05 08:04:01  ganter
if no sat fix, satdisplay is red, otherwise green

Revision 1.500  2004/02/04 20:27:47  ganter
adjust sat level bars

Revision 1.499  2004/02/04 14:47:09  ganter
added GPGSA sentence for PDOP (Position Dilution Of Precision).

Revision 1.498  2004/02/03 23:19:27  ganter
fixed wrong string size

Revision 1.497  2004/02/03 08:20:49  ganter
night mode works fine
map handling for new users also
fixed bug: you was unable to edit the name in friends menu

Revision 1.496  2004/02/03 07:11:20  ganter
working on problems if gpsdrive is not installed

Revision 1.495  2004/02/03 06:17:52  ganter
nightmode sets background to red

Revision 1.494  2004/02/02 17:54:14  ganter
new, self rendered earth animation

Revision 1.493  2004/02/02 09:03:42  ganter
2.08pre10

Revision 1.492  2004/02/02 08:23:10  ganter
in "Search" window and in "Send message" window you can sort the entries
by clicking on the column label

Revision 1.491  2004/02/02 07:12:57  ganter
inserted function calcxytopos, key x,y and right mouseclick are now correct in topomaps

Revision 1.490  2004/02/02 03:38:31  ganter
code cleanup

Revision 1.489  2004/02/01 01:57:02  ganter
it seems that nasamaps now working fine

Revision 1.488  2004/01/31 14:48:03  ganter
pre8

Revision 1.487  2004/01/31 13:43:56  ganter
nasamaps are working better, but still bugs

Revision 1.486  2004/01/31 06:24:21  ganter
nasa maps at lon=0 works now

Revision 1.485  2004/01/31 04:43:04  ganter
...

Revision 1.484  2004/01/31 03:33:11  ganter
nasa map loading seems to work
bug: it would not work around 0 meridian

Revision 1.483  2004/01/30 22:20:44  ganter
convnasamap creates mapfiles from the big nasa map files

Revision 1.482  2004/01/30 17:54:57  ganter
i have to add gdk_threads_enter()/gdk_threads_leave() into all timeouts :-(

Revision 1.481  2004/01/30 02:58:04  ganter
new splash picture

Revision 1.480  2004/01/29 05:11:33  ganter
v2.08pre6
changed sat level to GPS info

Revision 1.479  2004/01/29 04:42:17  ganter
after valgrind

Revision 1.478  2004/01/29 02:32:58  ganter
changed layout of sats display

Revision 1.477  2004/01/28 15:31:43  ganter
initialize FDs to -1

Revision 1.476  2004/01/28 14:46:02  ganter
fixed silly  if (sock == 0) bug

Revision 1.475  2004/01/28 12:33:45  ganter
...

Revision 1.474  2004/01/28 10:14:31  ganter
replaced earth with a better one

Revision 1.473  2004/01/28 09:32:57  ganter
tested for memory leaks with valgrind, looks good :-)

Revision 1.472  2004/01/28 04:04:38  ganter
new animation

Revision 1.471  2004/01/28 03:20:16  ganter
moved tooltip

Revision 1.470  2004/01/28 03:13:57  ganter
...

Revision 1.469  2004/01/28 03:11:11  ganter
added gpsdriveanim.gif handling

Revision 1.468  2004/01/28 02:59:18  ganter
added animated icon
you can now switch between gpsd and sim mode

Revision 1.467  2004/01/27 22:51:56  ganter
added "direct serial connection" button in settings menu

Revision 1.466  2004/01/27 08:29:57  ganter
fixed bug of not working simulation mode

Revision 1.465  2004/01/27 06:59:14  ganter
The baudrate is now selectable in settings menu
GpsDrive now connects to the GPS receiver in following order:
Try to connect to gpsd
Try to find Garble-mode Garmin
Try to read data directly from serial port

If this all fails, it falls back into simulation mode

Revision 1.464  2004/01/27 05:25:59  ganter
added gpsserial.c

gpsdrive now detects a running gps receiver
You don't need to start gpsd now, serial connection is handled by GpsDrive directly

Revision 1.463  2004/01/25 05:58:41  ganter
...

Revision 1.462  2004/01/25 05:51:48  ganter
added alignments for battery and temperature

Revision 1.461  2004/01/25 01:11:34  ganter
...

Revision 1.460  2004/01/25 00:16:23  ganter
centered compass and satlevel display

Revision 1.459  2004/01/24 23:52:35  ganter
fixed bug in NESW (north,east,south,west compass label) string handling with
unicode

Revision 1.458  2004/01/24 22:51:55  ganter
set transient for file dialogs

Revision 1.457  2004/01/24 03:27:50  ganter
friends label color is now changeable in settings menu

Revision 1.456  2004/01/22 06:38:02  ganter
working on friendsd

Revision 1.455  2004/01/22 05:49:22  ganter
friendsd now sends a receiving acknoledge

Revision 1.454  2004/01/21 06:37:54  ganter
added compass image

Revision 1.453  2004/01/20 21:50:36  ganter
...

Revision 1.452  2004/01/20 21:35:51  ganter
import maps is working again

Revision 1.451  2004/01/20 17:39:18  ganter
working on import function

Revision 1.450  2004/01/20 06:25:18  ganter
fixed N/S bug in display
disabled non-working menu entries

Revision 1.449  2004/01/18 20:01:23  ganter
fixed bug for GTK<2.2.x

Revision 1.448  2004/01/18 19:37:24  ganter
this is the nice 2.07 release

Revision 1.447  2004/01/18 06:34:54  ganter
button for reminder window

Revision 1.446  2004/01/18 05:53:35  ganter
try to find the problem that x-server eats cpu after 5 hours

Revision 1.445  2004/01/18 05:28:02  ganter
changed all popups to gtk_dialog instead of a toplevel window
cosmetic changes in settings menu

Revision 1.444  2004/01/17 18:12:17  ganter
fixed some memory leaks

Revision 1.443  2004/01/17 17:41:48  ganter
replaced all gdk_pixbuf_render_to_drawable (obsolet) with gdk_draw_pixbuf

Revision 1.442  2004/01/17 06:11:04  ganter
added color setting for track color

Revision 1.441  2004/01/17 03:42:13  ganter
added better icon

Revision 1.440  2004/01/17 00:58:09  ganter
randomize the startposition and set it the hamburg cementry :-)

Revision 1.439  2004/01/16 19:49:30  ganter
added new icon

Revision 1.438  2004/01/16 13:19:59  ganter
update targetlist if goto button pressed

Revision 1.437  2004/01/15 22:10:11  ganter
changed waypoint layout

Revision 1.436  2004/01/15 21:57:43  ganter
v2.07pre9

Revision 1.435  2004/01/15 16:00:21  ganter
added gpssmswatch

Revision 1.434  2004/01/14 04:26:24  ganter
indent

Revision 1.433  2004/01/14 04:25:17  ganter
cosmetic changes in sendname selection

Revision 1.432  2004/01/14 03:47:36  ganter
removed some debug output

Revision 1.431  2004/01/14 03:31:37  ganter
now message acknoledge is done to and from friendsserver

Revision 1.430  2004/01/14 01:20:28  ganter
added a Goto button in search menu, now you can jump to your waypoints

Revision 1.429  2004/01/14 00:48:49  ganter
fixed bug if no crypt is avail.

Revision 1.428  2004/01/14 00:06:27  ganter
...

Revision 1.427  2004/01/13 23:38:30  ganter
added new field in waypoints display for number of friends received

Revision 1.426  2004/01/13 20:16:07  ganter
smaller message compose window

Revision 1.425  2004/01/13 19:43:50  ganter
removed old GTK1.x accelerators

Revision 1.424  2004/01/13 19:30:41  ganter
changed "operations menu" do "Misc. menu"

Revision 1.423  2004/01/13 14:31:31  ganter
status bar

Revision 1.422  2004/01/13 14:09:43  ganter
...

Revision 1.421  2004/01/13 12:52:54  ganter
added patch from Russell Harding <hardingr@billingside.com> for better menu bar

Revision 1.420  2004/01/13 00:31:41  ganter
fixed multiline message bug

Revision 1.419  2004/01/12 22:22:06  ganter
make message menu entry insensitive if message is not yet send

Revision 1.418  2004/01/12 22:00:18  ganter
changed "Chat" to "Messages"

Revision 1.417  2004/01/12 21:52:02  ganter
added friends message service

Revision 1.416  2004/01/11 13:55:22  ganter
...

Revision 1.415  2004/01/11 13:54:15  ganter
reduce height

Revision 1.414  2004/01/11 13:48:28  ganter
added about screen
added menubar

Revision 1.413  2004/01/10 10:06:00  ganter
make select target popup larger

Revision 1.412  2004/01/10 09:59:56  ganter
avoid NAN in calcdist if position is exactly the same as destination

Revision 1.411  2004/01/10 09:54:16  ganter
autsch, big mistake in drawfriends

Revision 1.410  2004/01/10 09:38:47  ganter
some changes in friendsmode settings menu

Revision 1.409  2004/01/09 13:14:08  ganter
fixed locale bug for topo maps

Revision 1.408  2004/01/09 10:32:24  ganter
...

Revision 1.407  2004/01/09 00:00:43  ganter
added topomap download patch from Russell Harding <hardingr@billingside.com>
Thanks for the lot of work!

Revision 1.406  2004/01/06 09:26:37  ganter
...

Revision 1.405  2004/01/06 08:46:16  ganter
changed target text to the shorter form: "To:" instead of "Distance to"

Revision 1.404  2004/01/06 07:50:51  ganter
friendsmode: displays selected unit instead of km/h, displayed time now respects timezone setting.

Revision 1.403  2004/01/05 05:52:58  ganter
changed all frames to respect setting

Revision 1.402  2004/01/04 17:05:15  ganter
display SQL waypoint fields only if SQL is used

Revision 1.401  2004/01/03 03:39:52  ganter
added settings switch for etched frames

Revision 1.400  2004/01/03 02:08:02  ganter
...

Revision 1.399  2004/01/02 10:15:05  ganter
changed back Geschw. to Geschwindigkeit (in german translation)

Revision 1.398  2004/01/02 09:42:35  ganter
...

Revision 1.397  2004/01/02 09:19:23  ganter
...

Revision 1.396  2004/01/02 09:07:00  ganter
translated

Revision 1.395  2004/01/02 08:58:23  ganter
working on wplabels

Revision 1.394  2004/01/02 08:37:18  ganter
nicer waypoint info

Revision 1.393  2004/01/01 09:07:31  ganter
v2.06
trip info is now live updated
added cpu temperature display for acpi
added tooltips for battery and temperature

Revision 1.392  2003/12/28 23:33:48  ganter
...

Revision 1.391  2003/12/28 23:29:15  ganter
added battery tooltip

Revision 1.390  2003/12/28 23:19:45  ganter
tooltip for temperature

Revision 1.389  2003/12/28 22:58:05  ganter
...

Revision 1.388  2003/12/28 22:29:26  ganter
...

Revision 1.387  2003/12/28 22:20:23  ganter
temp tooltip

Revision 1.386  2003/12/28 22:16:20  ganter
...

Revision 1.385  2003/12/28 21:35:34  ganter
temp...

Revision 1.384  2003/12/28 21:32:52  ganter
...

Revision 1.383  2003/12/28 21:19:28  ganter
working on temperature

Revision 1.382  2003/12/28 20:12:29  ganter
better acpi-temperature support

Revision 1.381  2003/12/28 19:48:01  ganter
added patch from Jaap Hogenberg for temperature display

Revision 1.380  2003/12/28 08:49:35  ganter
...

Revision 1.379  2003/12/28 08:44:11  ganter
distance display changes

Revision 1.378  2003/12/28 08:21:19  ganter
new gpsreplay version

Revision 1.377  2003/12/28 08:14:37  ganter
cosmetic changes in distance and altitude display

Revision 1.376  2003/12/28 07:47:24  ganter
...

Revision 1.375  2003/12/28 07:44:16  ganter
removed some warnings

Revision 1.373  2003/12/28 06:04:54  ganter
new version 1.20 of gpsreplay

Revision 1.372  2003/12/23 23:33:51  ganter
release 2.06pre2

Revision 1.371  2003/12/23 23:27:29  ganter
fix bug (download button was not sensitive after download)

Revision 1.370  2003/12/23 21:00:27  ganter
v2.06pre2
disable multiple popups

Revision 1.369  2003/12/22 19:33:34  ganter
better test if image has alpha

Revision 1.368  2003/12/21 17:31:32  ganter
error handling for not installed program
real v2.05 :-)

Revision 1.367  2003/12/21 16:59:03  ganter
fixed bug in timezone setting
timezone will be stored now

Revision 1.366  2003/12/17 21:30:02  ganter
acpi battery status works now again (tested with 2.4.22ac4)

Revision 1.365  2003/12/17 20:32:31  ganter
now CPU load is reduced heavily through removing alpha channel from image

Revision 1.364  2003/12/17 02:17:56  ganter
added donation window
waypoint describtion (.dsc files) works again
added dist_alarm ...

Revision 1.363  2003/10/23 17:39:26  ganter
changes for cygwin

Revision 1.362  2003/10/04 17:43:58  ganter
translations don't need to be utf-8, but the .po files must specify the
correct coding (ie, UTF-8, iso8859-15)

Revision 1.361  2003/10/01 23:42:30  ganter
specfile patched

Revision 1.360  2003/09/18 10:33:40  ganter
changed orange color

Revision 1.359  2003/09/17 19:31:47  ganter
cvs test

Revision 1.358  2003/09/17 12:05:14  ganter
2.05pre1
fixed malloc problem in friends server
force name in friendsmode to replace space with underscore

Revision 1.357  2003/08/12 14:21:18  ganter
v2.03
fixed kismet bug (wrong GPS position)
compiles also on SuSE 8.1
compiles on GTK+ >= 2.0.6
fixed wrong font (Sans 10 Bold 10 message)

Revision 1.356  2003/08/12 12:21:19  ganter
fixed bugs of PDA patch

Revision 1.355  2003/07/25 23:28:34  ganter
2.01
expedia works again

Revision 1.354  2003/07/25 12:17:14  ganter
2.00

Revision 1.353  2003/06/08 13:31:49  ganter
release 2.0pre9
Added setting of timeperiod in friends mode (see settings menu)

Revision 1.352  2003/06/01 17:27:33  ganter
v2.0pre8
friendsmode works fine and can be set in settings menu

Revision 1.351  2003/05/31 23:39:41  ganter
...

Revision 1.350  2003/05/31 20:32:01  ganter
friendsd2 works fine with sven's server

Revision 1.348  2003/05/29 19:26:19  ganter
testing...

Revision 1.346  2003/05/11 21:15:46  ganter
v2.0pre7
added script convgiftopng
This script converts .gif into .png files, which reduces CPU load
run this script in your maps directory, you need "convert" from ImageMagick

Friends mode runs fine now
Added parameter -H to correct the alitude

Revision 1.345  2003/05/11 11:17:53  ganter
friendsmode is now working fine

Revision 1.344  2003/05/08 21:18:04  ganter
added settings menu entry for fonts setting
made a new cool splash screen
updated da and it translations
v2.0-pre6

Revision 1.343  2003/05/07 22:50:08  ganter
als functions are working (except import)
ready for 2.0pre4

Revision 1.342  2003/05/07 19:27:13  ganter
replaced degree symbol with unicode string
gpsdrive should now be unicode clean

Revision 1.341  2003/05/07 10:52:23  ganter
...

Revision 1.340  2003/05/06 17:49:11  ganter
wp label text is now pango

Revision 1.339  2003/05/03 18:59:47  ganter
shortcuts are now working

Revision 1.338  2003/05/03 03:03:30  ganter
added help window

Revision 1.337  2003/05/02 19:40:43  ganter
changed location of datadir files

Revision 1.336  2003/05/02 18:27:18  ganter
porting to GTK+-2.2
GpsDrive Version 2.0pre3

Revision 1.335  2003/04/28 21:11:03  ganter
1.33pre1

Revision 1.334  2003/04/28 15:42:38  ganter
compiles now with gcc 3.3

Revision 1.333  2003/04/28 15:10:03  ganter
...

Revision 1.332  2003/01/26 20:27:46  ganter
v1.32

Revision 1.331  2003/01/23 14:05:34  ganter
added greek translation
added geocache scripts
added geocache icon
improved search for libmysqlclient.so

Revision 1.330  2003/01/15 19:09:07  ganter
v1.32pre4

Revision 1.329  2003/01/15 17:03:16  ganter
MySQL is now loaded dynamically on runtime, no mysql needed for compile.
Needs only libmysqlclient.so now.

Revision 1.328  2003/01/15 15:30:28  ganter
before dynamically loading mysql

Revision 1.327  2002/12/24 01:46:00  ganter
FAQ

Revision 1.326  2002/12/23 18:39:47  ganter
...

Revision 1.325  2002/12/08 03:18:26  ganter
shortly before 1.31

Revision 1.324  2002/12/08 01:38:26  ganter
perhaps fix for systems without glib-locale installed.
added "y" key to create waypoint at current mouse position.

Revision 1.323  2002/12/07 21:38:46  ganter
delete wp now also works in sqlmode

Revision 1.322  2002/11/29 15:33:45  ganter
v1.31pre3

Revision 1.321  2002/11/27 00:02:27  ganter
1.31pre2

Revision 1.320  2002/11/25 21:56:23  ganter
added icons

Revision 1.319  2002/11/25 20:30:22  ganter
added airport icon

Revision 1.318  2002/11/24 23:36:17  ganter
added icon

Revision 1.317  2002/11/24 17:10:46  ganter
radar R- works again

Revision 1.316  2002/11/24 16:56:29  ganter
speedtrap works now with sql

Revision 1.315  2002/11/24 16:01:32  ganter
added speedtrap icon, thanks to Sven Fichtner

Revision 1.314  2002/11/19 14:49:39  ganter
...

Revision 1.313  2002/11/16 20:23:44  ganter
bugfixes for 1.30

Revision 1.312  2002/11/14 00:05:55  ganter
added README.kismet
v 1.30pre5

Revision 1.311  2002/11/13 18:20:41  ganter
fixed buffer overflow in gpssql.c

Revision 1.310  2002/11/13 17:31:57  ganter
added display of number of waypoints

Revision 1.309  2002/11/12 20:37:15  ganter
v30pre4
added more icons, fix for kismet w/o mysql

Revision 1.308  2002/11/08 23:35:19  ganter
v1.30pre3

Revision 1.307  2002/11/08 22:08:10  ganter
...

Revision 1.306  2002/11/06 05:29:14  ganter
fixed most warnings

Revision 1.305  2002/11/06 01:44:15  ganter
v1.30pre2

Revision 1.304  2002/11/05 17:04:04  ganter
...

Revision 1.303  2002/11/05 00:00:14  ganter
gpskismet seems to work

Revision 1.302  2002/11/02 12:38:55  ganter
changed website to www.gpsdrive.de

Revision 1.301  2002/10/30 09:32:49  ganter
v1.29pre9
hopefully removed all gtk-warnings

Revision 1.300  2002/10/29 16:42:03  ganter
improved configure.in (sql can be disabled)
v1.29pre9

Revision 1.299  2002/10/27 10:51:29  ganter
1.28pre8

Revision 1.298  2002/10/24 08:44:09  ganter
...

Revision 1.297  2002/10/16 14:16:13  ganter
working on SQL gui

Revision 1.296  2002/10/15 07:44:11  ganter
...

Revision 1.295  2002/10/14 08:38:59  ganter
v1.29pre3
added SQL support

Revision 1.294  2002/09/24 08:51:55  ganter
updated translations
changed gpsdrive.spec

Revision 1.293  2002/09/18 13:42:52  ganter
mapblast url changed again
v1.28pre1

Revision 1.292  2002/09/17 20:57:58  ganter
added copyright and README for gpsd
v1.27

Revision 1.291  2002/09/12 10:05:21  ganter
fixed Timeout if getting only GGA data

Revision 1.290  2002/09/10 12:55:25  ganter
v1.27pre1
set battery status update to 5 seconds
added portuguese translation

Revision 1.289  2002/09/01 18:30:06  ganter
fixed segfault on no apm computers

Revision 1.288  2002/08/31 13:10:52  ganter
v1.26 release

Mapblast server works again (they changed the URL).
Bugfix for -a option.
Added -i option to ignore NMEA checksum (for broken GPS receivers).
Added "j" key to switch to next waypoint on route mode.
Added support for festival lite (flite) speech output.

Revision 1.287  2002/08/29 15:20:51  ganter
timezone in settings/geoinfo works, but window must be refreshed

Revision 1.286  2002/08/29 13:11:05  ganter
1.26pre65
Mapblast works again.
Download between 0E and 1W works now in non-degree display mode.
Current speed speech output only when driving faster than 20km/h

Revision 1.285  2002/08/29 11:17:40  ganter
will change mapblast URL

Revision 1.284  2002/08/05 19:16:27  ganter
fixed bug in mapdownload (triggered thru change to HTTP1.1)

Revision 1.283  2002/08/04 12:24:57  ganter
Current speed speech output only when driving faster than 20km/h

Revision 1.282  2002/07/30 21:33:53  ganter
added "J" key to switch to next waypoint

Revision 1.281  2002/07/30 21:01:30  ganter
added patches from Marco Molteni for separate track.c

Revision 1.280  2002/07/30 20:49:54  ganter
1.26pre3
added support for festival lite (flite)
changed http request to HTTP1.1 and added correct servername

Revision 1.279  2002/07/17 12:31:54  ganter
v1.25

Revision 1.278  2002/07/14 18:22:19  ganter
v1.25pre1

Revision 1.277  2002/07/14 14:23:34  ganter
testnewmap works now again, but w/o new algorithmus

Revision 1.276  2002/07/13 23:40:17  ganter
changed testnewmap algorithmus

Revision 1.275  2002/07/13 13:31:41  ganter
in download menu the download area is now marked

Revision 1.274  2002/07/02 12:12:55  ganter
v1.24

Revision 1.273  2002/07/01 00:45:00  ganter
added trip info (in settings menu)
ACPI fixes (close battery fd)

Revision 1.272  2002/06/30 11:32:28  ganter
fix convertRMC
new arrows

Revision 1.271  2002/06/30 09:38:17  ganter
make fields in convertXXX larger, earthmate seems to send larger GPGSV
v1.24pre1

Revision 1.270  2002/06/29 21:47:38  ganter
v1.23

Revision 1.269  2002/06/29 00:28:21  ganter
v1.23pre10

Revision 1.268  2002/06/29 00:23:18  ganter
added ACPI support for battery meter

Revision 1.267  2002/06/23 17:09:35  ganter
v1.23pre9
now PDA mode looks good.

Revision 1.266  2002/06/23 08:57:50  ganter
changed menu look for -x

Revision 1.265  2002/06/16 21:35:08  ganter
got settings smaller

Revision 1.264  2002/06/16 17:50:34  ganter
working on PDA screen

Revision 1.263  2002/06/12 10:58:49  ganter
v1.23pre7

Revision 1.262  2002/06/10 18:04:22  ganter
added compass

Revision 1.261  2002/06/02 20:54:09  ganter
added navigation.c and copyrights

Revision 1.260  2002/06/02 18:46:30  ganter
v1.23pre6
did lot of bug fixing for small displays.

Revision 1.259  2002/06/01 15:42:31  ganter
fixed bug for little screens
added new wpget from Miguel Angelo Rozsas <miguel@rozsas.xx.nom.br>

Revision 1.258  2002/06/01 15:24:42  ganter
working on bugfix for small screens

Revision 1.257  2002/05/30 19:10:55  ganter
v1.23pre4
use GPGGA if no GPRMC is available

Revision 1.256  2002/05/29 20:41:41  ganter
added swedish translation

Revision 1.255  2002/05/29 19:21:01  ganter
added gpsfetchmap.pl and geocache2way

Revision 1.254  2002/05/23 09:07:37  ganter
v1.23pre1
added new BSD battery stuff

Revision 1.253  2002/05/20 22:41:02  ganter
removed race condition in setutc()
new 1.22

Revision 1.252  2002/05/20 20:40:23  ganter
v1.22

Revision 1.251  2002/05/20 10:02:48  ganter
v1.22pre7

Revision 1.250  2002/05/19 19:25:16  ganter
fly and nautic loading works

Revision 1.249  2002/05/18 20:58:53  ganter
finished geo infos
v1.22pre5

Revision 1.248  2002/05/17 23:22:25  ganter
added sunrise,sunset in settings menu

Revision 1.247  2002/05/17 20:05:23  ganter
changed settings menu to notebook widget

Revision 1.246  2002/05/15 19:05:31  ganter
created settings.c

Revision 1.245  2002/05/13 10:01:47  ganter
fixed bug in downloadsetparm if not decimal notation
new v1.21

Revision 1.244  2002/05/12 23:14:53  ganter
new 1.21
changed B to N key.

Revision 1.243  2002/05/12 20:54:08  ganter
v1.21

Revision 1.242  2002/05/12 19:52:39  ganter
added nightmode. See settings menu.

Revision 1.241  2002/05/11 15:45:31  ganter
v1.21pre1
degree,minutes,seconds should work now

Revision 1.240  2002/05/10 00:20:14  ganter
display of lat/long is switchable between decimal and degree,minutes
and seconds display
added display of radar warning as scrolling text
distance to recognize arriving of the destination is now speed
depending
added check of the checksum of the NMEA sentences to avoid crashes if
invalid NMEA sentences are received

Revision 1.239  2002/05/04 09:17:36  ganter
added new intl subdir

Revision 1.238  2002/05/02 01:34:11  ganter
added speech output of waypoint description

Revision 1.237  2002/05/02 00:31:21  ganter
speech output: say reached target also if not in route mode

Revision 1.236  2002/05/01 21:44:38  ganter
added README.FreeBSD

Revision 1.235  2002/04/29 02:52:18  ganter
v1.20pre1
added display of sat position

Revision 1.234  2002/04/28 22:21:13  ganter
new 1.19 ;-)

Revision 1.233  2002/04/28 21:54:20  ganter
v1.19

Revision 1.232  2002/04/28 11:53:43  ganter
v1.19pre2
button to delete waypoint
fixed miles distance on startup in miles mode
new sat level display colors

Revision 1.231  2002/04/21 13:29:30  ganter
fixed bug in downloadsetparms (longitude comparision)

Revision 1.230  2002/04/21 13:15:02  ganter
Now getting good maps for USA from expedia server works: I changed in
the URL EUR0809 to USA0409 if longitude is west of 30ÂŽ°W. If anyone
have found a system for this EUR0809,USA0409 strings in the URL,
please inform me.

Revision 1.229  2002/04/21 00:27:22  ganter
v1.18pre3
much more precise calculation of distance (uses WGS84 elipsoid)

Revision 1.228  2002/04/18 22:50:37  ganter
added turkish translation
added display number of satellites
v1.18pre2

Revision 1.227  2002/04/16 21:08:50  ganter
...

Revision 1.226  2002/04/14 23:06:26  ganter
v1.17

Revision 1.225  2002/04/14 15:25:56  ganter
v1.17pre3
added simulaton follow switch in setup menu

Revision 1.224  2002/04/13 18:08:30  ganter
added comment

Revision 1.223  2002/04/13 17:49:44  ganter
...

Revision 1.222  2002/04/13 17:30:35  ganter
v1.17pre2

Revision 1.221  2002/04/12 14:10:19  ganter
removed way.txt checking and popup, if there are wrong entries, the
entries are ignored, not the whole file.

Revision 1.220  2002/04/10 21:32:55  ganter
sometimes wrong coordinates in download map window, fixed.

Revision 1.219  2002/04/07 19:55:54  ganter
bugfix in gpsdrive.c and gpsfetchmap

Revision 1.218  2002/04/07 17:15:31  ganter
v1.16

Revision 1.217  2002/04/07 12:23:41  ganter
v1.16pre9

Revision 1.216  2002/04/07 00:47:42  ganter
added fly.c
added more GPS-receivers

Revision 1.215  2002/04/06 19:14:42  ganter
I found a better documentation for GARMIN receivers. So I removed DOP
and added EPE (estimated position error). Sorry, if you have no
GARMIN. The used NMEA sentence is $PGRME.

Revision 1.214  2002/04/06 17:12:11  ganter
changed #elif to #else

Revision 1.213  2002/04/06 17:08:56  ganter
v1.16pre8
cleanup of gpsd files

Revision 1.212  2002/04/06 13:14:04  ganter
removed feature to set posmode on with left mouseclick in the map
window. It has to switched on with the toogle button in the menu.

Revision 1.211  2002/04/06 00:36:01  ganter
changing filelist

Revision 1.210  2002/04/05 23:20:51  ganter
added DGPS displaying
added DOP (DILUTION OF PRECISION):
A measure of the GPS receiver-satellite geometry. A low DOP value
indicates better relative geometry and correspondingly higher
accuracy.

Revision 1.209  2002/04/05 19:05:48  ganter
changed to Remco Treffkorn's gpsd

Revision 1.208  2002/04/04 22:34:07  ganter
v1.16pre7
added altitude in stored tracks
added zoom factor display on map

Revision 1.207  2002/04/03 10:20:31  ganter
added altitude display

Revision 1.206  2002/04/03 08:57:25  ganter
trying to get out altitude

Revision 1.205  2002/04/02 19:46:15  ganter
v1.16pre4
you can type in coordinates in the "Add waypoint" window (x-key)
Autosave of configuration
update spanish translation (translater had holiday)

Revision 1.204  2002/04/01 22:39:04  ganter
v1.16pre3

Revision 1.203  2002/04/01 22:32:04  ganter
added garmin and serialdevice in setup

Revision 1.202  2002/04/01 14:38:13  ganter
really upload new 1.15 and v1.16-pre2
added penguin
fixed gpsd detection
variable testgarmin inserted, set it to FALSE if you have problems with your
NMEA receiver detection.
don't forget to call "gpsdrive -t /dev/ttySx" the first time, where x is
your port number (0=COM1:)

Revision 1.201  2002/04/01 14:13:38  ganter
v1.16-pre2
serialdev fix was not good enough

Revision 1.200  2002/04/01 12:55:45  ganter
I decided to create a new 1.15 from 1.16-pre1 because if the serialdev bug.

Revision 1.199  2002/04/01 12:43:12  ganter
v1.16-pre1 bugfix for serialdev if no gpsdriverc exists yet

Revision 1.198  2002/04/01 02:35:13  ganter
added -b parameter for NMEA server
added -c parameter to set position in simulation mode to waypoint name

Revision 1.197  2002/03/31 20:42:55  ganter
v1.15 Summary:
You can select your "way*.txt" file in setup menu.
	   The DEFAUL entry in way.txt is now obsolet.
	   The "setdefaultpos" entry in gpsdricerc in now obsolet.
	   Added battery meter, shows battery capacity and battery/plugged mode
	   (only shown on notebooks).
	   Removed command line parameter: -w.
	   Added command line parameter: -a , use it if gpsdrive
	   crashes (happens on broken apm BIOSes).
	   Removed popup to start GPSD, its now a button.
	   Added new unit "nautical miles".
	   Moved buttons to setup menu, setup menu with new options.

Revision 1.196  2002/03/31 18:11:06  ganter
v1.15-pre5
you can select your "way*.txt" file in setup menu
The DEFAUL entry in way.txt is now obsolet
The "setdefaultpos" entry in gpsdricerc in now obsolet

Revision 1.195  2002/03/31 16:41:25  ganter
The "DEFAULT" waypoint is now obsolet. All references to it will now
be removed.

Revision 1.194  2002/03/31 14:30:23  ganter
working on chooseable waypoint files

Revision 1.193  2002/03/30 20:07:33  ganter
2nd pre 1.15

Revision 1.192  2002/03/30 10:37:43  ganter
pre 1.15

Revision 1.191  2002/03/30 00:42:58  ganter
created setup menu

Revision 1.190  2002/03/29 23:09:06  ganter
added battery meter, shows battery capacity and battery/plugged mode

Revision 1.189  2002/03/24 17:45:26  ganter
v1.14 some cosmetic changes

Revision 1.188  2002/03/23 18:38:38  ganter
v1.13

Revision 1.187  2002/03/23 18:34:02  ganter
added better background for waypoint text, wp text has now a bold font

Revision 1.186  2002/03/22 00:36:40  ganter
bugfix for late gpsd start
remembering last position if setdefaultpos = 0 in gpsdriverc

Revision 1.185  2002/03/21 21:49:08  ganter
added question if gpsd should be started.
Thanks to daZwerg(gEb-Dude) for suggestion.

Revision 1.184  2002/03/17 23:14:06  ganter
v1.12

Revision 1.183  2002/03/17 19:13:15  ganter
added route
v1.12 preview

Revision 1.182  2002/03/16 20:21:07  ganter
fixed segfault if no gpsdriverc exists. New v1.11

Revision 1.181  2002/03/16 19:17:59  ganter
v1.11

Revision 1.180  2002/03/16 19:15:10  ganter
added gpsdriverc file to save and restore settings
added shadow to all drawn elements on map
removed -m flag for setting miles because it is saved in gpsdriverc

Revision 1.179  2002/03/14 23:54:16  ganter
added speech output of target set
use HTTP_PROXY or http_proxy for enviroment variable to set proxy server

Revision 1.178  2002/03/10 18:50:52  ganter
minimap is now clickable for switch to position mode
parameter -1 for 1 button mouse, i.e. touchpads
viewable satellites with 0db Signal are shown als short red bar

Revision 1.177  2002/03/03 15:31:52  ganter
posmode is switched off after 10 seconds automatically

Revision 1.176  2002/02/27 12:00:13  ganter
download default is now expedia server

Revision 1.175  2002/02/26 15:34:17  ganter
added x key for set waypoint on actual position

Revision 1.174  2002/02/25 01:45:38  ganter
bugfix for loading tracks. now date/time is also loaded

Revision 1.173  2002/02/24 22:26:41  ganter
new v1.9, added shortcuts

Revision 1.172  2002/02/24 20:15:05  ganter
prevent to call target window more than once

Revision 1.171  2002/02/24 19:38:44  ganter
Real, real v1.9, fixed division by zero handling

Revision 1.170  2002/02/24 19:16:27  ganter
Real v1.9, added  -ffast-math because of better DIVZERO handling in code

Revision 1.169  2002/02/24 16:46:04  ganter
v1.9

Revision 1.168  2002/02/24 15:58:24  ganter
the "select waypoint" window is now auto-updated and shows every 5
seconds the true distance to the waypoints.

Revision 1.167  2002/02/23 23:47:55  ganter
added set waypoint at current position by CTRL-right mouse click

Revision 1.166  2002/02/18 01:37:18  ganter
added set waypoint by CTRL-left mouse click and storing into way.txt

Revision 1.165  2002/02/17 23:35:29  ganter
perhaps bugfix for black maps
added reread of way.txt if file is changed

Revision 1.164  2002/01/11 16:38:14  ganter
Changed URL for map webservers. A lot of thanks to Oliver Kuehlert <Oliver.Kuehlert@mpi-hd.mpg.de>!

Revision 1.163  2001/12/12 23:06:48  ganter
added changes from Richard Scheffenegger <rscheff@chello.at>

Revision 1.162  2001/12/02 21:41:44  ganter
fixed bug in setlocale

Revision 1.161  2001/12/02 17:40:37  ganter
friends mode bugfix. Enhanced -x option

Revision 1.160  2001/11/16 20:12:19  ganter
v1.5

Revision 1.159  2001/11/16 18:00:54  ganter
tcpserver/client works

Revision 1.158  2001/11/13 18:37:46  ganter
v1.4 enhanced friends functions

Revision 1.157  2001/11/12 19:55:47  ganter
v1.3

Revision 1.156  2001/11/12 19:11:53  ganter
friends server and client starting to work

Revision 1.155  2001/11/11 17:25:34  ganter
added friendsd

Revision 1.154  2001/11/04 19:38:32  ganter
autodetects setting for voice output, cosmetic changes

Revision 1.153  2001/11/03 23:27:00  ganter
moved manpages to man directory

Revision 1.152  2001/11/03 19:22:07  ganter
next public v1.1, made testnewmap more accurate

Revision 1.151  2001/11/03 19:10:05  ganter
new layout, some buttons are now checkboxes

Revision 1.150  2001/11/03 12:52:12  ganter
v1.1, added best map button and next more/less detailed map button

Revision 1.149  2001/11/02 00:42:32  ganter
real v1.0, changed manpage

Revision 1.148  2001/11/02 00:10:02  ganter
v1.0 for public

Revision 1.147  2001/11/01 20:17:58  ganter
v1.0 added spanish voice output

Revision 1.146  2001/11/01 11:36:39  ganter
added festival init for german and english. See manpage

Revision 1.145  2001/10/29 15:26:20  ganter
added -o

Revision 1.144  2001/10/29 12:43:23  ganter
added -o option to output NMEA sentences. Written by Dan Egnor <egnor@ofb.net>

Revision 1.143  2001/10/28 17:17:38  ganter
v0.35

Revision 1.142  2001/10/28 15:54:56  ganter
v0.35 added load and store track

Revision 1.141  2001/10/27 16:05:03  ganter
autodetected screen size

Revision 1.140  2001/10/26 17:58:09  ganter
added auto detecting screen size

Revision 1.139  2001/10/22 23:37:23  ganter
v 0.34 added proxy server

Revision 1.138  2001/10/21 18:12:51  ganter
v0.33

Revision 1.137  2001/10/21 16:44:10  ganter
bugfix in position calculation for big maps.
pre 0.33

Revision 1.136  2001/10/21 14:38:09  ganter
found buffer overflow in get_position_data_cb only in NMEA mode, result was wrong text translations.

Revision 1.135  2001/10/21 03:06:30  ganter
new position calculation looks good, but in progress...

Revision 1.134  2001/10/19 23:30:11  ganter
track parts are not drawn if no GPS Fix is avail. Should work...

Revision 1.133  2001/10/16 14:11:40  ganter
added po files

Revision 1.132  2001/10/14 14:35:38  ganter
really V0.32 expedia server works now

Revision 1.131  2001/10/14 13:09:11  ganter
working on expedia scaling

Revision 1.130  2001/10/14 11:26:04  ganter
Program now creates a ~/.gpsdrive directory if it was not found. Also
it creates a map_koord.txt in it.

Revision 1.129  2001/10/14 10:41:59  ganter
v0.32 correct path for prefix other than /usr/local

Revision 1.128  2001/10/13 23:04:56  ganter
corrected paths for locale and pixmap, ./configure --prefix= works now as
expected

Revision 1.127  2001/10/13 15:33:18  ganter
test

Revision 1.126  2001/10/09 13:14:02  ganter
v0.31

Revision 1.125  2001/10/08 23:36:17  ganter
added download from expedia.com, but it don't work yet.
Reason: Server don't send CONTENT-LENGTH

Revision 1.124  2001/10/08 12:14:05  ganter
radar detection stopped working, runs now again

Revision 1.123  2001/10/08 02:24:39  ganter
fixed bug in testconfig_cb

Revision 1.122  2001/10/07 20:06:08  ganter
detects wrong names in map_koord.txt

Revision 1.121  2001/10/07 16:30:09  ganter
v0.30

Revision 1.120  2001/10/07 15:19:55  ganter
removed some debugging lines

Revision 1.119  2001/10/07 15:17:29  ganter
import seems to work

Revision 1.118  2001/10/07 00:59:13  ganter
map import nearly finished

Revision 1.117  2001/10/04 19:26:59  ganter
added rectangle on minimap
working on map import

Revision 1.116  2001/09/30 18:45:27  ganter
v0.29
added choice of map type

Revision 1.115  2001/09/30 15:54:32  ganter
added minimap, removed gpsdrivelogo

Revision 1.114  2001/09/30 12:08:11  ganter
added parameter -x to use a seperate window for the menu
some changes in speech output
added help button

Revision 1.113  2001/09/28 15:43:56  ganter
v0.28 changed layout, some bugfixes

Revision 1.112  2001/09/27 16:52:29  ganter
....

Revision 1.111  2001/09/25 23:49:43  ganter
v 0.27

Revision 1.110  2001/09/25 23:32:59  ganter
some bugfixes for tracking

Revision 1.109  2001/09/25 20:58:13  ganter
tracking is working

Revision 1.108  2001/09/25 17:00:32  ganter
fallback font strings corrected

Revision 1.107  2001/09/24 19:42:20  ganter
removed -s -g -f command line options. GpsDrive tries first port 2222,
then port 2947 and if not found the GARMIN mode. If this is also not
found it switches to simulation mode.

Revision 1.106  2001/09/24 19:00:46  ganter
added support for gpsd by  Remco Treffkorn using port 2947

Revision 1.105  2001/09/23 22:31:13  ganter
v0.26

Revision 1.104  2001/09/23 22:04:03  ganter
Added -t and -l command line switches, see gpsdrive -h
Added german speech texts.
Bugfix in radar detection. Only the nearest Radar gives speech output.
Cosmetic changes for markers.
#define MAXSHOWNWP 100 for max. shown waypoints. Tested with list of
30000 waypoints. If gpsdrive hangs, reduce number of waypoints in
file.
Better fallbacks for fonts. I prefer an arial truetype font. Helvetica
is used if no arial font is found.

Revision 1.103  2001/09/23 15:06:32  ganter
....

Revision 1.102  2001/09/23 00:58:59  ganter
....

Revision 1.101  2001/09/22 23:35:00  ganter
Added program icon. Added check for way.txt format errors.

Revision 1.100  2001/09/22 16:22:47  ganter
solved this intl compile problem

Revision 1.99  2001/09/21 23:16:20  ganter
v0.25

Revision 1.98  2001/09/21 23:11:07  ganter
added error message for not existent DEFAULT waypoint
added popup error window

Revision 1.97  2001/09/21 21:32:20  ganter
In the way.txt waypoint file the waypoint
named "DEFAULT" is the start position of the program,
important if you start it in simulation mode.
So not everybody in the world need to start at my
house in Austria! ;-)
Bugfix if at start no map is found.

Revision 1.96  2001/09/21 10:47:06  ganter
V0.24: bugfix for displays which have not 16 bit colordepth

Revision 1.95  2001/09/20 15:07:44  ganter
added Radar warning. If you store the radars as waypoints named R-XXXX where
XXXX can be a incremented number. You hear radar warning as voice message and
a red/black blinking Bearing pointer.

Revision 1.94  2001/09/18 23:02:03  ganter
v0.22

Revision 1.93  2001/09/18 22:56:10  ganter
If you click with the left mouse button on the map you are in "Display
mode" where a rectangle is the cursor and no position is shown. If you
zoom or select another map scale, this is done for the position of the
rectangle-cursor in the same manner as it would be your actual
position.

The middle mouse button switches back to normal mode. The same if you
select a target with the right mouse button.  Shift-left-mouse-button
and shift-right-mouse-button changes the map scale.

Revision 1.92  2001/09/18 21:44:42  ganter
testnewmap is not o.k.

Revision 1.91  2001/09/18 13:40:13  ganter
v0.21 using double buffering to avoid flicker.

Revision 1.90  2001/09/18 05:33:06  ganter
...

Revision 1.89  2001/09/17 19:03:54  ganter
changed speech text

Revision 1.88  2001/09/17 00:29:38  ganter
added speech output of bearing

Revision 1.87  2001/09/16 21:36:05  ganter
speech output is working

Revision 1.86  2001/09/16 19:12:35  ganter
....

Revision 1.84  2001/09/16 18:42:32  ganter
now we work on speech output

Revision 1.83  2001/09/09 17:31:36  ganter
v0.18 added defines to make it possible to define smaller screen sizes
this is the way to porting for iPaq

Revision 1.82  2001/09/09 15:30:39  ganter
numbers changed to SCREEN_X and SCREEN_Y, but doesn't work yet

Revision 1.81  2001/09/09 14:54:44  ganter
beginning rewrite source to set screenwidth and height as define

Revision 1.80  2001/09/09 08:07:11  ganter
v0.17

Revision 1.79  2001/09/08 18:49:11  ganter
added + sign as marker for the selected target

Revision 1.78  2001/09/08 13:20:58  ganter
added tooltips

Revision 1.77  2001/09/06 22:00:53  ganter
added: click with right mouse button on map sets the target waypoint

Revision 1.76  2001/09/06 12:19:43  ganter
changed strings

Revision 1.75  2001/09/06 08:51:57  ganter
changed label order, now really v0.15

Revision 1.74  2001/09/06 08:46:35  ganter
some string changes, really v0.15

Revision 1.73  2001/09/06 00:16:02  ganter
display_status

Revision 1.72  2001/09/06 00:05:50  ganter
new fields layout

Revision 1.71  2001/09/05 22:03:26  ganter
scaler works

Revision 1.70  2001/09/05 19:33:06  ganter
created scaler for map scale selection

Revision 1.69  2001/09/04 22:15:59  ganter
....

Revision 1.68  2001/09/04 21:30:48  ganter
added time to destination. I didn't calculate with bearing.

Revision 1.67  2001/09/04 19:55:40  ganter
moved progress bar into the download window
download window is only removed after download
added delete_event handler for download_cb

Revision 1.66  2001/09/04 12:43:15  ganter
added progress bar for download status

Revision 1.65  2001/09/04 01:20:23  ganter
restore cursor on end of download

Revision 1.64  2001/09/04 00:39:57  ganter
added cool cursor for map position selection

Revision 1.63  2001/09/03 23:51:11  ganter
....

Revision 1.62  2001/09/03 23:48:16  ganter
translations

Revision 1.61  2001/09/03 23:32:34  ganter
public v0.13

Revision 1.60  2001/09/03 23:04:53  ganter
map is now clickable for selection of the download position.
yardstick is fixed for m/yards display.

Revision 1.59  2001/09/03 15:42:56  ganter
missing translation

Revision 1.58  2001/09/03 15:36:39  ganter
added scale marker on map

Revision 1.57  2001/09/03 01:20:32  ganter
added scale line, it's not perfect yet

Revision 1.56  2001/09/02 18:48:11  ganter
near to v0.12

Revision 1.55  2001/09/02 17:09:32  ganter
download of gif doesn't work correctly sometimes

Revision 1.54  2001/09/02 14:39:15  ganter
....

Revision 1.53  2001/09/01 18:15:57  ganter
download in a non-blocking timeout routine

Revision 1.52  2001/09/01 15:50:43  ganter
working on downloading maps from internet

Revision 1.51  2001/08/31 19:53:48  ganter
rearranged calls of draw_marker.
fixed little bug in zoom-out, works now like expected.

Revision 1.50  2001/08/31 09:56:11  ganter
fixed bug if you are West or South. Thanks to Jason Aras.

Revision 1.49  2001/08/31 01:29:43  ganter
version 0.10

Revision 1.48  2001/08/30 01:24:43  ganter
better simulator

Revision 1.47  2001/08/30 00:08:01  ganter
more beautifully indent. Thanks to timecop@japan.co.jp

Revision 1.46  2001/08/29 23:13:44  ganter
version 0.9

Revision 1.45  2001/08/29 21:58:13  ganter
code cleanup, translated to english

Revision 1.44  2001/08/29 19:07:08  ganter
main() code rewritten for better readability because japanese people
complained about bad style coding.
Also translate all variable and messages to english.

Revision 1.43  2001/08/29 14:00:42  ganter
changed variable names to english

Revision 1.42  2001/08/27 21:40:46  ganter
get groundspeed to zero if no movement

Revision 1.41  2001/08/27 13:24:37  ganter
bugfix: . or , should be set depending on  LC_NUMERIC

Revision 1.40  2001/08/27 02:57:18  ganter
Version 0.7

Revision 1.39  2001/08/27 01:03:24  ganter
Version 0.7 added experimental -f flag for direct use of GARMIN format
no gpsd must be started!

Revision 1.38  2001/08/26 23:40:10  ganter
added garble sources to included in gpsdrive

Revision 1.37  2001/08/26 19:33:45  ganter
indent

Revision 1.36  2001/08/26 19:31:53  ganter
"distance to" frame label shows target name

Revision 1.35  2001/08/26 17:46:57  ganter
changed usage, still v0.6

Revision 1.34  2001/08/26 17:38:12  ganter
Version 0.6

Revision 1.33  2001/08/26 17:24:09  ganter
added big zoom field, set refresh rate to 500ms

Revision 1.32  2001/08/26 15:49:40  ganter
indent

Revision 1.31  2001/08/26 15:47:45  ganter
added splash(), coooool!

Revision 1.30  2001/08/25 13:39:33  ganter
*** empty log message ***

Revision 1.29  2001/08/25 12:53:09  ganter
pre v0.5

Revision 1.28  2001/08/25 08:56:53  ganter
added frames for fields

Revision 1.27  2001/08/24 21:34:43  ganter
simulator mode: pointer moves to selected destination

Revision 1.26  2001/08/24 16:56:17  ganter
working on satellites

Revision 1.25  2001/08/24 09:58:57  ganter
course pointer is something ugly

Revision 1.24  2001/08/24 00:25:29  ganter
corrected angel values, 0ÂŽ° is on top (north) and angel is counting clockwise

Revision 1.23  2001/08/23 00:08:11  ganter
added pointer to destination

Revision 1.22  2001/08/22 09:31:37  ganter
changes to way.txt handling

Revision 1.21  2001/08/22 08:45:58  ganter
version 0.4 for public

Revision 1.20  2001/08/22 00:10:19  ganter
distance measurment adjusted

Revision 1.19  2001/08/21 23:19:00  ganter
destination selection added

Revision 1.18  2001/08/20 15:35:45  ganter
little bug in zoom out found, xoff and yoff was not updated.

Revision 1.17  2001/08/19 16:17:08  ganter
Version 0.3 ready for public

Revision 1.16  2001/08/19 15:17:31  ganter
some cleanups

Revision 1.15  2001/08/19 14:26:19  ganter
fixed offset=0 when new map loaded.

Revision 1.14  2001/08/19 13:55:01  ganter
found this little bug (x instead of y in line 469).
Am I too old for programming?

Revision 1.13  2001/08/19 11:58:44  ganter
there is only a big bug in the y direction.

Revision 1.12  2001/08/19 00:34:01  ganter
working hard on zooming...

Revision 1.11  2001/08/18 20:28:02  ganter
working on zooming

Revision 1.10  2001/08/18 15:35:03  ganter
added command line options

Revision 1.9  2001/08/18 03:58:12  ganter
map switching works (theroretical, I need a testdrive, now at 6.00am localtime)

Revision 1.8  2001/08/18 00:08:12  ganter
"big" image is working, simulated_pos() simulates moving of map.

Revision 1.7  2001/08/16 23:17:42  ganter
displaying position on map is working and tested through test drive ;-)

Revision 1.6  2001/08/16 20:03:49  ganter
should work

Revision 1.5  2001/08/15 22:39:45  ganter
new version

Revision 1.4  2001/08/15 12:06:03  ganter
data conversion for statusline works

Revision 1.3  2001/08/15 00:45:24  ganter
get GPS data and display in statusbar is working

Revision 1.2  2001/08/14 18:53:37  ganter
first version running

Revision 1.1.1.1  2001/08/14 13:52:20  ganter
gpsdrive started

***********************************************************************/

/*  Include Dateien */
#include "../config.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include <sys/time.h>
#include <compass.h>
#include <errno.h>

#if HAVE_LOCALE_H
#include <locale.h>
#else
# define setlocale(Category, Locale)
#endif
#include "gettext.h"


#include "LatLong-UTMconversion.h"
#include "gpsdrive.h"
#include <dirent.h>
#include "battery.h"
#include "track.h"
#include "icons.h"
#include <dlfcn.h>
#include <pthread.h>
#include <semaphore.h>

#ifndef NOPLUGINS
#include "gmodule.h"
#endif
#ifdef USETELEATLAS
#include "gpsnavlib.h"
#endif

/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif


#if GTK_MINOR_VERSION < 2
#define gdk_draw_pixbuf _gdk_draw_pixbuf
#endif

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

/*  global variables */
gint statusid, messagestatusbarid, timeoutcount;
GtkWidget *mainwindow, *status, *messagestatusbar, *pixmapwidget, *gotowindow,
  *messagewindow, *routewindow, *downloadbt;
extern GtkWidget *splash_window;
gint sock = -1, dlsock = -1, bigp = 0, bigpGSV = 0, bigpGGA = 0, bigpRME =
  0, bigpGSA = 0;
gint lastpRME = 0, lastpGSA = 0, lastp = 0, lastpGSV = 0, lastpGGA =
  0, debug = 0;
FILE *nmeaout = NULL;
gchar *buffer = NULL, *big = NULL;
fd_set readmask;
struct timeval timeout;
gchar targetname[40];
gdouble current_long, current_lat, old_long, old_lat, groundspeed;
gdouble zero_long, zero_lat, target_long, target_lat, dist;
gdouble long_diff = 0, lat_diff = 0;
GdkGC *kontext;
GdkColor red = { 0, 0xFF00, 0x00, 0x00 };
GdkColor black = { 0, 0x00, 0x00, 0x00 };
GdkColor white = { 0, 0xFFFF, 0xFFFF, 0xFFFF };
GdkColor blue = { 0, 0x00, 0x00, 0xff00 };

//GdkColor nightcolor = { 0, 0x0000, 0xa000, 0x0000 };

GdkColor nightcolor = { 0, 0xa000, 0x0000, 0x0000 };
GdkColor lcd = { 0, 0x8b00, 0x9500, 0x8b00 };
GdkColor lcd2 = { 0, 0x7300, 0x7d00, 0x6a00 };
GdkColor yellow = { 0, 0xff00, 0xff00, 0x00 };
GdkColor green = { 0, 0x0000, 0xb000, 0x0000 };
GdkColor green2 = { 0, 0x0000, 0xff00, 0x0000 };
GdkColor mygray = { 0, 0xd500, 0xd600, 0xd500 };
GdkColor textback = { 0, 0xa500, 0xa600, 0xa500 };
GdkColor textbacknew = { 0, 0x4000, 0x7600, 0xcf00 };
GdkColor grey = { 0, 0xC000, 0xC000, 0xC000 };
GdkColor defaultcolor;

/* GdkColor orange = { 0, 0xfb00, 0x8d00, 0x0a00 }; */
GdkColor orange = { 0, 0xf000, 0x6000, 0x0000 };
GdkColor darkgrey = { 0, SHADOWGREY, SHADOWGREY, SHADOWGREY };

GtkWidget *drawing_area, *drawing_bearing, *drawing_sats, *drawing_miniimage;
GdkColor orange2 = { 0, 0xff00, 0x8000, 0x0000 };
GtkWidget *drawing_battery, *drawing_temp;
GtkWidget *distlabel, *speedlabel, *altilabel, *miles, *startgpsbt;
GdkDrawable *drawable, *drawable_bearing, *drawable_sats;
gint havepos, haveposcount, blink, gblink, xoff, yoff, crosstoogle = 0;
gdouble pixelfact, posx, posy, angle_to_destination, direction, bearing;
GdkPixbuf *image = NULL, *friendsimage = NULL, *tempimage =
  NULL, *friendspixbuf = NULL, *miniimage = NULL;
typedef struct
{
  gchar filename[200];
  gdouble lat;
  gdouble lon;
  glong scale;
}
mapsstruct;
mapsstruct *maps = NULL;

/* action=1: radar (speedtrap) */

typedef struct
{
  gchar name[40];
  gdouble lat;
  gdouble lon;
  gdouble dist;
  gchar typ[40];
  gint wlan;
  gint action;
  gint sqlnr;
  gint proximity;
}
wpstruct;
wpstruct *wayp, *routelist;
friendsstruct *friends, *fserver;
char friendsserverip[20], friendsname[40], friendsidstring[40],
  friendsserverfqn[255];


gchar *dlpstart;
gchar oldfilename[200];
GString *tempmapfile;
gint nrmaps = 0, dldiff;
gdouble earthr;
/*Earth radius calibrated with GARMIN III */
/*  gdouble R = 6383254.445; */


#define R earthr


/*  if we get data from gpsd in NMEA format haveNMEA is TRUE */
/*  haveGARMIN is TRUE if we get data from program garble in GARMIN we get only long and lat */
gint haveNMEA, haveGARMIN;

gint maploaded, simmode, zoom, iszoomed;
gchar homedir[500], mapdir[500];
static gchar const rcsid[] =
  "$Id$";
gint thisline, thisrouteline = 0, routeitems = 0, routepointer = 0;
gint maxwp, maxfriends = 0;
GtkStyle *style = NULL;
GtkRcStyle *mainstyle;
gint satlist[MAXSATS][4], satlistdisp[MAXSATS][4], satbit = 0;
GtkWidget *mylist, *myroutelist, *destframe;
gchar mapfilename[200];
gdouble gbreit, glang, milesconv, olddist = 99999.0;
GTimer *timer, *disttimer;
gint gcount, milesflag, downloadwindowactive;
gint metricflag, nauticflag;
struct lconv *mylocale;
gchar *dlbuff, downloadfilename[512], localedecimal;
gint scaleprefered = 1, scalewanted = 100000;
gint dlcount, downloadactive, importactive, downloadfilelen;
glong mapscale;
GtkWidget *dltext1, *dltext2, *dltext3, *dltext4, *wptext1, *wptext2;
gdouble wplat, wplon;
GtkWidget *dltext5, *dltext6, *dltext7, *myprogress;
GtkWidget *scaler, *downloadwindow;
gchar writebuff[2000];
gchar newmaplat[100], newmaplongi[100], newmapsc[100], oldangle[100];
GdkCursor *cursor;
gchar *slist[] = { "1000", "1500", "2000", "3000", "5000", "7500",
  "10000", "15000", "20000", "30000", "50000", "75000",
  "100000", "150000", "200000", "300000", "500000", "750000",
  "1000000", "1500000", "2000000", "3000000", "5000000", "7500000",
  "10000000", "15000000", "20000000", "30000000", "50000000", "75000000"
};
gint nlist[] = { 1000, 1500, 2000, 3000, 5000, 7500,
  10000, 15000, 20000, 30000, 50000, 75000,
  100000, 150000, 200000, 300000, 500000, 750000,
  1000000, 1500000, 2000000, 3000000, 5000000, 7500000,
  10000000, 15000000, 20000000, 30000000, 50000000, 75000000
};
GtkWidget *l1, *l2, *l3, *l4, *l5, *l6, *l7, *l8, *mutebt, *sqlbt;
GtkWidget *trackbt, *wpbt;
GtkWidget *bestmapbt, *maptogglebt, *topotogglebt, *savetrackbt;
GtkWidget *loadtrackbt, *radio1, *radio2, *scalerlbt, *scalerrbt;
GtkWidget *setupbt;
gint savetrack = 0, havespeechout, hours, minutes, speechcount = 0;
gint muteflag = 0, sqlflag = 0, trackflag = 1, wpflag = TRUE;
gint displaymap_top = TRUE, displaymap_map = TRUE;
gint posmode = 0;
gdouble posmode_x, posmode_y;
GtkObject *adj;
gchar lastradar[40], lastradar2[40], nmeamodeandport[50], importfilename[80];
gint foundradar, messageshown = FALSE;
gdouble radarbearing;
gint errortextmode = TRUE;
enum
{ english, german, spanish }
voicelang;
gchar serialdev[80];
GdkSegment *track, *trackshadow;
glong tracknr;
trackcoordstruct *trackcoord;
glong trackcoordnr, tracklimit, trackcoordlimit;
gint extrawinmenu = FALSE, expedia = TRUE;
typedef struct
{
  gdouble lon;
  gdouble lat;
  gint x;
  gint y;
}
impstruct;
impstruct imports[3];
gdouble Ra[201];
gint haveproxy, proxyport;
gchar proxy[256], hostname[256], savetrackfn[256];

/*** Mod by Arms */
gint real_screen_x, real_screen_y, real_psize, real_smallmenu, int_padding;
gint SCREEN_X_2, SCREEN_Y_2;
gint havefriends = 0;
time_t waytxtstamp = 0, maptxtstamp = 0;
gint showallmaps = TRUE;
/* guint selwptimeout; */
gint dontsetwp = FALSE;
gint setwpactive = FALSE;
gint onemousebutton = FALSE;
gint shadow = TRUE, createroute = FALSE, routemode = FALSE;
gdouble routenearest = 9999999999.0;
GtkWidget *askwindow;
gint simpos_timeout = 0;
gint setdefaultpos = TRUE;
GtkWidget *create_route_button, *create_route2_button, *select_route_button;
gint markwaypoint = FALSE;
GtkWidget *addwaypointwindow, *setupfn[30];
gint oldbat = 125, oldloading = FALSE;
gint bat, loading, havebattery, havetemperature = FALSE;	/* Battery level and loading flag */
gint defaultserver = 0, disableapm = FALSE;
typedef struct
{
  gchar n[200];
}
namesstruct;
namesstruct *names;
gchar activewpfile[200], gpsdservername[200], setpositionname[80];
gint newsatslevel = TRUE, testgarmin = TRUE, needtosave = FALSE;
GtkWidget *serialbt, *mapdirbt, *addwaypoint1, *addwaypoint2, *frame_speed,
  *frame_sats, *slowcpubt;
gdouble altitude = 0.0, precision = (-1.0), gsaprecision = (-1.0);
gint oldsatfix = 0, oldsatsanz = -1, havealtitude = FALSE;
gint wpsize = 1000, satfix = 0, usedgps = FALSE;
gchar dgpsserver[80], dgpsport[10];
GtkWidget *posbt, *cover;
gint needreloadmapconfig = FALSE, simfollow = TRUE;
GdkPixbuf *batimage = NULL;
GdkPixbuf *temimage = NULL;
GdkPixbuf *satsimage = NULL;
gint numsats = 0, satsavail = 0;
gint numgrids = 4, scroll = TRUE, deleteline = 0;
gint satposmode = FALSE;
gint saytarget = FALSE, printoutsats = FALSE;
extern gchar *displaytext;
extern gint do_display_dsc, textcount;
gint minsecmode = FALSE, nightmode = FALSE, isnight = FALSE, disableisnight;
gint nighttimer;
GtkWidget *setupentry[50], *setupentrylabel[50];
void (*setupfunction[50]) ();
gchar utctime[20], loctime[20];
gint mod_setupcounter = 4;
gint cpuload = 40, redrawtimeout;
gint haveRMCsentence = FALSE;
gdouble NMEAsecs = 0.0;
gint NMEAoldsecs = 0;
gint borderlimit;
gint zoomscale = TRUE;
gint pdamode = FALSE;
gint exposecounter = 0, exposed = FALSE;
gint useplugins = FALSE;
gdouble tripodometer, tripavspeed, triptime, tripmaxspeed, triptmp;
gint tripavspeedcount;
gdouble trip_long, trip_lat;
gint lastnotebook = 0;
GtkWidget *settingsnotebook;
gint useflite = FALSE, forcenextroutepoint = FALSE;
extern gint zone;
gint ignorechecksum = FALSE;
GdkColor trackcolorv;

/* Give more debug informations */
gint mydebug = FALSE;
#define MAXDBNAME 30
char dbhost[MAXDBNAME], dbuser[MAXDBNAME], dbpass[MAXDBNAME];
char dbtable[MAXDBNAME], dbname[MAXDBNAME];
char dbtypelist[100][40];
int dbtypelistcount;
char dbwherestring[5000];
double dbdistance;
gint usesql = FALSE, dbusedist = FALSE;
extern gint sqlselects[100], sqlplace, friendsplace, kismetsock, havekismet;
GdkPixbuf *kismetpixbuf, *openwlanpixbuf, *closedwlanpixbuf, *iconpixbuf[50];
gint earthmate = FALSE;
/* gchar *p_1, *p_2, *p_3;
 * gsize *size_1;
 * gchar s_1[100], s_2[100];
 */
extern gint wptotal, wpselected;
gchar wplabelfont[100], bigfont[100];
gint showroute = TRUE;
extern gint zone;
/* gchar gradsym[]="\302\260";  */
static gchar gradsym[] = "\xc2\xb0";
gdouble normalnull = 0;
gint etch = 1;
gint drawgrid = TRUE;
gint drawmarkercounter = 0, loadpercent = 10, globruntime = 30;
extern int pleasepollme;
/* socket for friends  */
extern int sockfd;
extern long int maxfriendssecs;
gint forcehavepos = FALSE, needreminder = TRUE;
char actualhostname[200];
gdouble alarm_lat = 53.583033, alarm_long = 9.969533, alarm_dist = 9999999.0;
extern gchar cputempstring[20], batstring[20];
GtkTooltips *temptooltips = NULL;
GtkWidget *tempeventbox = NULL, *batteventbox = NULL, *sateventbox =
  NULL, *compasseventbox = NULL;
GtkWidget *wplabel1, *wplabel2, *wplabel3, *wplabel4, *wplabel5;
GtkWidget *wp1eventbox, *wp2eventbox, *wp3eventbox, *wp4eventbox,
  *wp5eventbox, *satsvbox, *satshbox, *satslabel1eventbox,
  *satslabel2eventbox, *satslabel3eventbox;
GtkWidget *satslabel1, *satslabel2, *satslabel3;
GtkWidget *lf1, *lf2, *lf3, *lf4, *lf5, *lf6, *lf7, *lf8;
GtkWidget *dframe, *frame_bearing, *frame_target, *frame_altitude,
  *frame_wp, *frame_maptype, *frame_toogles, *lab1, *fbat, *ftem;
GtkWidget *menubar;
gchar bluecolor[40], trackcolor[40], friendscolor[40];
gchar messagename[40], messagesendtext[1024], messageack[100];
GtkItemFactory *item_factory;
gint statuslock = 0, gpson = FALSE;
int messagenumber = 0, actualfriends = 0, didrootcheck = 0, haveserial = 0;
int gotneverserial = TRUE, timerto = 0, serialspeed = 1;
int disableserial = 1, disableserialcl = 0;
GtkTextBuffer *getmessagebuffer;
GdkColormap *cmap;
extern int newdata;
extern unsigned char serialdata[4096];
extern pthread_mutex_t mutex;
int mapistopo = FALSE;
int havenasa = -1, nosplash = FALSE, sortcolumn = 4, sortflag = 0;
int havedefaultmap = TRUE, alreadydaymode = FALSE, alreadynightmode = FALSE;
int nmeaverbose = 0, storetz = FALSE;
int egnoson = 0, egnosoff = 0;
extern char actualstreetname[200];
int sound_direction = 1, sound_distance = 1, sound_speed = 1, sound_gps = 1;
int expedia_de = 0;

static GtkItemFactoryEntry main_menu[] = {
  {N_("/_Misc. Menu"), NULL, NULL, 0, "<Branch>"},
  {N_("/_Misc. Menu/Maps"), NULL, NULL, 0, "<Branch>"},
  {N_("/_Misc. Menu/Maps/_Import map"), NULL,
   (gpointer) import1_cb,
   1,
   NULL},
  {N_("/_Misc. Menu/Maps/_Map Manager"), NULL,
   (gpointer) about_cb, 0, NULL},
  {N_("/_Misc. Menu/_Waypoint Manager"), NULL,
   (gpointer) sel_target_cb, 0, NULL},
  {N_("/_Misc. Menu/_Load track file"), NULL,
   (gpointer) loadtrack_cb, 0,
   "<StockItem>", GTK_STOCK_OPEN},

  {N_("/_Misc. Menu/Messages"), NULL, NULL, 0, "<Branch>"},
  {N_("/_Misc. Menu/Messages/Send message to mobile target"),
   NULL,
   (gpointer) sel_message_cb,
   0,
   NULL},
  {N_("/_Misc. Menu/Help"), NULL, NULL, 0, "<LastBranch>"},
  {N_("/_Misc. Menu/Help/About"), NULL, (gpointer) about_cb, 0,
   "<StockItem>", GTK_STOCK_DIALOG_INFO},
  {N_("/_Misc. Menu/Help/Topics"), NULL, (gpointer) help_cb, 0,
   "<StockItem>",
   GTK_STOCK_HELP}
};





/*  stolen from bluefish, translates the menu */
gchar *
menu_translate (const gchar * path, gpointer data)
{
  static gchar *menupath = NULL;

  gchar *retval;
  gchar *factory;

  factory = (gchar *) data;

  if (menupath)
    g_free (menupath);

  menupath = g_strdup (path);

  if ((strstr (path, "/tearoff1") != NULL) || (strstr (path, "/---") != NULL)
      || (strstr (path, "/MRU") != NULL))
    return menupath;

  retval = _(menupath);

  return retval;
}


#include "f_quit_program.c"
#include "f_error_popup.c"
#include "f_startgpsd.c"
#include "f_garblemain.c"

void
convertRMC (char *f)
{
  gchar field[50][100], b[100];
  gint start = 0;
  gint longdegree, latdegree;
  gchar langri, breitri;
  size_t i, j = 0;
  memset (b, 0, 100);

/*  if simulation mode we display status and return */
  if (simmode && maploaded && !posmode)
    {
      display_status (_("Simulation mode"));
      return;
    }

/*  get fields delimited with ',' */
  for (i = 0; i < strlen (f); i++)
    {
      if (f[i] == ',')
	{
	  g_strlcpy (field[j], (f + start), 100);
	  field[j][i - start] = 0;
	  start = i + 1;
	  j++;
	}
    }
  if ((j != 11) && (j != 12))
    {
      g_print ("\nGPRMC: wrong number of fields (%d)", (int) j);
      return;
    }
  if (!haveRMCsentence)
    {
      g_print (_("got RMC data, using it\n"));
      haveRMCsentence = TRUE;
    }

  if (debug)
    {
      g_print ("\nRMC Fields:\n");
      for (i = 0; i < j; i++)
	g_print ("\n%d:%s", (int) i, field[i]);
    }
  g_snprintf (b, sizeof (b), "%c%c:%c%c.%c%c ", field[1][0], field[1][1],
	      field[1][2], field[1][3], field[1][4], field[1][5]);
  g_strlcpy (utctime, b, sizeof (utctime));
  if (debug)
    g_print ("\nutctime: %s", utctime);
  if ((field[2][0] != 'A') || forcehavepos)
    {
      havepos = FALSE;
      haveposcount = 0;
      return;
    }
  else
    {
      havepos = TRUE;
      haveposcount++;
      if (haveposcount == 3)
	rebuildtracklist ();
    }
/*  Latitude North / South */
  b[0] = field[3][0];
  b[1] = field[3][1];
  b[2] = 0;
  latdegree = atoi (b);


  b[0] = field[3][2];
  b[1] = field[3][3];
  b[2] = localedecimal;
  b[3] = field[3][5];
  b[4] = field[3][6];
  b[5] = field[3][7];
  b[6] = 0;
  if (!posmode)
    {
      gdouble cl;
      cl = latdegree + atof (b) / 60.0;
      if (field[4][0] == 'S')
	cl = cl * -1;
      if ((cl >= -90.0) && (cl <= 90.0))
	current_lat = cl;
      breitri = field[4][0];
      g_snprintf (b, sizeof (b), " %8.5f%s%c", current_lat, gradsym, breitri);
    }

/*  Longitude East / West */
  b[0] = field[5][0];
  b[1] = field[5][1];
  b[2] = field[5][2];
  b[3] = 0;
  longdegree = atoi (b);

  b[0] = field[5][3];
  b[1] = field[5][4];
  b[2] = localedecimal;
  b[3] = field[5][6];
  b[4] = field[5][7];
  b[5] = field[5][8];
  b[6] = 0;
  if (!posmode)
    {
      gdouble cl;
      cl = longdegree + atof (b) / 60.0;
      if (field[6][0] == 'W')
	cl = cl * -1;
      if ((cl >= -180.0) && (cl <= 180.0))
	current_long = cl;

      langri = field[6][0];
      g_snprintf (b, sizeof (b), " %8.5f%s%c", current_long, gradsym, langri);
    }

/*  speed */
  b[0] = field[7][0];
  b[1] = field[7][1];
  b[2] = field[7][2];
  b[3] = localedecimal;
  b[4] = field[7][4];
  b[5] = 0;
  groundspeed = atof (b) * 1.852 * milesconv;
  g_snprintf (b, sizeof (b), " %s: %s", _("Map"), mapfilename);
/*    g_print("Field %s\n",field[8]); */
  b[0] = field[8][0];
  b[1] = field[8][1];
  b[2] = field[8][2];
  b[3] = localedecimal;
  b[4] = field[8][4];
  b[5] = 0;
/*  direction is the course we are driving */
  direction = atof (b);
  direction = direction * M_PI / 180;

  {
    int h, m, s;
    h = m = s = 0;
    if (strcmp (utctime, "n/a") != 0)
      {
	sscanf (utctime, "%d:%d.%d", &h, &m, &s);
	h += zone;
	if (h > 23)
	  h -= 24;
	if (h < 0)
	  h += 24;
	g_snprintf (loctime, sizeof (loctime), "%d:%02d.%02d", h, m, s);
      }
    else
      g_strlcpy (loctime, "n/a", sizeof (loctime));
  }

}

/*  show satellites signal level */
gint
convertGSV (char *f)
{
  gchar field[50][100], b[500];
  gint i, l, i2, j = 0, start = 0, n, db, anz, az, el;

  memset (b, 0, 100);
  l = strlen (f);
  for (i = 0; i < l; i++)
    {
      if ((f[i] == ',') || (f[i] == '*'))
	{
	  g_strlcpy (field[j], (f + start), 100);
	  field[j][i - start] = 0;
	  start = i + 1;
	  j++;
	}
    }
  if (debug)
    {
      g_print ("\nGSV Fields:\n");
      for (i = 0; i < j; i++)
	g_print ("%d:%s$", i, field[i]);
    }
  if (j > 40)
    {
      g_print ("\nGPGSV: wrong number of fields (%d)", j);
      return FALSE;
    }

  if (field[2][0] == '1')
    satbit = satbit | 1;
  if (field[2][0] == '2')
    satbit = satbit | 2;
  if (field[2][0] == '3')
    satbit = satbit | 4;

  anz = atoi (field[3]);
  b[0] = field[1][0];
  b[1] = 0;
  i2 = atof (b);
  if (mydebug)
    g_print ("\nbits should be:%d  is: %d", i2, satbit);
  g_snprintf (b, sizeof (b), "\nSatellites: %d ", anz);
  if (anz != oldsatsanz)
    newsatslevel = TRUE;
  oldsatsanz = anz;

  for (i = 4; i < j; i += 4)
    {
      n = atoi (field[i]);
      if (n > MAXSATS)
	{
	  fprintf (stderr, "illegal satellite number: %d, ignoring\n", n);
	  continue;
	}
      db = atoi (field[i + 3]);
      el = atoi (field[i + 1]);
      az = atoi (field[i + 2]);
      if (debug)
	fprintf (stderr,
		 "satnumber: %2d elev: %3d azimut: %3d signal %3ddb\n", n, el,
		 az, db);

      satlist[n][0] = n;
      satlist[n][1] = db;
      satlist[n][2] = el;
      satlist[n][3] = az;
    }

  if (((pow (2, i2)) - 1) == satbit)
    {
      satsavail = 0;
      for (i = 0; i < MAXSATS; i++)
	if (satlist[i][0] != 0)
	  {
	    g_snprintf (b, sizeof (b), "% 2d: % 2ddb   ", satlist[i][0],
			satlist[i][1]);
	    satsavail++;
	  }
      satbit = 0;

      memcpy (satlistdisp, satlist, sizeof (satlist));
      memset (satlist, 0, sizeof (satlist));
      newsatslevel = TRUE;
      return TRUE;
    }
  return FALSE;
}


/*  show altitude and satfix */
void
convertGGA (char *f)
{
  gchar field[50][100], b[500];
  gint i, l, j = 0, start = 0;
  gint longdegree, latdegree;
  gchar langri, breitri;

  memset (b, 0, 100);
  l = strlen (f);
  for (i = 0; i < l; i++)
    {
      if ((f[i] == ',') || (f[i] == '*'))
	{
	  g_strlcpy (field[j], (f + start), 100);
	  field[j][i - start] = 0;
	  start = i + 1;
	  j++;
	}
    }
  if (debug)
    {
      g_print ("\nGGA Fields:\n");
      for (i = 0; i < j; i++)
	g_print ("%d:%s$", i, field[i]);
    }

  if (j != 15)
    {
      g_print ("\nGPGGA: wrong number of fields (%d)", j);
      return;
    }

/*   the receiver sends no GPRMC, so we get the data from here */
  if (!haveRMCsentence)
    {
      gint mysecs;

/*       if (debug) */
      g_print (_("got no RMC data, using GGA data\n"));
      g_snprintf (b, sizeof (b), "%c%c", field[1][4], field[1][5]);
      sscanf (b, "%d", &mysecs);
      if (mysecs != NMEAoldsecs)
	{
	  NMEAsecs = mysecs - NMEAoldsecs;
	  if (NMEAsecs < 0)
	    NMEAsecs += 60;
	  NMEAoldsecs = mysecs;
	}
/*       g_print ("\nnmeasecs: %.2f mysecs: %d, nmeaoldsecs: %d", NMEAsecs, */
/* 	       mysecs, NMEAoldsecs); */
      g_snprintf (b, sizeof (b), "%c%c:%c%c.%c%c ", field[1][0], field[1][1],
		  field[1][2], field[1][3], field[1][4], field[1][5]);
      g_strlcpy (utctime, b, sizeof (utctime));

      if (field[6][0] == '0')
	{
	  havepos = FALSE;
	  haveposcount = 0;
	  return;
	}
      else
	{
	  havepos = TRUE;
	  haveposcount++;
	  if (haveposcount == 3)
	    rebuildtracklist ();
	}

/*  Latitude North / South */
      b[0] = field[2][0];
      b[1] = field[2][1];
      b[2] = 0;
      latdegree = atoi (b);


      b[0] = field[2][2];
      b[1] = field[2][3];
      b[2] = localedecimal;
      b[3] = field[2][5];
      b[4] = field[2][6];
      b[5] = field[2][7];
      b[6] = 0;
      if ( debug ) 
	fprintf(stderr,"\nposmode: %d\n",posmode);
      if (!posmode)
	{
	  gdouble cl;
	  cl = latdegree + atof (b) / 60.0;
	  if (field[3][0] == 'S')
	    cl = cl * -1;
	  if ((cl >= -90.0) && (cl <= 90.0))
	    current_lat = cl;

	  breitri = field[3][0];
/* 	  fprintf (stderr, "%8.5f%s%c cl:%f\n", current_lat, gradsym, breitri,cl); */
	}

/*  Longitude East / West */
      b[0] = field[4][0];
      b[1] = field[4][1];
      b[2] = field[4][2];
      b[3] = 0;
      longdegree = atoi (b);

      b[0] = field[4][3];
      b[1] = field[4][4];
      b[2] = localedecimal;
      b[3] = field[4][6];
      b[4] = field[4][7];
      b[5] = field[4][8];
      b[6] = 0;

      if (!posmode)
	{
	  gdouble cl;
	  cl = longdegree + atof (b) / 60.0;
	  if (field[5][0] == 'W')
	    cl = cl * -1;
	  if ((cl >= -180.0) && (cl <= 180.0))
	    current_long = cl;

	  langri = field[5][0];
/* 	  fprintf (stderr, "%8.5f%s%c cl:%f\n", current_long, gradsym, langri,cl); */
	}

      if (debug)
	g_print ("\nGGA pos: %f %f\n", current_lat, current_long);
    }

  satfix = g_strtod (field[6], 0);
  numsats = g_strtod (field[7], 0);
  if (havepos)
    {
      havealtitude = TRUE;
      altitude = g_strtod (field[9], 0);
      if (debug)
	{
	  g_print ("\nAltitude: %.1f, Fix: %d", altitude, satfix);
	}
    }
  else
    {
      groundspeed = 0;
      numsats = 0;
    }
  {
    int h, m, s;
    h = m = s = 0;
    if (strcmp (utctime, "n/a") != 0)
      {
	sscanf (utctime, "%d:%d.%d", &h, &m, &s);
	h += zone;
	if (h > 23)
	  h -= 24;
	if (h < 0)
	  h += 24;
	g_snprintf (loctime, sizeof (loctime), "%d:%02d.%02d", h, m, s);
      }
    else
      g_strlcpy (loctime, "n/a", sizeof (loctime));
  }
}


/*  show estimated position error $PGRME (Garmin only)  */
void
convertRME (char *f)
{
  gchar field[50][100], b[500];
  gint i, l, j = 0, start = 0;

  memset (b, 0, 100);
  l = strlen (f);
  for (i = 0; i < l; i++)
    {
      if ((f[i] == ',') || (f[i] == '*'))
	{
	  g_strlcpy (field[j], (f + start), 100);
	  field[j][i - start] = 0;
	  start = i + 1;
	  j++;
	}
    }
  if (debug)
    {
      g_print ("\nRME Fields:\n");
      for (i = 0; i < j; i++)
	g_print ("%d:%s$", i, field[i]);
    }
  if (havepos)
    {

      precision = g_strtod (field[1], 0);
      if (debug)
	{
	  g_print ("\nRME precision: %.1f", precision);
	}
    }
}

/*  show  HDOP in meters */
void
convertGSA (char *f)
{
  gchar field[50][100], b[500];
  gint i, l, j = 0, start = 0;

  memset (b, 0, 100);
  l = strlen (f);
  for (i = 0; i < l; i++)
    {
      if ((f[i] == ',') || (f[i] == '*'))
	{
	  g_strlcpy (field[j], (f + start), 100);
	  field[j][i - start] = 0;
	  start = i + 1;
	  j++;
	}
    }
  if (debug)
    {
      g_print ("\nGSA Fields:\n");
      for (i = 0; i < j; i++)
	g_print ("%d:%s$", i, field[i]);
    }
  if (havepos)
    {

      gsaprecision = g_strtod (field[15], 0);
      if (debug)
	{
	  g_print ("\nGSA PDOP: %.1f", gsaprecision);
	}
    }
}



void
display_status (char *message)
{
  gchar tok[200];

  if (downloadactive == TRUE)
    return;
  if (importactive == TRUE)
    return;
  if (statuslock)
    return;
  g_snprintf (tok, sizeof (tok), "%s", message);
  gtk_statusbar_pop (GTK_STATUSBAR (status), statusid);
  gtk_statusbar_push (GTK_STATUSBAR (status), statusid, tok);
}


gint
get_position_data_cb (GtkWidget * widget, guint * datum)
{
  gint e = 0, j = 0, i = 0, found = 0, foundGSV = 0, foundGGA = 0, foundGSA =
    0, foundRME = 0;
  gdouble secs = 0, tx, ty, lastdirection;
  typedef struct
  {
    gchar *a1;
    gchar *a2;
    gchar *a3;
  }
  argument;
  argument *argumente;
  char data[4096], tok[1000];
  static int serialtimeoutcount = 0;
  int tilimit;

  if (importactive)
    return TRUE;


  if ((timeoutcount > 3) && (debug))
    g_print ("*** %d. timeout getting data from GPS-Receiver!\n",
	     timeoutcount);

  if (haveserial)
    tilimit = 5;
  else
    tilimit = 10;

  if (timeoutcount > tilimit)
    {
      gtk_statusbar_pop (GTK_STATUSBAR (status), statusid);
      gtk_statusbar_push (GTK_STATUSBAR (status), statusid,
			  _("Timeout getting data from GPS-Receiver!"));
      havepos = FALSE;
      haveposcount = 0;
      memset (satlist, 0, sizeof (satlist));
      memset (satlistdisp, 0, sizeof (satlist));
      if (numsats != 0)
	{
	  numsats = 0;
	  gtk_widget_draw (drawing_sats, NULL);	/* this  calls expose handler */
	}
    }

  if (haveserial)
    {
      if ((timeoutcount > 8) && gotneverserial)
	{
	  havepos = FALSE;
	  haveposcount = 0;
	  haveserial = FALSE;
	  simmode = TRUE;
	  haveNMEA = FALSE;
	  newsatslevel = TRUE;
	  if (simpos_timeout == 0)
	    simpos_timeout =
	      gtk_timeout_add (300, (GtkFunction) simulated_pos, 0);
	  gtk_timeout_remove (timerto);
	  timerto =
	    gtk_timeout_add (TIMER, (GtkFunction) get_position_data_cb, NULL);

	  gtk_widget_set_sensitive (startgpsbt, TRUE);
	  gtk_widget_draw (drawing_sats, NULL);	/* this  calls expose handler */
/* 	  expose_sats_cb (NULL, 0); */

	  return TRUE;
	}

/*       getserialdata(); */
      if (newdata == TRUE)
	{
	  pthread_mutex_lock (&mutex);
	  g_strlcpy (data, serialdata, sizeof (data));
/* 	  fprintf (stderr, "#########:%s\n", data); */
	  newdata = FALSE;
	  pthread_mutex_unlock (&mutex);
	  timeoutcount = 0;

	  if (posmode)
	    display_status (_("Press middle mouse button for navigation"));
	  else
	    {
	      g_snprintf (nmeamodeandport, sizeof (nmeamodeandport),
			  _("Direct serial connection to %s"), serialdev);
	    }
	  if ((strncmp ((data + 3), "RMC", 3) == 0) && (data[0] == '$'))
	    {
	      havepos = TRUE;
	      gotneverserial = FALSE;
/*  we have the $GPRMC string completed, now parse it */
	      if (checksum (data) == TRUE)
		convertRMC (data);
	    }

	  if ((strncmp ((data + 3), "GSV", 3) == 0) && (data[0] == '$'))
	    {
	      gotneverserial = FALSE;
	      if (checksum (data) == TRUE)
		convertGSV (data);
	    }

	  if ((strncmp ((data + 3), "GGA", 3) == 0) && (data[0] == '$'))
	    {
	      gotneverserial = FALSE;
	      if (checksum (data) == TRUE)
		convertGGA (data);
	    }

	  if ((strncmp ((data + 3), "RME", 3) == 0) && (data[0] == '$'))
	    {
	      gotneverserial = FALSE;
	      if (checksum (data) == TRUE)
		convertRME (data);
	    }

	  if ((strncmp ((data + 3), "GSA", 3) == 0) && (data[0] == '$'))
	    {
	      gotneverserial = FALSE;
	      if (checksum (data) == TRUE)
		convertGSA (data);
	    }


/*  display the position and map in the statusline */
	  if (havepos)
	    {
	      if (posmode)
		display_status (_
				("Press middle mouse button for navigation"));
	      else
		display_status (nmeamodeandport);
	    }
	  else
	    display_status (_("Not enough satellites in view!"));
	}
      else
	{
	  serialtimeoutcount++;
	  if (serialtimeoutcount > (1000.0 / TIMERSERIAL))
	    {
	      timeoutcount++;
	      serialtimeoutcount = 0;
	      if (debug)
		fprintf (stderr, "timeout: %d\n", timeoutcount);
	    }
	}

      return TRUE;
    }


  argumente = NULL;
  if ((haveGARMIN) || (!haveRMCsentence))
    {
      if (haveGARMIN)
	{
	  argumente = g_new (argument, 1);
	  argumente->a1 = "-p";
	  argumente->a2 = "-p";
	  argumente->a3 = 0;

	  e = garblemain (1, (char **) argumente);
	  if (e == -2)
	    {
	      havepos = FALSE;
	      display_status (_("Not enough satellites in view!"));
	      haveposcount = 0;

	      return TRUE;
	    }
	  else
	    {
	      havepos = TRUE;
	      haveposcount++;
	    }
	  if (!posmode)
	    {
	      current_long = glang;
	      current_lat = gbreit;
	    }

	}
      if (haveGARMIN)
	secs = g_timer_elapsed (timer, 0);
      else
	secs = NMEAsecs;

      if (secs >= 1.0)
	{
	  tx =
	    (2 * R * M_PI / 360) * cos (M_PI * current_lat / 180.0) *
	    (current_long - old_long);
	  ty = (2 * R * M_PI / 360) * (current_lat - old_lat);
#define MINMOVE 4.0
	  if (((fabs (tx)) > MINMOVE) || (((fabs (ty)) > MINMOVE)))
	    {
	      if (haveGARMIN)
		{
		  g_timer_stop (timer);
		  g_timer_start (timer);
		}

	      lastdirection = direction;
	      if (ty == 0)
		direction = 0.0;
	      else
		direction = atan (tx / ty);
	      if (!finite (direction))
		direction = lastdirection;
	      if (ty < 0)
		direction = M_PI + direction;
	      if (direction >= (2 * M_PI))
		direction -= 2 * M_PI;
	      if (direction < 0)
		direction += 2 * M_PI;

	      groundspeed = milesconv * sqrt (tx * tx + ty * ty) * 3.6 / secs;
	      old_lat = current_lat;
	      old_long = current_long;
	    }
	  else if (secs > 4.0)
	    groundspeed = 0.0;

	  if (groundspeed > 2000)
	    groundspeed = 0;
	  if (groundspeed < 3.6)
	    groundspeed = 0;

	  if (mydebug)
	    g_print ("\nTime: %f", secs);
	}
/*  display status line */
      if (posmode)
	display_status (_("Press middle mouse button for navigation"));
      else
	{
	  if (haveGARMIN)
	    display_status (_("GARMIN Mode"));
	}

      if (haveGARMIN)
	{
	  g_free (argumente);
/*  return if in GARMIN mode */

	  return TRUE;
	}
    }

/*    only if we have neither NMEA nor GARMIN, means no measured position */
  if (!haveNMEA)
    {
/*  display status line */
      if (!simmode)
	display_status (_("No GPS used"));
      else if (maploaded && !posmode)
	display_status (_("Simulation mode"));
      else if (posmode)
	display_status (_("Press middle mouse button for sim mode"));


      return TRUE;
    }

/*  this is the NMEA reading part. data comes from port 2222 served by gpsd */
  FD_ZERO (&readmask);
  FD_SET (sock, &readmask);
  timeout.tv_sec = 0;
  timeout.tv_usec = 100000;
  if (select (FD_SETSIZE, &readmask, NULL, NULL, &timeout) < 0)
    {
      perror ("select() call");
    }
  if (FD_ISSET (sock, &readmask))
    {
      if ((e = read (sock, buffer, 2000)) < 0)
	perror ("read from gpsd connection");
      buffer[e] = 0;
      if ((bigp + e) < MAXBIG)
	{
	  if (mydebug)
	    g_print ("\n!!bigp:%d, e: %d!!", bigp, e);
	  g_strlcat (big, buffer, MAXBIG);
	  if (mydebug)
	    g_print (", strlen big:%d", strlen (big));
	  bigp += e;
	  bigpGSA = bigpRME = bigpGSV = bigpGGA = bigp;

	  for (i = bigp; i > lastp; i--)
	    {
	      if (big[i] == '$')
		{
/*  collect string for position data $GPRMC */
		  if ((big[i + 3] == 'R') && (big[i + 4] == 'M')
		      && (big[i + 5] == 'C'))
		    {
		      found = 0;
		      if (debug)
			g_print ("\n found #RMC#");
		      for (j = i; j <= bigp; j++)
			if (big[j] == 13)
			  {
			    found = j;
			    break;
			  }
		      if (found != 0)
			{
			  strncpy (tok, (big + i), found - i);
			  tok[found - i] = 0;
			  lastp = found;
			  timeoutcount = 0;
/*  we have the $GPRMC string completed, now parse it */
			  if (checksum (tok) == TRUE)
			    convertRMC (tok);
/*  display the position and map in the statusline */
			  if (havepos)
			    {
			      if (posmode)
				display_status (_
						("Press middle mouse button for navigation"));
			      else
				display_status (nmeamodeandport);
			    }
			  else
			    display_status (_
					    ("Not enough satellites in view!"));
			}
		    }

		}
	    }

/*  collect string for satellite data $GPGSV */

	  for (i = bigpGSV; i > lastpGSV; i--)
	    {
	      if (big[i] == '$')
		{

		  if ((big[i + 3] == 'G') && (big[i + 4] == 'S')
		      && (big[i + 5] == 'V'))
		    {
		      foundGSV = 0;
		      if (debug)
			g_print ("\n found #GSV#, bigpGSV: %d", bigpGSV);
		      for (j = i; j <= bigpGSV; j++)
			if (big[j] == 13)
			  {
			    foundGSV = j;
			    break;
			  }
		      if (foundGSV != 0)
			{
			  gint lenstr;
			  lenstr = foundGSV - i;
			  if ((lenstr) > 200)
			    {
			      g_print
				("\nError in line %d, foundGSV=%d,i=%d, diff=%d\n",
				 __LINE__, foundGSV, i, lenstr);
			      lenstr = 200;
			    }
			  if (i > foundGSV)
			    {
			      g_print
				("\nError in line %d, foundGSV=%d,i=%d, diff=%d\n",
				 __LINE__, foundGSV, i, lenstr);
			      lenstr = 0;
			    }
			  if (lenstr < 0)
			    {
			      g_print
				("\nError in line %d, foundGSV=%d,i=%d, lenstr=%d\n",
				 __LINE__, foundGSV, i, lenstr);
			      lenstr = 0;
			    }
			  if (lenstr != 0)
			    {
			      strncpy (tok, (big + i), lenstr);
			      tok[lenstr] = 0;

			      if (checksum (tok) == TRUE)
				{
				  if ((convertGSV (tok)) == TRUE)
				    lastpGSV = foundGSV;
				}
			    }
			}
		    }
		}
	    }


/*  collect string for altitude from  $GPGGA if available*/

	  for (i = bigpGGA; i > lastpGGA; i--)
	    {
	      if (big[i] == '$')
		{

		  if ((big[i + 3] == 'G') && (big[i + 4] == 'G')
		      && (big[i + 5] == 'A'))
		    {
		      foundGGA = 0;
		      if (debug)
			g_print ("\n found #GGA#");
		      timeoutcount = 0;

		      for (j = i; j <= bigpGGA; j++)
			if (big[j] == 13)
			  {
			    foundGGA = j;
			    break;
			  }
		      if (foundGGA != 0)
			{
			  gint lenstr;
			  lenstr = foundGGA - i;
			  if ((lenstr) > 200)
			    {
			      g_print
				("\nError in line %d, foundGGA=%d,i=%d, diff=%d\n",
				 __LINE__, foundGGA, i, lenstr);
			      lenstr = 200;
			    }
			  if (i > foundGGA)
			    {
			      g_print
				("\nError in line %d, foundGGA=%d,i=%d, diff=%d\n",
				 __LINE__, foundGGA, i, lenstr);
			      lenstr = 0;
			    }
			  if (lenstr != 0)
			    {
			      strncpy (tok, (big + i), lenstr);
			      tok[lenstr] = 0;
			      lastpGGA = foundGGA;

			      if (checksum (tok) == TRUE)
				convertGGA (tok);
			      if (havepos)
				{
				  if (posmode)
				    display_status (_
						    ("Press middle mouse button for navigation"));
				  else
				    display_status (nmeamodeandport);
				}
			      else
				display_status (_
						("Not enough satellites in view!"));

			    }
			}
		    }
		}
	    }

/*  collect string for precision from  $PGRME if available*/

	  for (i = bigpRME; i > lastpRME; i--)
	    {
	      if (big[i] == '$')
		{

		  if ((big[i + 1] == 'P') && (big[i + 2] == 'G')
		      && (big[i + 3] == 'R') && (big[i + 4] == 'M')
		      && (big[i + 5] == 'E'))
		    {
		      foundRME = 0;
		      if (debug)
			g_print ("\n found #RME#");
		      for (j = i; j <= bigpRME; j++)
			if (big[j] == 13)
			  {
			    foundRME = j;
			    break;
			  }
		      if (foundRME != 0)
			{
			  gint lenstr;
			  lenstr = foundRME - i;
			  if ((lenstr) > 200)
			    {
			      g_print
				("\nError in line %d, foundRME=%d,i=%d, diff=%d\n",
				 __LINE__, foundRME, i, lenstr);
			      lenstr = 200;
			    }
			  if (i > foundRME)
			    {
			      g_print
				("\nError in line %d, foundRME=%d,i=%d, diff=%d\n",
				 __LINE__, foundRME, i, lenstr);
			      lenstr = 0;
			    }
			  if (lenstr != 0)
			    {
			      strncpy (tok, (big + i), lenstr);
			      tok[lenstr] = 0;
			      lastpRME = foundRME;

			      if (checksum (tok) == TRUE)
				convertRME (tok);
			    }
			}
		    }
		}
	    }

/*  collect string for precision from  $GPGSA if available*/

	  for (i = bigpGSA; i > lastpGSA; i--)
	    {
	      if (big[i] == '$')
		{

		  if ((big[i + 1] == 'G') && (big[i + 2] == 'P')
		      && (big[i + 3] == 'G') && (big[i + 4] == 'S')
		      && (big[i + 5] == 'A'))
		    {
		      foundGSA = 0;
		      if (debug)
			g_print ("\n found #GSA#");
		      for (j = i; j <= bigpGSA; j++)
			if (big[j] == 13)
			  {
			    foundGSA = j;
			    break;
			  }
		      if (foundGSA != 0)
			{
			  gint lenstr;
			  lenstr = foundGSA - i;
			  if ((lenstr) > 200)
			    {
			      g_print
				("\nError in line %d, foundGSA=%d,i=%d, diff=%d\n",
				 __LINE__, foundGSA, i, lenstr);
			      lenstr = 200;
			    }
			  if (i > foundGSA)
			    {
			      g_print
				("\nError in line %d, foundGSA=%d,i=%d, diff=%d\n",
				 __LINE__, foundGSA, i, lenstr);
			      lenstr = 0;
			    }
			  if (lenstr != 0)
			    {
			      strncpy (tok, (big + i), lenstr);
			      tok[lenstr] = 0;
			      lastpGSA = foundGSA;

			      if (checksum (tok) == TRUE)
				convertGSA (tok);
			    }
			}
		    }
		}
	    }



	  if (mydebug)
	    g_print ("\nsize:%d lastp: %d \n%s", e, lastp, buffer);

	}
      else
	{
	  lastp = lastpGGA = lastpGSV = lastpRME = lastpGSA = 0;
	  bigp = e;
	  g_strlcpy (big, buffer, MAXBIG);
	}

    }
  else
    {
      timeoutcount++;
    }



  return (TRUE);
}

int
checkalarm (void)
{
  gdouble d;
  FILE *f;
  int pid, e;

  d = calcdist (alarm_long, alarm_lat);
  if (d < alarm_dist)
    return 0;

  fprintf (stderr, _("Distance to HomeBase: %.1fkm, max. allowed: %.1fkm\n"),
	   d, alarm_dist);
  f = fopen ("/tmp/cammain.pid", "r");
  if (f <= 0)
    {
      fprintf (stderr, "no cammain running!\n");
      return -1;
    }
  fscanf (f, "%d", &pid);
  e = kill (pid, SIGUSR1);
  if (e != 0)
    fprintf (stderr, "sending signal failed!\n");
  return 0;
}

/*  test if we need to load another map */
void
testnewmap ()
{
  long long best = 1000000000LL;
  gdouble posx, posy;
  long long bestmap = 9999999999LL;
  gdouble pixelfactloc, bestscale = 1000000000.0, fact;
  gint i, j, skip, istopo, ncount = 0;
  gchar str[100], buf[200], mappath[500];
  gdouble dif;
  gchar nasaname[255];
  static int nasaisvalid = FALSE;

  if (importactive)
    return;

  if (posmode)
    {
      current_long = posmode_x;
      current_lat = posmode_y;
    }
  else
    {
      gdouble d;
/*  test for new route point */
      if (strcmp (targetname, "     "))
	{
	  if (routemode)
	    d =
	      calcdist ((routelist + routepointer)->lon,
			(routelist + routepointer)->lat);
	  else
	    d = calcdist (target_long, target_lat);

	  if (d < routenearest)
	    {
	      routenearest = d;
	    }
/* 	  g_print */
/* 	    ("\nroutepointer: %d d: %.1f routenearest: %.1f routereach: %0.3f", */
/* 	     routepointer, d, routenearest, ROUTEREACH); */
	  if ((d <= ROUTEREACH) || (d > (ROUTEREACHFACT * routenearest))
	      || forcenextroutepoint)
	    {
	      forcenextroutepoint = FALSE;
	      if ((routepointer != (routeitems - 1)) && (routemode))
		{
		  routenearest = 9999999999.0;
		  routepointer++;
/* let's say the waypoint description */
		  g_strlcpy (mappath, homedir, sizeof (mappath));
		  g_strlcat (mappath, activewpfile, sizeof (mappath));
		  saytargettext (mappath, targetname);

		  setroutetarget (NULL, -1);
		}
	      else
		{
/*  route endpoint reached */
		  if (saytarget)
		    {
		      g_snprintf (str, sizeof (str), "%s: %s", _("To"),
				  targetname);
		      gtk_frame_set_label (GTK_FRAME (destframe), str);
		      createroute = FALSE;
		      routemode = FALSE;
		      saytarget = FALSE;
		      routepointer = routeitems = 0;
		      switch (voicelang)
			{
			case english:
			  g_snprintf (buf, sizeof (buf),
				      "You reached the target %s",
				      targetname);
			  break;
			case spanish:
			  g_snprintf (buf, sizeof (buf),
				      "usted ha llegado a %s", targetname);
			  break;
			case german:
			  g_snprintf (buf, sizeof (buf),
				      "Sie haben das Ziel %s erreicht",
				      targetname);
			}
		      speech_out_speek (buf);
/* let's say the waypoint description */
		      g_strlcpy (mappath, homedir, sizeof (mappath));
		      g_strlcat (mappath, activewpfile, sizeof (mappath));
		      saytargettext (mappath, targetname);
		    }
		}
	    }
	}
    }


/* search for suitable maps */
  if (displaymap_top)
    if ((havenasa < 0) || (!nasaisvalid))
      {

/* delete nasamaps entries from maps list */
	for (i = 0; i < nrmaps; i++)
	  {
	    if ((strcmp ((maps + i)->filename, "top_NASA_IMAGE.ppm")) == 0)
	      {
		for (j = i; j < (nrmaps - 1); j++)
		  *(maps + j) = *(maps + j + 1);
		nrmaps--;
		continue;
	      }
	  }

	havenasa =
	  create_nasa_mapfile (current_lat, current_long, TRUE, nasaname);
	if (havenasa > 0)
	  {
	    skip = FALSE;
	    i = nrmaps;
	    nrmaps++;
	    maps = g_renew (mapsstruct, maps, (nrmaps + 2));
	    havenasa =
	      create_nasa_mapfile (current_lat, current_long, FALSE,
				   nasaname);
	    (maps + i)->lat = current_lat;
	    (maps + i)->lon = current_long;
	    (maps + i)->scale = havenasa;
	    g_strlcpy ((maps + i)->filename, nasaname, 200);
	    if ((strcmp (oldfilename, "top_NASA_IMAGE.ppm")) == 0)
	      g_strlcpy (oldfilename, "XXXOLDMAPXXX.ppm",
			 sizeof (oldfilename));
	  }
      }

  nasaisvalid = FALSE;

  for (i = 0; i < nrmaps; i++)
    {
      skip = TRUE;
      if (displaymap_map){
        if (!(strncmp ((maps + i)->filename, "map_", 4))) {
          skip = FALSE;
          istopo = FALSE;
        }
        if (strstr ((maps + i)->filename, "/map_")){
          skip = FALSE;
          istopo = FALSE;
        }
      }
      if (displaymap_top) {
        if (!(strncmp ((maps + i)->filename, "top_", 4))) {
          skip = FALSE;
          istopo = TRUE;
        }
        if (strstr ((maps + i)->filename, "/top_")){
          skip = FALSE;
          istopo = TRUE;
        }
      }

      if (skip)
        continue;


/*        calcxy (&posx, &posy, (maps + i)->lon, (maps + i)->lat,1); */
/*  Longitude */
      if (istopo == FALSE)
	posx =
	  (Ra[(int) (100 + (maps + i)->lat)] * M_PI / 180) * cos (M_PI *
								  (maps +
								   i)->lat /
								  180.0) *
	  (current_long - (maps + i)->lon);
      else
	posx =
	  (Ra[(int) (100 + 0)] * M_PI / 180) * (current_long -
						(maps + i)->lon);


/*  latitude */

      if (istopo == FALSE)
	{
	  posy =
	    (Ra[(int) (100 + (maps + i)->lat)] * M_PI / 180) * (current_lat -
								(maps +
								 i)->lat);
	  dif =
	    Ra[(int) (100 + (maps + i)->lat)] * (1 -
						 (cos
						  ((M_PI *
						    (current_long -
						     (maps +
						      i)->lon)) /
						   180.0)));
	  posy = posy + dif / 2.0;
	}
      else
	posy =
	  (Ra[(int) (100 + 0)] * M_PI / 180) * (current_lat -
						(maps + i)->lat);


      pixelfactloc = (maps + i)->scale / PIXELFACT;
      posx = posx / pixelfactloc;
      posy = posy / pixelfactloc;

      if (strcmp ("top_NASA_IMAGE.ppm", (maps + i)->filename) == 0)
	{
	  ncount++;
	}

      if ((posx > -(640 - borderlimit)) && (posx < (640 - borderlimit)))
	{
	  if (((posy - borderlimit) > -512) && ((posy + borderlimit) < 512))
	    {


	      if (displaymap_top)
		if (strcmp ("top_NASA_IMAGE.ppm", (maps + i)->filename) == 0)
		  {
/* nasa map is in range */
		    nasaisvalid = TRUE;
		  }

	      if (scaleprefered)
		{
		  if (scalewanted > (maps + i)->scale)
		    fact = (gdouble) scalewanted / (maps + i)->scale;
		  else
		    fact = (maps + i)->scale / (gdouble) scalewanted;

		  if (fact < bestscale)
		    {
		      bestscale = fact;
		      bestmap = i;
/* 			  bestcentereddist = centereddist; */
		    }
		}
	      else		/* autobestmap */
		{
		  if ((maps + i)->scale < best)
		    {
		      bestmap = i;
		      best = (maps + i)->scale;
		    }
		}
	    }			/*  End of if posy> ... */
	}

    }
  if (bestmap != 9999999999LL)
    {
      g_strlcpy (mapfilename, (maps + bestmap)->filename,
		 sizeof (mapfilename));
      if ((strcmp (oldfilename, mapfilename)) != 0)
	{
	  g_strlcpy (oldfilename, mapfilename, sizeof (oldfilename));
	  if (debug)
	    g_print ("\nNew map: %s\n", mapfilename);
	  if (debug)
	    printf ("\nNew map: %s\n", mapfilename);
	  pixelfact = (maps + bestmap)->scale / PIXELFACT;
	  zero_long = (maps + bestmap)->lon;
	  zero_lat = (maps + bestmap)->lat;
	  mapscale = (maps + bestmap)->scale;
	  xoff = yoff = 0;
	  if (nrmaps > 0)
	    loadmap (mapfilename);
	}
    }
  else
    {
      if (((strcmp (oldfilename, mapfilename)) != 0) && (havedefaultmap))
	{
	  g_strlcpy (oldfilename, mapfilename, sizeof (oldfilename));
	  g_strlcpy (mapfilename, "top_GPSWORLD.jpg", sizeof (mapfilename));
	  pixelfact = 88226037.0 / PIXELFACT;
	  zero_long = 0;
	  zero_lat = 0;
	  mapscale = 88226037;
	  xoff = yoff = 0;
	  loadmap (mapfilename);
	}
    }

}

/*  set the target in routemode */
void
setroutetarget (GtkWidget * widget, gint datum)
{
  gchar buf[1000], buf2[1000], str[200], *tn;

  if (datum != -1)
    routepointer = datum;

  routenearest = 9999999;
  g_strlcpy (targetname, (routelist + routepointer)->name,
	     sizeof (targetname));
  target_lat = (routelist + routepointer)->lat;
  target_long = (routelist + routepointer)->lon;
  g_snprintf (str, sizeof (str), "%s: %s[%d/%d]", _("To"), targetname,
	      routepointer + 1, routeitems);
  gtk_frame_set_label (GTK_FRAME (destframe), str);
  tn = g_strdelimit (targetname, "_", ' ');
  g_strlcpy (buf2, "", sizeof (buf2));
  if (tn[0] == '*')
    {
      g_strlcpy (buf2, "das mobile Ziel ", sizeof (buf2));
      g_strlcat (buf2, (tn + 1), sizeof (buf2));
    }
  else
    g_strlcat (buf2, tn, sizeof (buf2));

  switch (voicelang)
    {
    case english:
      g_snprintf (buf, sizeof (buf), "New target is %s", buf2);
      break;
    case spanish:
      g_snprintf (buf, sizeof (buf), "DestinaciÂŽón definida: %s", buf2);
      break;
    case german:
      g_snprintf (buf, sizeof (buf), "Neues Ziel ist %s ", buf2);
    }
  speech_out_speek (buf);
  speechcount = 0;
  g_strlcpy (oldangle, "XXX", sizeof (oldangle));
  saytarget = TRUE;
}


gint
posmodeoff_cb (GtkWidget * widget, guint * datum)
{
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (posbt), FALSE);
  return FALSE;
}

gint
lightoff (GtkWidget * widget, guint * datum)
{
  disableisnight = FALSE;
/* gtk_widget_modify_bg (mainwindow, GTK_STATE_NORMAL, &nightcolor); */
  return FALSE;
}

gint
lighton (void)
{
  disableisnight = TRUE;
/*   nighttimer = gtk_timeout_add (30000, (GtkFunction) lightoff, 0); */

/*   gtk_widget_restore_default_style(mainwindow); */
  return TRUE;
}

gint
tripreset ()
{

  tripodometer = tripavspeed = triptime = tripmaxspeed = triptmp = 0.0;
  tripavspeedcount = 0;
  trip_lat = current_lat;
  trip_long = current_long;
  triptime = time (NULL);

  return TRUE;
}

/* map_koord.txt is in mappath! */
gint
testconfig_cb (GtkWidget * widget, guint * datum)
{
#ifdef MAKETHISTEST
  gint i;
  for (i = 0; i < nrmaps; i++)
    {
      if ((strncmp ((maps + i)->filename, "map_", 4)))
	if ((strncmp ((maps + i)->filename, "top_", 4)))
	  {
	    GString *error;
	    error = g_string_new (NULL);
	    g_string_printf (error, "\n%s%d\n%s", _("Error in line "), i + 1,
			     _
			     ("I have found filenames in map_koord.txt which are\n"
			      "not map_* or top_* files. Please rename them and change the entries in\n"
			      "map_koord.txt.  Use map_* for street maps and top_* for topographical\n"
			      "maps.  Otherwise, the maps will not be displayed!"));
	    error_popup ((gpointer *) error->str);
	    g_string_free (error, TRUE);
	    messageshown = TRUE;
	    return FALSE;
	  }
    }
#endif

  friendsagent_cb (NULL, 0);
  tripreset ();
  gtk_timeout_add (TRIPMETERTIMEOUT * 1000, (GtkFunction) dotripmeter, NULL);

  return FALSE;
}

gint
watchwp_cb (GtkWidget * widget, guint * datum)
{
  gint angle, i, radarnear;
  gdouble d;
  gchar buf[400], lname[200], l2name[200];
  gdouble ldist = 9999.0, l2dist = 9999.0;
  gdouble tx, ty, lastbearing;

/*  calculate new earth radius */
  earthr = calcR (current_lat);

  if (importactive)
    return TRUE;

  foundradar = FALSE;
  radarnear = FALSE;


  for (i = 0; i < maxwp; i++)
    {
/*  test for radar */
      if (((wayp + i)->action) == 1)
	{
	  d = calcdist2 ((wayp + i)->lon, (wayp + i)->lat);
	  if (d < 0.6)
	    {
	      lastbearing = radarbearing;
	      tx = -current_long + (wayp + i)->lon;
	      ty = -current_lat + (wayp + i)->lat;
	      radarbearing = atan (tx / ty);
	      if (!finite (radarbearing))
		radarbearing = lastbearing;

	      if (ty < 0)
		radarbearing = M_PI + radarbearing;
	      radarbearing -= direction;
	      if (radarbearing >= (2 * M_PI))
		radarbearing -= 2 * M_PI;
	      if (radarbearing < 0)
		radarbearing += 2 * M_PI;
	      if (radarbearing < 0)
		radarbearing += 2 * M_PI;
	      angle = radarbearing * 180.0 / M_PI;

	      if ((angle < 40) || (angle > 320))
		{
		  foundradar = TRUE;
		  if (d < ldist)
		    {
		      ldist = d;
		      g_strlcpy (lname, (wayp + i)->name, sizeof (lname));
		    }
		  if (d < 0.2)
		    {
		      foundradar = TRUE;
		      radarnear = TRUE;
		      if (d < l2dist)
			{
			  l2dist = d;
			  g_strlcpy (l2name, (wayp + i)->name,
				     sizeof (l2name));
			}
		    }
		}

	    }

	}
    }
  if (!foundradar)
    {
      g_strlcpy (lastradar, "----", sizeof (lastradar));
      g_strlcpy (lastradar2, "----", sizeof (lastradar2));
    }
  else
    {
      if ((strcmp (lname, lastradar)) != 0)
	{
	  g_strlcpy (lastradar, lname, sizeof (lastradar));
	  switch (voicelang)
	    {
	    case english:
	      g_snprintf (buf, sizeof (buf),
			  "Danger, Danger, Radar in %d meters, Your speed is %d",
			  (int) (ldist * 1000.0), (int) groundspeed);
	      break;
	    case spanish:
	      g_snprintf (buf, sizeof (buf),
			  "Atención Atención, control de  velocidad en %d metros, Su velocidad es %d",
			  (int) (ldist * 1000.0), (int) groundspeed);
	      break;
	    case german:
	      g_snprintf (buf, sizeof (buf),
			  "Achtung, Achtung, Radar in %d metern, Ihre Geschwindigkeit ist %d",
			  (int) (ldist * 1000.0), (int) groundspeed);
	    }
	  if (displaytext != NULL)
	    free (displaytext);
	  displaytext = strdup (buf + 10);
	  displaytext = g_strdelimit (displaytext, "\n", ' ');
	  displaytext = g_strdelimit (displaytext, "\")", ' ');

	  do_display_dsc = TRUE;
	  textcount = 0;

	  speech_out_speek (buf);
	}

      if (radarnear)
	if ((strcmp (l2name, lastradar2)) != 0)
	  {
	    g_strlcpy (lastradar2, l2name, sizeof (lastradar2));
	    switch (voicelang)
	      {
	      case english:
		g_snprintf (buf, sizeof (buf),
			    "Information, Radar in %d meters",
			    (int) (ldist * 1000.0));
		break;
	      case spanish:
		g_snprintf (buf, sizeof (buf),
			    "InformaciÂŽón, control de  velocidad en %d metros",
			    (int) (ldist * 1000.0));
		break;
	      case german:
		g_snprintf (buf, sizeof (buf),
			    "Information, Radar in %d meter",
			    (int) (ldist * 1000.0));
	      }
	    speech_out_speek (buf);
	  }

    }

  return TRUE;
}

void
checkinput (gchar * text)
{
  if ((strstr (text, "'")) != NULL)
    mintodecimal (text);
}

void
decimaltomin (gchar * text, gint islat)
{
  gint grad, min, minus = FALSE;
  gdouble dec, rest, minf, sec;
  dec = g_strtod (text, NULL);
  if (dec < 0)
    {
      minus = TRUE;
      dec *= -1.0;
    }
  grad = dec;
  rest = dec - grad;

  minf = (60.0 * rest);
  min = minf;
  rest = minf - min;
  sec = 60.0 * rest;
#if PREFER_MinDec
  /* Hack alert: there should be a 3-way lat/lon format selection in the GUI */
  if (islat)
    g_snprintf (text, 100, "%d%s%.3f'%c", grad, gradsym, minf,
		(minus) ? 'S' : 'N');
  else
    g_snprintf (text, 100, "%d%s%.3f'%c", grad, gradsym, minf,
		(minus) ? 'W' : 'E');
#else
  if (islat)
    g_snprintf (text, 100, "%d%s%.2d'%05.2f''%c", grad, gradsym,
		min, sec, (minus) ? 'S' : 'N');
  else
    g_snprintf (text, 100, "%d%s%.2d'%05.2f''%c", grad, gradsym,
		min, sec, (minus) ? 'W' : 'E');
#endif

}

void
mintodecimal (gchar * text)
{
  gint grad, minus = FALSE;
  gdouble sec;
  gint min;
  gdouble dec;
  gchar s2;
  gint rc;

  /*
   * Handle either DMS or MinDec formats
   */
  rc = sscanf (text, "%d\xc2\xb0%d'%lf''%c", &grad, &min, &sec, &s2);
  if (rc == 4)
    {
      /*   g_strdelimit (s1, ",", '.'); */
      /*   sscanf(s1,"%f",&sec); */
      dec = grad + min / 60.0 + sec / 3600.0;
    }
  else
    {
      gdouble fmin;
      rc = sscanf (text, "%d\xc2\xb0%lf'%c", &grad, &fmin, &s2);
      if (rc != 3)
	{
	  /* TODO: handle bad format gracefully */
	}
      dec = grad + fmin / 60.0;
    }
  if (s2 == 'W')
    minus = TRUE;
  if (s2 == 'S')
    minus = TRUE;
  if (minus)
    dec *= -1.0;
  g_snprintf (text, 100, "%.6f", dec);
/*   g_print ("%s\n", text); */
}

/*  display upper status line */
void
display_status2 ()
{
  gchar s2[100], buf[200], mf[50];
  gint h, m;
  gdouble secs, v;
  if (downloadactive == TRUE)
    return;
  if (importactive == TRUE)
    return;
  secs = g_timer_elapsed (disttimer, 0);
  h = hours;
  m = minutes;
  if (secs >= 300.0)
    {
      g_timer_stop (disttimer);
      g_timer_start (disttimer);
      v = 3600.0 * (olddist - dist) / secs;
      h = dist / v;
      m = 60 * (dist / v - h);
      if (mydebug)
	g_print ("\nsecs: %.0fs,v:%.0f,h:%d,m:%d", secs, v, h, m);
      if ((m < 0) || (h > 99))
	{
	  h = 99;
	  m = 99;
	}
      olddist = dist;
      hours = h;
      minutes = m;
      if (!muteflag)
	if (hours < 99)
	  {
	    switch (voicelang)
	      {
	      case english:
		if (hours > 0)
		  g_snprintf (buf, sizeof (buf),
			      "Arrival in approximatly %d hours and %d minutes",
			      hours, minutes);
		else
		  g_snprintf (buf, sizeof (buf),
			      "Arrival in approximatly %d minutes", minutes);
		break;
	      case spanish:
		if (hours > 0)
		  g_snprintf (buf, sizeof (buf),
			      "Llegada en %d horas y %d minutos",
			      hours, minutes);
		else
		  g_snprintf (buf, sizeof (buf), "Llegada en %d minutos",
			      minutes);
		break;
	      case german:
		if (hours > 0)
		  {
		    if (hours == 1)
		      g_snprintf (buf, sizeof (buf),
				  "Ankunft in circa einer Stunde und %d minuten",
				  minutes);
		    else
		      g_snprintf (buf, sizeof (buf),
				  "Ankunft in circa %d Stunden und %d minuten",
				  hours, minutes);
		  }
		else
		  g_snprintf (buf, sizeof (buf),
			      "Ankunft in zirca %d minuten", minutes);
		speech_out_speek (buf);
	      }
	  }

    }

  if (minsecmode)
    {
      g_snprintf (s2, sizeof (s2), "%8.5f", current_lat);
      decimaltomin (s2, 1);
    }
  else
    {
      g_snprintf (s2, sizeof (s2), "%8.5f", fabs (current_lat));
      if (current_lat >= 0)
	g_strlcat (s2, "N", sizeof (s2));
      else
	g_strlcat (s2, "S", sizeof (s2));
    }
  gtk_label_set_text (GTK_LABEL (l1), s2);
  if (minsecmode)
    {
      g_snprintf (s2, sizeof (s2), "%8.5f", current_long);
      decimaltomin (s2, 0);
    }
  else
    {
      g_snprintf (s2, sizeof (s2), "%8.5f", fabs (current_long));
      if (current_long >= 0)
	g_strlcat (s2, "E", sizeof (s2));
      else
	g_strlcat (s2, "W", sizeof (s2));
    }

  gtk_label_set_text (GTK_LABEL (l2), s2);
  strncpy (mf, mapfilename, 18);
  mf[17] = 0;
  gtk_label_set_text (GTK_LABEL (l3), mf);
  g_snprintf (s2, sizeof (s2), "1:%ld", mapscale);
  gtk_label_set_text (GTK_LABEL (l4), s2);
  g_snprintf (s2, sizeof (s2), "%3.0f%s", direction * 180.0 / M_PI, gradsym);

  gtk_label_set_text (GTK_LABEL (l5), s2);
  g_snprintf (s2, sizeof (s2), "%3.0f%s", bearing * 180.0 / M_PI, gradsym);

  gtk_label_set_text (GTK_LABEL (l6), s2);
  g_snprintf (s2, sizeof (s2), "%2d:%02dh", h, m);
  gtk_label_set_text (GTK_LABEL (l7), s2);
  if (scaleprefered)
    g_snprintf (s2, sizeof (s2), "1:%d", scalewanted);
  else
    g_snprintf (s2, sizeof (s2), _("Auto"));
  gtk_label_set_text (GTK_LABEL (l8), s2);
  if (debug)
    {
      if (havepos)
	g_print ("\n***Position: %f %f***\n", current_lat, current_long);
      else
	g_print ("\n***no valid Position:\n");
    }
}


/* draw the marker on the map */
gint
drawmarker_cb (GtkWidget * widget, guint * datum)
{
  static struct timeval tv1, tv2;
  struct timezone tz;
  long runtime, runtime2;

  if (importactive)
    return TRUE;

  if ((!disableisnight) && (!downloadwindowactive))
    {
      if (((nightmode == 1) || ((nightmode == 2) && isnight))
	  && (!alreadynightmode))
	{
	  gtk_widget_modify_bg (mainwindow, GTK_STATE_NORMAL, &nightcolor);
	  alreadydaymode = FALSE;
	  alreadynightmode = TRUE;
	  if (debug)
	    fprintf (stderr, "setting to night view\n");
	}
    }
  if (((nightmode == 0) || disableisnight || downloadwindowactive)
      && (!alreadydaymode))
    {
      gtk_widget_modify_bg (mainwindow, GTK_STATE_NORMAL, &defaultcolor);
      if (debug)
	fprintf (stderr, "setting to daylight view\n");
      alreadydaymode = TRUE;
      alreadynightmode = FALSE;
    }


  gettimeofday (&tv1, &tz);
  runtime2 = tv1.tv_usec - tv2.tv_usec;
  if (tv1.tv_sec != tv2.tv_sec)
    runtime2 += 1000000l * (tv1.tv_sec - tv2.tv_sec);
/*  we test if we have to load a new map because we are outside 
the currently loaded map */
  testnewmap ();
/*   g_print("\ndrawmarker_cb %d",drawmarkercounter++); */
  exposed = FALSE;
/* The position calculation is made in expose_cb() */
  gtk_widget_draw (drawing_area, NULL);	/* this  calls expose handler */
  if (!exposed)
    expose_cb (NULL, 0);
  expose_compass (NULL, 0);
  gettimeofday (&tv2, &tz);
  runtime = tv2.tv_usec - tv1.tv_usec;
  if (tv2.tv_sec != tv1.tv_sec)
    runtime += 1000000l * (tv2.tv_sec - tv1.tv_sec);
  globruntime = runtime / 1000;
  loadpercent = (int) (100.0 * (float) runtime / (runtime + runtime2));
/*   if (debug) */
/*     g_print ("Rechenzeit: %dms, %d%%\n", (int) (runtime / 1000), loadpercent); */

  return FALSE;
}

/* Try to produce only given CPU load  */

gint
calldrawmarker_cb (GtkWidget * widget, guint * datum)
{
  gint period;

  if (cpuload < 1)
    cpuload = 1;
  if (cpuload > 95)
    cpuload = 95;
  if (!haveNMEA)
    expose_sats_cb (NULL, 0);
  if (pleasepollme)
    {
      pleasepollme++;
      if (pleasepollme > 10)
	{
	  pleasepollme = 1;
	  friends_sendmsg (friendsserverip, NULL);
	}
    }
  period = 10 * globruntime / (10 * cpuload);
  drawmarkercounter++;
/*   g_print ("period: %d, drawmarkercounter %d\n", period, drawmarkercounter);  */

  if (((drawmarkercounter > period) || (drawmarkercounter > 50))
      && (drawmarkercounter >= 3))
    {
      drawmarkercounter = 0;
      drawmarker_cb (NULL, NULL);
    }
  return TRUE;
}


/* Friends agent */
gint
friendsagent_cb (GtkWidget * widget, guint * datum)
{
  time_t tii;
  gchar buf[MAXMESG], buf2[40], la[20], lo[20], num[5];
  gint i;

/* typedef struct
 * {
 * 	  char id[30], name[40], lat[40], longi[40], timesec[40], speed[10],
 *     heading[10];
 * }
 * friendsstruct;
 */

/* Don't allow spaces in name */
  for (i = 0; (size_t) i < strlen (friendsname); i++)
    if (friendsname[i] == ' ')
      friendsname[i] = '_';

/*  send position to friendsserver */

  if (havefriends)
    {
      if (strlen (messagesendtext) > 0)
	{
/* send message to server */
	  if (messagenumber < 99)
	    messagenumber++;
	  else
	    messagenumber = 0;
	  needtosave = TRUE;
	  g_snprintf (num, sizeof (num), "%02d", messagenumber);
	  g_strlcpy (buf2, friendsidstring, sizeof (buf2));
	  buf2[0] = 'M';
	  buf2[1] = 'S';
	  buf2[2] = 'G';
	  buf2[3] = num[0];
	  buf2[4] = num[1];
	  g_snprintf (buf, sizeof (buf), "SND: %s %s %s", buf2, messagename,
		      messagesendtext);
	  if (debug)
	    fprintf (stderr, "\nsending to %s:\n%s\n", friendsserverip, buf);
	  if (sockfd != -1)
	    close (sockfd);
	  sockfd = -1;
	  friends_sendmsg (friendsserverip, buf);
	  g_snprintf (messageack, sizeof (messageack), "SND: %s", buf2);
	}
      else
	{
/* send position to server */
	  g_snprintf (la, sizeof (la), "%10.6f", current_lat);
	  g_snprintf (lo, sizeof (lo), "%10.6f", current_long);
	  g_strdelimit (la, ",", '.');
	  g_strdelimit (lo, ",", '.');
	  tii = time (NULL);
	  g_snprintf (buf, sizeof (buf), "POS: %s %s %s %s %ld %.0f %.0f",
		      friendsidstring, friendsname, la, lo, tii,
		      groundspeed / milesconv, 180.0 * direction / M_PI);
	  if (debug)
	    fprintf (stderr, "\nsending to %s:\n%s\n", friendsserverip, buf);
	  if (sockfd != -1)
	    close (sockfd);
	  sockfd = -1;
	  friends_sendmsg (friendsserverip, buf);
	}
    }

  return TRUE;
}



/* Master agent */
gint
masteragent_cb (GtkWidget * widget, guint * datum)
{
  gchar mappath[400];
  gint f;
  struct stat buf;
  gchar buffer[200];

  if (needtosave)
    writeconfig ();


/* Check for changed map_koord.txt and reload if changed */
  g_strlcpy (mappath, mapdir, sizeof (mappath));
  g_strlcat (mappath, "map_koord.txt", sizeof (mappath));

  f = stat (mappath, &buf);
  if (buf.st_mtime != maptxtstamp)
    {
      loadmapconfig ();
      g_print ("\n%s reloaded\n", "map_koord.txt");
      maptxtstamp = buf.st_mtime;

    }

  if (needreloadmapconfig)
    loadmapconfig ();

  if (needreminder)
    {
      reminder_cb (NULL, 0);
      needreminder = FALSE;
    }
  checkalarm ();

  testifnight ();


  if (!didrootcheck)
    if (getuid () == 0)
      {
	g_snprintf (buffer, sizeof (buffer),
		    "<span  weight=\"bold\">%s</span>\n%s", _("Warning!"),
		    _("You should not start GpsDrive as user root!!!"));
	error_popup (buffer);
	didrootcheck = TRUE;
      }

/* Check for changed way.txt and reload if changed */
  g_strlcpy (mappath, homedir, sizeof (mappath));

  if (!sqlflag)
    g_strlcat (mappath, activewpfile, sizeof (mappath));
  else
    g_strlcat (mappath, "way-SQLRESULT.txt", sizeof (mappath));

  f = stat (mappath, &buf);
  if (buf.st_mtime != waytxtstamp)
    {
      loadwaypoints ();
      iszoomed = FALSE;
      if (!sqlflag)
	g_print ("\n%s reloaded\n", activewpfile);
      else
	g_print ("\n%s reloaded\n", "way-SQLRESULT.txt");
    }
  if (tracknr > (tracklimit - 1000))
    {
      g_print ("tracklimit: %ld", tracklimit);
      track = g_renew (GdkSegment, track, tracklimit + 100000);
      trackshadow = g_renew (GdkSegment, trackshadow, tracklimit + 100000);
      tracklimit += 100000;
    }
  if (trackcoordnr > (trackcoordlimit - 1000))
    {
      trackcoord =
	g_renew (trackcoordstruct, trackcoord, trackcoordlimit + 100000);
      trackcoordlimit += 100000;
    }



  return TRUE;
}

/* draw track on image */
gint
storetrack_cb (GtkWidget * widget, guint * datum)
{
  gint i, so;
  gchar buf3[35];
  time_t t;
  struct tm *ts;

/*    if (posmode) */
/*      return TRUE; */
  if (posmode)
    i = 1;
  else
    i = 2;

/*    g_print ("\nHavepos: %d", havepos); */
  if ((!simmode && !havepos) || posmode /*  ||((!simmode &&haveposcount<3)) */ )	/* we have no valid position */
    {
      (trackcoord + trackcoordnr)->longi = 1001.0;
      (trackcoord + trackcoordnr)->lat = 1001.0;
      (trackcoord + trackcoordnr)->alt = 1001.0;
    }
  else
    {
      (trackcoord + trackcoordnr)->longi = current_long;
      (trackcoord + trackcoordnr)->lat = current_lat;
      (trackcoord + trackcoordnr)->alt = altitude;
    }


/*  The double storage seems to be silly, but I have to use  */
/*  gdk_draw_segments instead of gdk_draw_lines.   */
/*  gkd_draw_lines is dog slow because of a gdk-bug. */

  if (tracknr == 0)
    {
      if ((trackcoord + trackcoordnr)->longi < 1000.0)
	{
	  (track + tracknr)->x1 = posx;
	  (track + tracknr)->y1 = posy;
	  (trackshadow + tracknr)->x1 = posx + SHADOWOFFSET;
	  (trackshadow + tracknr)->y1 = posy + SHADOWOFFSET;
	  tracknr++;
	}
    }
  else
    {
      if ((trackcoord + trackcoordnr)->longi < 1000.0)
	{
	  if ((posx != (track + tracknr - 1)->x2)
	      || (posy != (track + tracknr - 1)->y2))
	    {
/*  		so=(int)(((trackcoord + trackcoordnr)->alt))>>5; */
	      so = SHADOWOFFSET;
	      (track + tracknr)->x1 = (track + tracknr - 1)->x2 = posx;
	      (track + tracknr)->y1 = (track + tracknr - 1)->y2 = posy;
	      (trackshadow + tracknr)->x1 = (trackshadow + tracknr - 1)->x2 =
		posx + so;
	      (trackshadow + tracknr)->y1 = (trackshadow + tracknr - 1)->y2 =
		posy + so;
	      tracknr += 1;
	    }
	}
      else
	tracknr = tracknr & ((glong) - 2);
    }
  time (&t);
  ts = localtime (&t);
  strncpy (buf3, asctime (ts), 32);
  buf3[strlen (buf3) - 1] = '\0';	/* get rid of \n */
  g_strlcpy ((trackcoord + trackcoordnr)->postime, buf3, 30);
  trackcoordnr++;

  return TRUE;
}

void
drawwlan (gint posxdest, gint posydest, gint wlan)
{
  /*  wlan=0: no wlan, 1:open wlan, 2:WEP crypted wlan */

  if (wlan == 0)
    return;

  if ((posxdest >= 0) && (posxdest < SCREEN_X))
    {
      if ((posydest >= 0) && (posydest < SCREEN_Y))
	{
	  if (wlan == 1)
	    gdk_draw_pixbuf (drawable, kontext, openwlanpixbuf, 0, 0,
			     posxdest - 14, posydest - 12, 24, 24,
			     GDK_RGB_DITHER_NONE, 0, 0);
	  else
	    gdk_draw_pixbuf (drawable, kontext, closedwlanpixbuf, 0, 0,
			     posxdest - 14, posydest - 12, 24, 24,
			     GDK_RGB_DITHER_NONE, 0, 0);
	}
    }
}

int
drawicon (gint posxdest, gint posydest, char *ic)
{
  int symbol = 0, aux = -1, i, x, y;
  gchar icon[80];


  g_strlcpy (icon, ic, sizeof (icon));
  if (!sqlflag)
    g_strup (icon);

  if ((strcmp (icon, "REST")) == 0)
    symbol = 1;
  else if ((strcmp (icon, "MCDONALDS")) == 0)
    symbol = 2;
  else if ((strcmp (icon, "HOTEL")) == 0)
    symbol = 3;
  else if ((strcmp (icon, "BURGERKING")) == 0)
    symbol = 4;
  else if ((strcmp (icon, "SHOP")) == 0)
    symbol = 5;
  else if ((strcmp (icon, "MONU")) == 0)
    symbol = 6;
  else if ((strcmp (icon, "NIGHTCLUB")) == 0)
    symbol = 7;
  else if ((strcmp (icon, "SPEEDTRAP")) == 0)
    symbol = 8;
  else if ((strcmp (icon, "AIRPORT")) == 0)
    symbol = 9;
  else if ((strcmp (icon, "GOLF")) == 0)
    symbol = 10;
  else if ((strcmp (icon, "GASSTATION")) == 0)
    symbol = 11;
  else if ((strcmp (icon, "CAFE")) == 0)
    symbol = 12;
  else if ((strcmp (icon, "GEOCACHE")) == 0)
    symbol = 13;

  for (i = 0; i < lastauxicon; i++)
    if ((strcmp (icon, (auxicons + i)->name)) == 0)
      {
	if ((posxdest >= 0) && (posxdest < SCREEN_X))
	  {

	    if ((posydest >= 0) && (posydest < SCREEN_Y))
	      {
		x = gdk_pixbuf_get_width ((auxicons + i)->icon);
		y = gdk_pixbuf_get_width ((auxicons + i)->icon);
		gdk_draw_pixbuf (drawable, kontext, (auxicons + i)->icon,
				 0, 0,
				 posxdest - x / 2,
				 posydest - y / 2, x, y, GDK_RGB_DITHER_NONE,
				 0, 0);
		aux = i;
	      }
	  }
	return 99999;
      }

  if (symbol == 0)
    return 0;

  if ((posxdest >= 0) && (posxdest < SCREEN_X))
    {

      if ((posydest >= 0) && (posydest < SCREEN_Y))
	{
	  gdk_draw_pixbuf (drawable, kontext, iconpixbuf[symbol - 1],
			   0, 0,
			   posxdest - 12,
			   posydest - 12, 24, 24, GDK_RGB_DITHER_NONE, 0, 0);
	}
    }
  return symbol;
}

void
drawfriends (void)
{
  gint i;
  gdouble posxdest, posydest, clong, clat, direction;
  gint width, height;
  gdouble w;
  GdkPoint poly[16];
  struct tm *t;
  time_t ti, tif;
#define PFSIZE 55

  actualfriends = 0;
/*   g_print ("Maxfriends: %d\n",maxfriends); */
  for (i = 0; i < maxfriends; i++)
    {

/* 	    return if too old  */
      ti = time (NULL);
      tif = atol ((friends + i)->timesec);
      if (!(tif > 1000000000))
	fprintf (stderr, "Format error! timesec: %s, Name: %s, i: %d\n",
		 (friends + i)->timesec, (friends + i)->name, i);
      if ((ti - maxfriendssecs) > tif)
	continue;
      actualfriends++;
      clong = g_strtod ((friends + i)->longi, NULL);
      clat = g_strtod ((friends + i)->lat, NULL);


      calcxy (&posxdest, &posydest, clong, clat, zoom);

      if ((posxdest >= 0) && (posxdest < SCREEN_X))
	{

	  if ((posydest >= 0) && (posydest < SCREEN_Y))
	    {

	      gdk_draw_pixbuf (drawable, kontext, friendspixbuf, 0, 0,
			       posxdest - 18,
			       posydest - 12, 39, 24,
			       GDK_RGB_DITHER_NONE, 0, 0);
	      gdk_gc_set_line_attributes (kontext, 4, 0, 0, 0);
/*  draw pointer to direction */
	      direction =
		strtod ((friends + i)->heading, NULL) * M_PI / 180.0;
	      w = direction + M_PI;
	      gdk_gc_set_line_attributes (kontext, 2, 0, 0, 0);
	      poly[0].x = posxdest + (PFSIZE) / 2.3 * (cos (w + M_PI_2));
	      poly[0].y = posydest + (PFSIZE) / 2.3 * (sin (w + M_PI_2));
	      poly[1].x = posxdest + (PFSIZE) / 9 * (cos (w + M_PI));
	      poly[1].y = posydest + (PFSIZE) / 9 * (sin (w + M_PI));
	      poly[2].x = posxdest + PFSIZE / 10 * (cos (w + M_PI_2));
	      poly[2].y = posydest + PFSIZE / 10 * (sin (w + M_PI_2));
	      poly[3].x = posxdest - (PFSIZE) / 9 * (cos (w + M_PI));
	      poly[3].y = posydest - (PFSIZE) / 9 * (sin (w + M_PI));
	      poly[4].x = poly[0].x;
	      poly[4].y = poly[0].y;
	      gdk_gc_set_foreground (kontext, &blue);
	      gdk_draw_polygon (drawable, kontext, 0, (GdkPoint *) poly, 5);
	      gdk_draw_arc (drawable, kontext, 0, posxdest + 2 - 7,
			    posydest + 2 - 7, 10, 10, 0, 360 * 64);

/*   draw + sign at destination   */
	      gdk_gc_set_foreground (kontext, &red);
	      gdk_draw_line (drawable, kontext, posxdest + 1,
			     posydest + 1 - 5, posxdest + 1,
			     posydest + 1 + 5);
	      gdk_draw_line (drawable, kontext, posxdest + 1 + 5,
			     posydest + 1, posxdest + 1 - 5, posydest + 1);
	      {
		/* prints in pango */
		PangoFontDescription *pfd;
		PangoLayout *wplabellayout;
		gchar txt[200], txt2[100], s1[10];
		time_t sec;
		char *as, day[20], dispname[40];
		int speed, ii;

		sec = atol ((friends + i)->timesec);
		sec += 3600 * zone;
		t = gmtime (&sec);

		as = asctime (t);
		sscanf (as, "%s", day);
		sscanf ((friends + i)->speed, "%d", &speed);

		/* replace _ with  spaces in name */
		g_strlcpy (dispname, (friends + i)->name, sizeof (dispname));
		for (ii = 0; (size_t) ii < strlen (dispname); ii++)
		  if (dispname[ii] == '_')
		    dispname[ii] = ' ';

		g_snprintf (txt, sizeof (txt), "%s,%d", dispname,
			    (int) (speed * milesconv));
		if (milesflag)
		  g_snprintf (s1, sizeof (s1), "%s", _("mi/h"));
		else if (nauticflag)
		  g_snprintf (s1, sizeof (s1), "%s", _("knots"));
		else
		  g_snprintf (s1, sizeof (s1), "%s", _("km/h"));
		g_strlcat (txt, s1, sizeof (txt));
		g_snprintf (txt2, sizeof (txt2), "\n%s, %2d:%02d", day,
			    t->tm_hour, t->tm_min);
		g_strlcat (txt, txt2, sizeof (txt));
		wplabellayout =
		  gtk_widget_create_pango_layout (drawing_area, txt);
//KCFX  
		if (pdamode)
		  pfd = pango_font_description_from_string ("Sans 8");
		else
		  pfd = pango_font_description_from_string ("Sans bold 11");
		pango_layout_set_font_description (wplabellayout, pfd);
		pango_layout_get_pixel_size (wplabellayout, &width, &height);
		gdk_gc_set_foreground (kontext, &textbacknew);
/* 		gdk_draw_rectangle (drawable, kontext, 1, posxdest + 18,
 * 				    posydest - height/2 , width + 2,
 * 				    height + 2);
 */

		gdk_draw_layout_with_colors (drawable, kontext, posxdest + 21,
					     posydest - height / 2 + 1,
					     wplabellayout, &black, NULL);
		gdk_draw_layout_with_colors (drawable, kontext, posxdest + 20,
					     posydest - height / 2,
					     wplabellayout, &orange, NULL);

		if (wplabellayout != NULL)
		  g_object_unref (G_OBJECT (wplabellayout));
		/* freeing PangoFontDescription, cause it has been copied by prev. call */
		pango_font_description_free (pfd);

	      }


	    }
	}
    }



}


gint
expose_sats_cb (GtkWidget * widget, guint * datum)
{
  gint k, i, yabs, h, j, l;
  static GdkGC *mykontext = NULL;
  gchar t[10], t1[20], buf[300], txt[10];
  static GdkPixbufAnimation *anim = NULL;
  static GdkPixbufAnimationIter *animiter = NULL;
  GTimeVal timeresult;
#define SATX 5
/*  draw satellite level (field strength) only in NMEA modus*/

  if (haveNMEA)
    {
      gdk_gc_set_foreground (kontext, &lcd);

      gdk_draw_rectangle (drawable_sats, kontext, 1, 3, 0, PSIZE + 2,
			  PSIZE + 5);
      gdk_gc_set_line_attributes (kontext, 1, 0, 0, 0);
      gdk_gc_set_foreground (kontext, &black);
      gdk_draw_rectangle (drawable_sats, kontext, 0, 3, 0, PSIZE + 2,
			  PSIZE + 5);

      gdk_gc_set_foreground (kontext, &lcd);

      if (!satposmode)
	{
	  if (havepos)
	    gdk_gc_set_foreground (kontext, &green);
	  else
	    gdk_gc_set_foreground (kontext, &red);
	  gdk_gc_set_line_attributes (kontext, 2, 0, 0, 0);
	  for (i = 3; i < (PSIZE + 5); i += 3)
	    gdk_draw_line (drawable_sats, kontext, 4, i, PSIZE + 4, i);


	  gdk_gc_set_foreground (kontext, &lcd2);
	  gdk_gc_set_line_attributes (kontext, 5, 0, 0, 0);
	  k = 0;
	  for (i = 0; i < 16; i++)
	    {
	      if (i > 5)
		yabs = 2 + PSIZE;
	      else
		yabs = 1 + PSIZE / 2;
	      h = PSIZE / 2 - 2;
	      gdk_draw_line (drawable_sats, kontext, 6 + 7 * k + SATX, yabs,
			     6 + 7 * k + SATX, yabs - h);
	      k++;
	      if (k > 5)
		k = 0;
	    }
	}
      if (satfix == 1)		/* normal Fix */
	gdk_gc_set_foreground (kontext, &black);
      else
	{
	  if (satfix == 0)	/* no Fix */
	    gdk_gc_set_foreground (kontext, &textback);
	  else			/* Differntial Fix */
	    gdk_gc_set_foreground (kontext, &blue);
	}
      j = k = 0;
#define DEG2RAD M_PI/180.0
      if (satposmode)
	{
	  gdk_gc_set_line_attributes (kontext, 1, 0, 0, 0);
	  gdk_gc_set_foreground (kontext, &lcd2);
	  gdk_draw_arc (drawable_sats, kontext, 0, 4,
			4, PSIZE, PSIZE, 105 * 64, 330 * 64);
	  gdk_draw_arc (drawable_sats, kontext, 0, 5 + PSIZE / 4,
			4 + PSIZE / 4, PSIZE / 2, PSIZE / 2, 0, 360 * 64);
	  gdk_gc_set_foreground (kontext, &darkgrey);
	  {
	    /* prints in pango */
	    PangoFontDescription *pfd;
	    PangoLayout *wplabellayout;
	    gint width;

	    wplabellayout =
	      gtk_widget_create_pango_layout (drawing_sats, "N");
//KCFX  
	    if (pdamode)
	      pfd = pango_font_description_from_string ("Sans 7");
	    else
	      pfd = pango_font_description_from_string ("Sans bold 10");
	    pango_layout_set_font_description (wplabellayout, pfd);

	    gdk_draw_layout_with_colors (drawable_sats, kontext,
					 0 + (PSIZE) / 2, -2, wplabellayout,
					 &grey, NULL);
	    pango_layout_get_pixel_size (wplabellayout, &width, NULL);
/* 	    printf ("\nw: %d", width);  */
	    if (wplabellayout != NULL)
	      g_object_unref (G_OBJECT (wplabellayout));
	    /* freeing PangoFontDescription, cause it has been copied by prev. call */
	    pango_font_description_free (pfd);

	  }

/* 	  gdk_draw_text (drawable_sats, verysmalltextfont, kontext,
 * 			 2 + (PSIZE) / 2, 9, "N", 1);
 */
	  gdk_gc_set_foreground (kontext, &lcd2);

	}

      for (i = 0; i < MAXSATS; i++)
	if (satlistdisp[i][0] != 0)
	  {
	    if ((satlistdisp[i][1] > 30) && (printoutsats))
	      g_print ("%d %d\n", satlistdisp[i][3], satlistdisp[i][2]);
	    if (satposmode)
	      {
		gint x, y;
		gdouble el, az;
		el = (90.0 - satlistdisp[i][2]);
		az = satlistdisp[i][3] * DEG2RAD;

		x = (PSIZE / 2) + sin (az) * (el / 90.0) * (PSIZE / 2);
		y = (PSIZE / 2) - cos (az) * (el / 90.0) * (PSIZE / 2);
		l = (satlistdisp[i][1] / 6);
		if (l > 7)
		  l = 7;
		switch (l & 7)
		  {
		  case 0:
		  case 1:
		    gdk_gc_set_foreground (kontext, &textback);
		    break;
		  case 2:
		  case 3:
		    gdk_gc_set_foreground (kontext, &red);
		    break;
		  case 4:
		  case 5:
		  case 6:
		    gdk_gc_set_foreground (kontext, &yellow);
		    break;
		  case 7:
		    gdk_gc_set_foreground (kontext, &green2);
		    break;
		  }
		gdk_draw_arc (drawable_sats, kontext, 1, 2 + x,
			      2 + y, 5, 5, 0, 360 * 64);

	      }
	    else
	      {
		if (j > 5)
		  yabs = PSIZE + 2;
		else
		  yabs = 1 + PSIZE / 2;
		h = satlistdisp[i][1] - 30;
		if (h < 0)
		  h = 1;
		if (h > 19)
		  h = 19;
		gdk_draw_line (drawable_sats, kontext, 6 + 7 * k + SATX, yabs,
			       6 + 7 * k + SATX,
			       yabs - (PSIZE / 2) * h / (PSIZE / 2 - 5));
		k++;
		if (k > 5)
		  k = 0;
		j++;
	      }
	  }
      newsatslevel = FALSE;
      g_snprintf (txt, sizeof (txt), "%d/%d", numsats, satsavail);
      gtk_entry_set_text (GTK_ENTRY (satslabel1), txt);
      if ((precision > 0.0) && (satfix != 0))
	{
	  if (milesflag || nauticflag)
	    g_snprintf (t1, sizeof (t1), "%.0fft", precision * 3.2808399);
	  else
	    g_snprintf (t1, sizeof (t1), "%.0fm", precision);
	  g_snprintf (t, sizeof (t), "%s", t1);
	}
      else
	g_snprintf (t, sizeof (t), "n/a");

      gtk_entry_set_text (GTK_ENTRY (satslabel2), t);
      if ((gsaprecision > 0.0) && (satfix != 0))
	{
	  g_snprintf (t1, sizeof (t1), "%.1f", gsaprecision);
	  g_snprintf (t, sizeof (t), "%s", t1);
	}
      else
	g_snprintf (t, sizeof (t), "n/a");
      gtk_entry_set_text (GTK_ENTRY (satslabel3), t);

      g_strlcpy (buf, "", sizeof (buf));
      if (debug)
	g_print ("\nSatfix: %d", satfix);
      if (satfix != oldsatfix)
	{
	  if (satfix == 2)
	    {
	      switch (voicelang)
		{
		case english:
		  g_snprintf (buf, sizeof (buf),
			      "Differential GPS signal found");
		  break;
		case spanish:
		  g_snprintf (buf, sizeof (buf), "Destinación definida: ");
		  break;
		case german:
		  g_snprintf (buf, sizeof (buf),
			      "Ein differenzielles GPS Signal wurde gefunden");
		}
	    }
	  else if ((satfix == 1) && (oldsatfix == 2))
	    {
	      switch (voicelang)
		{
		case english:
		  g_snprintf (buf, sizeof (buf),
			      "No differential GPS signal detected");
		  break;
		case spanish:
		  g_snprintf (buf, sizeof (buf),
			      "No differential GPS signal detected ");
		  break;
		case german:
		  g_snprintf (buf, sizeof (buf),
			      "Kein differenzielles GPS Signal vorhanden");
		}
	    }

	  if (satfix == 0)
	    {
	      switch (voicelang)
		{
		case english:
		  g_snprintf (buf, sizeof (buf), "No sufficent GPS signal");
		  break;
		case spanish:
		  g_snprintf (buf, sizeof (buf),
			      "No sufficent GPS signal detected ");
		  break;
		case german:
		  g_snprintf (buf, sizeof (buf),
			      "Kein ausreichendes GPS Signal vorhanden");
		}
	    }
	  if (satfix == 1)
	    {
	      switch (voicelang)
		{
		case english:
		  g_snprintf (buf, sizeof (buf), "GPS signal good");
		  break;
		case spanish:
		  g_snprintf (buf, sizeof (buf), "GPS signal bueno ");
		  break;
		case german:
		  g_snprintf (buf, sizeof (buf),
			      "Gutes GPS Signal vorhanden");
		}
	    }




	  oldsatfix = satfix;
	  if (!muteflag && sound_gps)
	    speech_out_speek (buf);
	}
    }
  else
    {
      if (mykontext == NULL)
	mykontext = gdk_gc_new (drawable_sats);
      if (satsimage == NULL)
	{
	  gchar mappath[400];

	  g_snprintf (mappath, sizeof (mappath), "%s/gpsdrive/%s", DATADIR,
		      "gpsdriveanim.gif");
/* 	satsimage = */
/* 	  gdk_pixbuf_new_from_xpm_data ((const char **) pinguin_xpm); */
	  anim = gdk_pixbuf_animation_new_from_file (mappath, NULL);
	  if (anim == NULL)
	    fprintf (stderr, _("\nWarning: unable to load gpsdriveanim.gif!\n"
			       "Please install the program as root with:\nmake install\n\n"));
	  g_get_current_time (&timeresult);
	  if (animiter != NULL)
	    g_object_unref (animiter);
	  animiter = gdk_pixbuf_animation_get_iter (anim, &timeresult);
	  satsimage = gdk_pixbuf_animation_iter_get_pixbuf (animiter);
	}
      if (gdk_pixbuf_animation_iter_advance (animiter, NULL))
	satsimage = gdk_pixbuf_animation_iter_get_pixbuf (animiter);
      gdk_gc_set_function (mykontext, GDK_AND);
      gdk_draw_pixbuf (drawable_sats, kontext, satsimage, 0, 0,
		       SATX, 0, 50, 50, GDK_RGB_DITHER_NONE, 0, 0);
      gdk_gc_set_function (mykontext, GDK_COPY);
    }
  return TRUE;
}

void
drawloadedmaps ()
{
  /* Robins hacking 
     Show (in yellow) any downloaded maps with in +/-20% of the currently requested map download
     also show bounds of map with a black border
     This is currently hooked in to the drawdownloadrectangle() function but may be better else where as
     a seperate function that can be turned on and off as requried.
     Due to RGB bit masks the map to be downloaded will now be green so that the new download area will be visible
     over the top of the previous downloaded maps.
   */
  int i;
  gdouble x, y, la, lo;
  gint scale, xo, yo;
  gchar sc[20];

  for (i = 0; i < nrmaps; i++)
    {
      g_strlcpy (sc, newmapsc, sizeof (sc));
      g_strdelimit (sc, ",", '.');
      scale = g_strtod (sc, NULL);

      if (maps[i].scale <= scale * 1.2 && maps[i].scale >= scale * 0.8)
	{
	  //printf("Selected map at long %lf lat %lf\n",maps[i].lat,maps[i].lon);
	  la = maps[i].lat;
	  lo = maps[i].lon;
	  //              scale=maps[i].scale;
	  calcxy (&x, &y, lo, la, zoom);
	  xo = 1280.0 * zoom * scale / mapscale;
	  yo = 1024.0 * zoom * scale / mapscale;
	  // yellow background
	  gdk_gc_set_foreground (kontext, &yellow);
	  gdk_gc_set_function (kontext, GDK_AND);
	  gdk_gc_set_line_attributes (kontext, 2, 0, 0, 0);
	  gdk_draw_rectangle (drawable, kontext, 1, x - xo / 2, y - yo / 2,
			      xo, yo);
	  // solid border
	  gdk_gc_set_foreground (kontext, &black);
	  gdk_gc_set_function (kontext, GDK_SOLID);
	  gdk_gc_set_line_attributes (kontext, 2, 0, 0, 0);
	  gdk_draw_rectangle (drawable, kontext, 0, x - xo / 2, y - yo / 2,
			      xo, yo);

	}
    }
}


void
drawdownloadrectangle (gint big)
{
/*   draw downloadrectangle */
  if (downloadwindowactive)
    {
      gdouble x, y, la, lo;
      gchar longi[100], lat[100], sc[20];
      gint scale, xo, yo;

      drawloadedmaps ();

      g_strlcpy (lat, newmaplat, sizeof (lat));
      g_strdelimit (lat, ",", '.');
      la = g_strtod (lat, NULL);

      g_strlcpy (longi, newmaplongi, sizeof (longi));
      g_strdelimit (longi, ",", '.');
      lo = g_strtod (longi, NULL);

      g_strlcpy (sc, newmapsc, sizeof (sc));
      g_strdelimit (sc, ",", '.');
      scale = g_strtod (sc, NULL);

      gdk_gc_set_foreground (kontext, &green2);
      gdk_gc_set_function (kontext, GDK_AND);
      gdk_gc_set_line_attributes (kontext, 2, 0, 0, 0);
      if (big)
	{
	  calcxy (&x, &y, lo, la, zoom);
	  xo = 1280.0 * zoom * scale / mapscale;
	  yo = 1024.0 * zoom * scale / mapscale;
	  gdk_draw_rectangle (drawable, kontext, 1, x - xo / 2, y - yo / 2,
			      xo, yo);
	}
      else
	{
	  calcxymini (&x, &y, lo, la, 1);
	  xo = 128.0 * scale / mapscale;
	  yo = 102.0 * scale / mapscale;
	  gdk_draw_rectangle (drawing_miniimage->window, kontext, 1,
			      x - xo / 2, y - yo / 2, xo, yo);
	}

      gdk_gc_set_function (kontext, GDK_COPY);
    }

}




/* draw the marker on the map */
gint
drawmarker (GtkWidget * widget, guint * datum)
{
  gdouble posxdest, posydest, posxmarker, posymarker;
  gchar s2[100], s3[200], txt[200], s2a[20];
  gdouble w;
  GdkPoint poly[16];
  gint k, k2, i, shownwp = 0;

  gblink = !gblink;
/*    g_print("\nsimmode: %d, nmea %d garmin %d",simmode,haveNMEA,haveGARMIN); */

  if (importactive)
    return TRUE;

  if (drawgrid)
    {
      // TODO: add lat/lon as Text to the lines so we know which is which
      gdouble lat, lon;

      // add more lines as the scale increases
      gdouble step;
      step=1;
      if (mapscale < 600000 ) 
	step=.5;

      gdk_gc_set_foreground (kontext, &darkgrey);
      gdk_gc_set_function (kontext, GDK_AND);
      gdk_gc_set_line_attributes (kontext, 1, 0, 0, 0);


      // TODO: calculate the start and stop for lat/lon according to the displayed section
      for ( lon=-180 ; lon<180 ; lon = lon + step ) {
	for ( lat=-90 ; lat < 90 ; lat = lat + step ) {
	  gdouble posxdest11, posydest11;
	  gdouble posxdest12, posydest12;
	  gdouble posxdest21, posydest21;
	  gdouble posxdest22, posydest22;
	  calcxy (&posxdest11, &posydest11, lon  , lat  , zoom);
	  calcxy (&posxdest12, &posydest12, lon  , lat+step, zoom);
	  calcxy (&posxdest21, &posydest21, lon+step, lat  , zoom);
	  calcxy (&posxdest22, &posydest22, lon+step, lat+step, zoom);

	  if ( ( (posxdest11 >= 0) && (posxdest11 < SCREEN_X) &&
		 (posydest11 >= 0) && (posydest11 < SCREEN_Y) 
		 )
	       &&
	       ( (posxdest22 >= 0) && (posxdest22 < SCREEN_X) &&
		 (posydest22 >= 0) && (posydest22 < SCREEN_Y) 
		 )
	      ) 
	    {
	      // TODO: add linethikness 2 for Mayor Lines
	      gdk_draw_line (drawable, kontext,  posxdest11,posydest11, posxdest21,posydest21);
	      gdk_draw_line (drawable, kontext,  posxdest11,posydest11, posxdest12,posydest12);
	    }
	}
      }
    }
      
  drawtracks ();


  if (wpflag)
    {
/*  draw waypoints */
      for (i = 0; i < maxwp; i++)
	{
	  calcxy (&posxdest, &posydest, 
		  (wayp + i)->lon, (wayp + i)->lat,
		  zoom);

	  if ((posxdest >= 0) && (posxdest < SCREEN_X)
	      && (shownwp < MAXSHOWNWP))
	    {
	      gdk_gc_set_line_attributes (kontext, 2, 0, 0, 0);

	      if ((posydest >= 0) && (posydest < SCREEN_Y))
		{
		  shownwp++;
		  g_strlcpy (txt, (wayp + i)->name, sizeof (txt));

		  if ((wayp + i)->wlan > 0)
		    drawwlan (posxdest, posydest, (wayp + i)->wlan);
		  else
		    if ((drawicon (posxdest, posydest, (wayp + i)->typ)) == 0)
		    {
		      if (shadow)
			{
/*  draw shadow of + sign */
			  gdk_gc_set_foreground (kontext, &darkgrey);
			  gdk_gc_set_function (kontext, GDK_AND);
			  gdk_draw_line (drawable, kontext,
					 posxdest + 1 + SHADOWOFFSET,
					 posydest + 1 - 5 + SHADOWOFFSET,
					 posxdest + 1 + SHADOWOFFSET,
					 posydest + 1 + 5 + SHADOWOFFSET);
			  gdk_draw_line (drawable, kontext,
					 posxdest + 1 + 5 + SHADOWOFFSET,
					 posydest + 1 + SHADOWOFFSET,
					 posxdest + 1 - 5 + SHADOWOFFSET,
					 posydest + 1 + SHADOWOFFSET);
			  gdk_gc_set_function (kontext, GDK_COPY);
			}

/*  draw + sign at destination */
		      gdk_gc_set_foreground (kontext, &red);
		      gdk_draw_line (drawable, kontext, posxdest + 1,
				     posydest + 1 - 5, posxdest + 1,
				     posydest + 1 + 5);
		      gdk_draw_line (drawable, kontext, posxdest + 1 + 5,
				     posydest + 1, posxdest + 1 - 5,
				     posydest + 1);
		      if ( (wayp + i)->proximity > 0.0 ) 
			{
			  gint proximity_pixels;
			  if ( mapscale ) 
			    proximity_pixels= 
			      ((wayp + i)->proximity)
			      * zoom
			      * PIXELFACT
			      / mapscale;
			  else 
			    proximity_pixels=2;

			  gdk_gc_set_foreground (kontext, &blue);

			  gdk_draw_arc(drawable, kontext, FALSE,
				       posxdest - proximity_pixels , 
				       posydest - proximity_pixels ,
				       proximity_pixels*2,proximity_pixels*2,
				       0, 64 * 360);
			}
		  
		    }
	      
/*  draw shadow of text */
		  {
		    /* prints in pango */
		    PangoFontDescription *pfd;
		    PangoLayout *wplabellayout;
		    gint width, height;
		    gchar *tn;

		    gdk_gc_set_foreground (kontext, &darkgrey);
		    gdk_gc_set_function (kontext, GDK_AND);
		    tn = g_strdelimit (txt, "_", ' ');

		    wplabellayout =
		      gtk_widget_create_pango_layout (drawing_area, tn);
		    pfd = pango_font_description_from_string (wplabelfont);
		    pango_layout_set_font_description (wplabellayout, pfd);
		    pango_layout_get_pixel_size (wplabellayout, &width,
						 &height);
/* 		  printf("\nj: %d",height);    */
		    k = width + 4;
		    k2 = height;
		    if (shadow)
		      {

			gdk_draw_layout_with_colors (drawable, kontext,
						     posxdest + 15 +
						     SHADOWOFFSET,
						     posydest - k2 / 2 +
						     SHADOWOFFSET,
						     wplabellayout, &darkgrey,
						     NULL);
		      }
		    if (wplabellayout != NULL)
		      g_object_unref (G_OBJECT (wplabellayout));
		    /* freeing PangoFontDescription, cause it has been copied by prev. call */
		    pango_font_description_free (pfd);

		  }
		  gdk_gc_set_function (kontext, GDK_COPY);


		  gdk_gc_set_function (kontext, GDK_AND);

		  gdk_gc_set_foreground (kontext, &textbacknew);
		  gdk_draw_rectangle (drawable, kontext, 1, posxdest + 13,
				      posydest - k2 / 2, k + 1, k2);
		  gdk_gc_set_function (kontext, GDK_COPY);
		  gdk_gc_set_foreground (kontext, &black);
		  gdk_gc_set_line_attributes (kontext, 1, 0, 0, 0);
		  gdk_draw_rectangle (drawable, kontext, 0, posxdest + 12,
				      posydest - k2 / 2 - 1, k + 2, k2);

/* 		  gdk_gc_set_foreground (kontext, &yellow);  */
		  {
		    /* prints in pango */
		    PangoFontDescription *pfd;
		    PangoLayout *wplabellayout;

		    wplabellayout =
		      gtk_widget_create_pango_layout (drawing_area, txt);
		    pfd = pango_font_description_from_string (wplabelfont);
		    pango_layout_set_font_description (wplabellayout, pfd);

		    gdk_draw_layout_with_colors (drawable, kontext,
						 posxdest + 15,
						 posydest - k2 / 2,
						 wplabellayout, &white, NULL);
		    if (wplabellayout != NULL)
		      g_object_unref (G_OBJECT (wplabellayout));
		    /* freeing PangoFontDescription, cause it has been copied by prev. call */
		    pango_font_description_free (pfd);

		  }

/* 		    gdk_draw_text (drawable, smalltextfont, kontext,
 * 				 posxdest + 13, posydest + 6, txt,
 * 				 strlen (txt));
 */
		}
	    }
	}
    }

  if (havefriends)
    drawfriends ();

  if (havekismet)
    readkismet ();

/*  draw scale */
  if (zoomscale)
    {
      gint pixels;
      gint m, l;
      gchar txt[100];
      pixels = 141 / milesconv;
      m = mapscale / (20 * zoom);
      if (m < 1000)
	{
	  if (!nauticflag)
	    g_snprintf (txt, sizeof (txt), "%d%s", m,
			(milesflag) ? "yrds" : "m");
	  else
	    g_snprintf (txt, sizeof (txt), "%.3f%s", m / 1000.0,
			(milesflag) ? "mi" : ((metricflag) ? "km" : "nmi"));

	  if (!metricflag)
	    pixels = pixels * milesconv * 0.9144;
	}
      else
	g_snprintf (txt, sizeof (txt), "%.1f%s", m / 1000.0,
		    (milesflag) ? "mi" : ((metricflag) ? "km" : "nmi"));
/*       l =
 * 	(SCREEN_X - 20) - pixels + (pixels -
 * 				    gdk_text_width (smalltextfont, txt,
 * 						    strlen (txt))) / 2;
 */
      l = (SCREEN_X - 40) - pixels + (pixels - strlen (txt) * 15);

/*       if (debug) */
/* 	g_print ("%d\n", m); */

      gdk_gc_set_function (kontext, GDK_OR);
      gdk_gc_set_foreground (kontext, &textback);
      gdk_draw_rectangle (drawable, kontext, 1, (SCREEN_X - 20) - pixels - 5,
			  SCREEN_Y - 35, pixels + 10, 30);
      gdk_gc_set_function (kontext, GDK_COPY);

      gdk_gc_set_foreground (kontext, &black);

/* 	  gdk_draw_text (drawable, smalltextfont, kontext, l, SCREEN_Y - 20, txt,
 * 		     strlen (txt));
 */
      {
	/* prints in pango */
	PangoFontDescription *pfd;
	PangoLayout *wplabellayout;

	wplabellayout = gtk_widget_create_pango_layout (drawing_area, txt);
//KCFX  
	if (pdamode)
	  pfd = pango_font_description_from_string ("Sans 8");
	else
	  pfd = pango_font_description_from_string ("Sans 11");
	pango_layout_set_font_description (wplabellayout, pfd);

	gdk_draw_layout_with_colors (drawable, kontext,
				     l, SCREEN_Y - 33,
				     wplabellayout, &black, NULL);
	if (wplabellayout != NULL)
	  g_object_unref (G_OBJECT (wplabellayout));
	/* freeing PangoFontDescription, cause it has been copied by prev. call */
	pango_font_description_free (pfd);

      }

      gdk_gc_set_line_attributes (kontext, 2, 0, 0, 0);
      gdk_draw_line (drawable, kontext, (SCREEN_X - 20) - pixels,
		     SCREEN_Y - 20 + 5, (SCREEN_X - 20), SCREEN_Y - 20 + 5);
      gdk_draw_line (drawable, kontext, (SCREEN_X - 20) - pixels,
		     SCREEN_Y - 20, (SCREEN_X - 20) - pixels,
		     SCREEN_Y - 20 + 10);
      gdk_draw_line (drawable, kontext, (SCREEN_X - 20),
		     SCREEN_Y - 20, (SCREEN_X - 20), SCREEN_Y - 20 + 10);

#ifdef USETELEATLAS
/* display the streetname */
      ta_displaystreetname (actualstreetname);
#endif


/*  draw zoom factor */
      g_snprintf (txt, sizeof (txt), "%dx", zoom);
/*       l = (SCREEN_X - 15) - gdk_text_width (textfont, txt, strlen (txt)) / 2;  */

      l = (SCREEN_X - 15) - 14 - strlen (txt) * 2;

      gdk_gc_set_function (kontext, GDK_OR);

      gdk_gc_set_foreground (kontext, &mygray);
      gdk_draw_rectangle (drawable, kontext, 1, (SCREEN_X - 30), 0, 30, 30);
      gdk_gc_set_function (kontext, GDK_COPY);

      gdk_gc_set_foreground (kontext, &blue);

/*       gdk_draw_text (drawable, textfont, kontext, l, 22, txt, strlen (txt));  */
      {
	/* prints in pango */
	PangoFontDescription *pfd;
	PangoLayout *wplabellayout;

	wplabellayout = gtk_widget_create_pango_layout (drawing_area, txt);
//KCFX  
	if (pdamode)
	  pfd = pango_font_description_from_string ("Sans 9");
	else
	  pfd = pango_font_description_from_string ("Sans 14");
	pango_layout_set_font_description (wplabellayout, pfd);

	gdk_draw_layout_with_colors (drawable, kontext,
				     l, 2, wplabellayout, &blue, NULL);
	if (wplabellayout != NULL)
	  g_object_unref (G_OBJECT (wplabellayout));
	/* freeing PangoFontDescription, cause it has been copied by prev. call */
	pango_font_description_free (pfd);

      }

      if (havekismet)
	gdk_draw_pixbuf (drawable, kontext, kismetpixbuf, 0, 0,
			 10, SCREEN_Y - 42,
			 36, 20, GDK_RGB_DITHER_NONE, 0, 0);

      if (savetrack)
	{
/* 	  k =
 * 	    gdk_text_width (smalltextfont, savetrackfn, strlen (savetrackfn));
 */
	  k = 100;
	  gdk_gc_set_foreground (kontext, &white);
	  gdk_draw_rectangle (drawable, kontext, 1, 10,
			      SCREEN_Y - 21, k + 3, 14);
	  gdk_gc_set_foreground (kontext, &red);
	  {
	    /* prints in pango */
	    PangoFontDescription *pfd;
	    PangoLayout *wplabellayout;

	    wplabellayout =
	      gtk_widget_create_pango_layout (drawing_area, savetrackfn);
//KCFX  
	    if (pdamode)
	      pfd = pango_font_description_from_string ("Sans 7");
	    else
	      pfd = pango_font_description_from_string ("Sans 10");
	    pango_layout_set_font_description (wplabellayout, pfd);

	    gdk_draw_layout_with_colors (drawable, kontext,
					 14, SCREEN_Y - 22,
					 wplabellayout, &red, NULL);
	    if (wplabellayout != NULL)
	      g_object_unref (G_OBJECT (wplabellayout));
	    /* freeing PangoFontDescription, cause it has been copied by prev. call */
	    pango_font_description_free (pfd);

	  }


/* 	  gdk_draw_text (drawable, smalltextfont, kontext,
 * 			 11, SCREEN_Y - 10, savetrackfn,
 * 			 strlen (savetrackfn));
 */

/*      gdk_draw_text (drawable, textfont, kontext, 10, */
/*  		   SCREEN_Y - 10, savetrackfn, strlen (savetrackfn)); */
	}
    }


  if (posmode)
    {
      blink = TRUE;
    }
#define PFSIZE 55
#define PFSIZE2 45


  if (havepos || blink)
    {
      if (posmode)
	{
	  gdk_gc_set_foreground (kontext, &blue);
	  gdk_gc_set_line_attributes (kontext, 4, 0, 0, 0);
	  gdk_draw_rectangle (drawable, kontext, 0, posx - 10,
			      posy - 10, 20, 20);
	}
      else
	{
	  if (shadow)
	    {
/*  draw shadow of  position marker */
	      gdk_gc_set_foreground (kontext, &darkgrey);
	      gdk_gc_set_line_attributes (kontext, 3, 0, 0, 0);
	      gdk_gc_set_function (kontext, GDK_AND);
	      gdk_draw_arc (drawable, kontext, 0, posx - 7 + SHADOWOFFSET,
			    posy - 7 + SHADOWOFFSET, 14, 14, 0, 360 * 64);
/*  draw pointer to destination */
	      gdk_gc_set_line_attributes (kontext, 4, 0, 0, 0);
/* 	      gdk_draw_line (drawable, kontext, */
/* 			     posx + 4 * sin (angle_to_destination) + */
/* 			     SHADOWOFFSET, */
/* 			     posy - 4 * cos (angle_to_destination) + */
/* 			     SHADOWOFFSET, */
/* 			     posx + 20 * sin (angle_to_destination) + */
/* 			     SHADOWOFFSET, */
/* 			     posy - 20 * cos (angle_to_destination) + */
/* 			     SHADOWOFFSET); */
	      w = angle_to_destination + M_PI;
	      poly[0].x =
		posx + SHADOWOFFSET + (PFSIZE) / 2.3 * (cos (w + M_PI_2));
	      poly[0].y =
		posy + SHADOWOFFSET + (PFSIZE) / 2.3 * (sin (w + M_PI_2));
	      poly[1].x =
		posx + SHADOWOFFSET + (PFSIZE) / 9 * (cos (w + M_PI));
	      poly[1].y =
		posy + SHADOWOFFSET + (PFSIZE) / 9 * (sin (w + M_PI));
	      poly[2].x =
		posx + SHADOWOFFSET + PFSIZE / 10 * (cos (w + M_PI_2));
	      poly[2].y =
		posy + SHADOWOFFSET + PFSIZE / 10 * (sin (w + M_PI_2));
	      poly[3].x =
		posx + SHADOWOFFSET - (PFSIZE) / 9 * (cos (w + M_PI));
	      poly[3].y =
		posy + SHADOWOFFSET - (PFSIZE) / 9 * (sin (w + M_PI));
	      poly[4].x = poly[0].x;
	      poly[4].y = poly[0].y;
	      gdk_draw_polygon (drawable, kontext, 1, (GdkPoint *) poly, 5);

/*  draw pointer to direction */
/* 	      gdk_draw_line (drawable, kontext, */
/* 			     posx + 4 * sin (direction) + SHADOWOFFSET, */
/* 			     posy - 4 * cos (direction) + SHADOWOFFSET, */
/* 			     posx + 20 * sin (direction) + SHADOWOFFSET, */
/* 			     posy - 20 * cos (direction) + SHADOWOFFSET); */
	      gdk_gc_set_line_attributes (kontext, 2, 0, 0, 0);
	      gdk_draw_arc (drawable, kontext, 0, posx + 2 - 7 + SHADOWOFFSET,
			    posy + 2 - 7 + SHADOWOFFSET, 10, 10, 0, 360 * 64);

	      w = direction + M_PI;
	      poly[0].x =
		posx + SHADOWOFFSET + (PFSIZE2) / 2.3 * (cos (w + M_PI_2));
	      poly[0].y =
		posy + SHADOWOFFSET + (PFSIZE2) / 2.3 * (sin (w + M_PI_2));
	      poly[1].x =
		posx + SHADOWOFFSET + (PFSIZE2) / 9 * (cos (w + M_PI));
	      poly[1].y =
		posy + SHADOWOFFSET + (PFSIZE2) / 9 * (sin (w + M_PI));
	      poly[2].x =
		posx + SHADOWOFFSET + PFSIZE2 / 10 * (cos (w + M_PI_2));
	      poly[2].y =
		posy + SHADOWOFFSET + PFSIZE2 / 10 * (sin (w + M_PI_2));
	      poly[3].x =
		posx + SHADOWOFFSET - (PFSIZE2) / 9 * (cos (w + M_PI));
	      poly[3].y =
		posy + SHADOWOFFSET - (PFSIZE2) / 9 * (sin (w + M_PI));
	      poly[4].x = poly[0].x;
	      poly[4].y = poly[0].y;
	      gdk_draw_polygon (drawable, kontext, 0, (GdkPoint *) poly, 5);
	      gdk_gc_set_function (kontext, GDK_COPY);
	    }
/*  draw real position marker */

	  gdk_gc_set_foreground (kontext, &black);
	  gdk_gc_set_line_attributes (kontext, 3, 0, 0, 0);
	  gdk_draw_arc (drawable, kontext, 0, posx - 7, posy - 7, 14,
			14, 0, 360 * 64);
/*  draw pointer to destination */

	  w = angle_to_destination + M_PI;

	  poly[0].x = posx + (PFSIZE) / 2.3 * (cos (w + M_PI_2));
	  poly[0].y = posy + (PFSIZE) / 2.3 * (sin (w + M_PI_2));
	  poly[1].x = posx + (PFSIZE) / 9 * (cos (w + M_PI));
	  poly[1].y = posy + (PFSIZE) / 9 * (sin (w + M_PI));
	  poly[2].x = posx + PFSIZE / 10 * (cos (w + M_PI_2));
	  poly[2].y = posy + PFSIZE / 10 * (sin (w + M_PI_2));
	  poly[3].x = posx - (PFSIZE) / 9 * (cos (w + M_PI));
	  poly[3].y = posy - (PFSIZE) / 9 * (sin (w + M_PI));
	  poly[4].x = poly[0].x;
	  poly[4].y = poly[0].y;
	  gdk_draw_polygon (drawable, kontext, 1, (GdkPoint *) poly, 5);

/*  draw pointer to direction */
	  gdk_gc_set_foreground (kontext, &red);
	  gdk_draw_arc (drawable, kontext, 0, posx + 2 - 7,
			posy + 2 - 7, 10, 10, 0, 360 * 64);
	  w = direction + M_PI;
	  poly[0].x = posx + (PFSIZE2) / 2.3 * (cos (w + M_PI_2));
	  poly[0].y = posy + (PFSIZE2) / 2.3 * (sin (w + M_PI_2));
	  poly[1].x = posx + (PFSIZE2) / 9 * (cos (w + M_PI));
	  poly[1].y = posy + (PFSIZE2) / 9 * (sin (w + M_PI));
	  poly[2].x = posx + PFSIZE2 / 10 * (cos (w + M_PI_2));
	  poly[2].y = posy + PFSIZE2 / 10 * (sin (w + M_PI_2));
	  poly[3].x = posx - (PFSIZE2) / 9 * (cos (w + M_PI));
	  poly[3].y = posy - (PFSIZE2) / 9 * (sin (w + M_PI));
	  poly[4].x = poly[0].x;
	  poly[4].y = poly[0].y;
	  gdk_draw_polygon (drawable, kontext, 0, (GdkPoint *) poly, 5);
	}
      if (markwaypoint)
	{
	  calcxy (&posxmarker, &posymarker, wplon, wplat, zoom);

	  gdk_gc_set_foreground (kontext, &green);
	  gdk_gc_set_line_attributes (kontext, 5, 0, 0, 0);
	  gdk_draw_arc (drawable, kontext, 0, posxmarker - 10,
			posymarker - 10, 20, 20, 0, 360 * 64);
	}
/*  If we are in position mode we set direction to zero to see where is the  */
/*  target  */
      if (posmode)
	direction = 0;

      bearing = angle_to_destination - direction;
      if (bearing < 0)
	bearing += 2 * M_PI;
      if (bearing > (2 * M_PI))
	bearing -= 2 * M_PI;
      display_status2 ();

    }



/*  now draw marker for destination point */

  calcxy (&posxdest, &posydest, target_long, target_lat, zoom);

  gdk_gc_set_line_attributes (kontext, 4, 0, 0, 0);
  if (shadow)
    {
/*  draw + sign at destination */
      gdk_gc_set_foreground (kontext, &darkgrey);
      gdk_gc_set_function (kontext, GDK_AND);
      gdk_draw_line (drawable, kontext, posxdest + 1 + SHADOWOFFSET,
		     posydest + 1 - 10 + SHADOWOFFSET,
		     posxdest + 1 + SHADOWOFFSET,
		     posydest + 1 - 2 + SHADOWOFFSET);
      gdk_draw_line (drawable, kontext, posxdest + 1 + SHADOWOFFSET,
		     posydest + 1 + 2 + SHADOWOFFSET,
		     posxdest + 1 + SHADOWOFFSET,
		     posydest + 1 + 10 + SHADOWOFFSET);
      gdk_draw_line (drawable, kontext, posxdest + 1 + 10 + SHADOWOFFSET,
		     posydest + 1 + SHADOWOFFSET,
		     posxdest + 1 + 2 + SHADOWOFFSET,
		     posydest + 1 + SHADOWOFFSET);
      gdk_draw_line (drawable, kontext, posxdest + 1 - 2 + SHADOWOFFSET,
		     posydest + 1 + SHADOWOFFSET,
		     posxdest + 1 - 10 + SHADOWOFFSET,
		     posydest + 1 + SHADOWOFFSET);
      gdk_gc_set_function (kontext, GDK_COPY);
    }

  if (crosstoogle)
    gdk_gc_set_foreground (kontext, &blue);
  else
    gdk_gc_set_foreground (kontext, &red);
  crosstoogle = !crosstoogle;
/*  draw + sign at destination */
  gdk_draw_line (drawable, kontext, posxdest + 1,
		 posydest + 1 - 10, posxdest + 1, posydest + 1 - 2);
  gdk_draw_line (drawable, kontext, posxdest + 1,
		 posydest + 1 + 2, posxdest + 1, posydest + 1 + 10);
  gdk_draw_line (drawable, kontext, posxdest + 1 + 10,
		 posydest + 1, posxdest + 1 + 2, posydest + 1);
  gdk_draw_line (drawable, kontext, posxdest + 1 - 2,
		 posydest + 1, posxdest + 1 - 10, posydest + 1);


/* display messages on map */
  display_dsc ();
/*  if distance is less then 1 km show meters */
  if (milesflag)
    {
      if (dist <= 1.0)
	{
	  g_snprintf (s2, sizeof (s2), "%.0f", dist * 1760.0);
	  g_strlcpy (s2a, "yrds", sizeof (s2a));
	}
      else
	{
	  if (dist <= 10.0)
	    {
	      g_snprintf (s2, sizeof (s2), "%.2f", dist);
	      g_strlcpy (s2a, "mi", sizeof (s2a));
	    }
	  else
	    {
	      g_snprintf (s2, sizeof (s2), "%.1f", dist);
	      g_strlcpy (s2a, "mi", sizeof (s2a));
	    }
	}
    }
  if (metricflag)
    {
      if (dist <= 1.0)
	{
	  g_snprintf (s2, sizeof (s2), "%.0f", dist * 1000.0);
	  g_strlcpy (s2a, "m", sizeof (s2a));
	}
      else
	{
	  if (dist <= 10.0)
	    {
	      g_snprintf (s2, sizeof (s2), "%.2f", dist);
	      g_strlcpy (s2a, "km", sizeof (s2a));
	    }
	  else
	    {
	      g_snprintf (s2, sizeof (s2), "%.1f", dist);
	      g_strlcpy (s2a, "km", sizeof (s2a));
	    }
	}
    }
  if (nauticflag)
    {
      if (dist <= 1.0)
	{
	  g_snprintf (s2, sizeof (s2), "%.3f", dist);
	  g_strlcpy (s2a, "nmi", sizeof (s2a));
	}
      else
	{
	  if (dist <= 10.0)
	    {
	      g_snprintf (s2, sizeof (s2), "%.2f", dist);
	      g_strlcpy (s2a, "nmi", sizeof (s2a));
	    }
	  else
	    {
	      g_snprintf (s2, sizeof (s2), "%.1f", dist);
	      g_strlcpy (s2a, "nmi", sizeof (s2a));
	    }
	}
    }
/*    display distance, speed and zoom */
/*   g_snprintf (s3, */
/* 	   "<span color=\"%s\" font_desc=\"%s\">%s</span><span color=\"%s\" font_desc=\"%s\">%s</span>", */
/* 	   bluecolor, bigfont, s2, bluecolor, "sans bold 18", s2a); */
  g_snprintf (s3, sizeof (s3),
	      "<span color=\"%s\" font_desc=\"%s\">%s<span size=\"16000\">%s</span></span>",
	      bluecolor, bigfont, s2, s2a);
  gtk_label_set_markup (GTK_LABEL (distlabel), s3);
/*    gtk_label_set_text (GTK_LABEL (distlabel), s2);  */
  if (milesflag)
    g_snprintf (s2, sizeof (s2), "%3.1f", groundspeed);
  if (metricflag)
    g_snprintf (s2, sizeof (s2), "%3.1f", groundspeed);
  if (nauticflag)
    g_snprintf (s2, sizeof (s2), "%3.1f", groundspeed);
  g_snprintf (s3, sizeof (s3),
	      "<span color=\"%s\" font_desc=\"%s\">%s</span>", bluecolor,
	      bigfont, s2);
  gtk_label_set_markup (GTK_LABEL (speedlabel), s3);

/*   gtk_label_set_text (GTK_LABEL (speedlabel), s2); */

  if (havealtitude)
    {
      if (milesflag || nauticflag)
	{
	  g_snprintf (s2, sizeof (s2), "%.0f",
		      altitude * 3.2808399 + normalnull);
	  g_strlcpy (s2a, "ft", sizeof (s2a));
	}
      else
	{
	  g_snprintf (s2, sizeof (s2), "%.0f", altitude + normalnull);
	  g_strlcpy (s2a, "m", sizeof (s2a));
	}
      gtk_label_set_text (GTK_LABEL (altilabel), s2);

      if (pdamode)
	{
	  if (normalnull == 0.0)

	    g_snprintf (s3, sizeof (s3),
			"<span color=\"%s\" font_family=\"Arial\" size=\"10000\">%s</span><span color=\"%s\" font_family=\"Arial\" size=\"5000\">%s</span>",
			bluecolor, s2, bluecolor, s2a);
	  else
	    g_snprintf (s3, sizeof (s3),
			"<span color=\"%s\" font_family=\"Arial\" size=\"10000\">%s</span><span color=\"%s\" font_family=\"Arial\" size=\"5000\">%s</span>"
			"<span color=\"red\" font_family=\"Arial\" size=\"5000\">\nNN %+.1f</span>",
			bluecolor, s2, bluecolor, s2a, normalnull);
	}
      else
	{
	  if (normalnull == 0.0)

	    g_snprintf (s3, sizeof (s3),
			"<span color=\"%s\" font_family=\"Arial\" weight=\"bold\" size=\"15000\">%s</span><span color=\"%s\" font_family=\"Arial\" weight=\"bold\" size=\"10000\">%s</span>",
			bluecolor, s2, bluecolor, s2a);
	  else
	    g_snprintf (s3, sizeof (s3),
			"<span color=\"%s\" font_family=\"Arial\" weight=\"bold\" size=\"15000\">%s</span><span color=\"%s\" font_family=\"Arial\" weight=\"bold\" size=\"10000\">%s</span>"
			"<span color=\"red\" font_family=\"Arial\" weight=\"bold\" size=\"8000\">\nNN %+.1f</span>",
			bluecolor, s2, bluecolor, s2a, normalnull);
	}
      gtk_label_set_markup (GTK_LABEL (altilabel), s3);
    }
  if (simmode)
    blink = TRUE;
  else
    {
      if (!havepos)
	blink = !blink;
    }

  if (newsatslevel)
    expose_sats_cb (NULL, 0);

  if (downloadwindowactive)
    {
      drawdownloadrectangle (1);
      expose_mini_cb (NULL, 0);
    }

/* force to say new direction */
  if (!strcmp (oldangle, "XXX"))
    speech_out_cb (NULL, 0);

  return (TRUE);
}




/* Copy Image from loaded map */
gint
expose_mini_cb (GtkWidget * widget, guint * datum)
{
/*  draw the minimap */
  if (!miniimage)
    return TRUE;

/*   g_print ("\nin expose_mini_cb"); */
  if (SMALLMENU == 0)
    {
      gdk_draw_pixbuf (drawing_miniimage->window,
		       kontext, miniimage, 0, 0, 0, 0, 128, 103,
		       GDK_RGB_DITHER_NONE, 0, 0);

/*       if ((nightmode == 1) || ((nightmode == 2) && (isnight&& !disableisnight)) */
/* 	{ */
/* 	  gdk_gc_set_function (kontext, GDK_AND); */
/* 	  gdk_gc_set_foreground (kontext, &nightcolor); */
/* 	  gdk_draw_rectangle (drawing_miniimage->window, kontext, 1, 0, 0, 128, */
/* 			      103); */
/* 	  gdk_gc_set_function (kontext, GDK_COPY); */
/* 	} */
      gdk_gc_set_foreground (kontext, &red);
      gdk_gc_set_line_attributes (kontext, 1, 0, 0, 0);

      gdk_draw_rectangle (drawing_miniimage->window, kontext, 0,
			  (64 - (SCREEN_X_2 / 10) / zoom) +
			  xoff / (zoom * 10),
			  (50 - (SCREEN_Y_2 / 10) / zoom) +
			  yoff / (zoom * 10), SCREEN_X / (zoom * 10),
			  SCREEN_Y / (zoom * 10));
      drawdownloadrectangle (0);
    }
  return TRUE;
}

gint
expose_compass (GtkWidget * widget, guint * datum)
{
  static GdkGC *compasskontext = NULL;
  gint l, l2, j;
  gint line_count;
  gchar txt[10], txt2[10], *txtp;
  gdouble w, kurz;
  GdkPoint poly[16];
  static GdkPixbuf *compassimage = NULL;

/*   This string means North,East,South,West -- please translate the letters */
  g_strlcpy (txt2, _("NESW"), sizeof (txt2));

  txtp = txt2;

  if (compasskontext == NULL)
    compasskontext = gdk_gc_new (drawable_bearing);
  if (compassimage == NULL)
    compassimage = gdk_pixbuf_new_from_xpm_data ((const char **) compass_xpm);


  gdk_draw_pixbuf (drawable_bearing, compasskontext, compassimage, 0, 0,
		   0, 0, PSIZE, PSIZE, GDK_RGB_DITHER_NONE, 0, 0);


  if (foundradar)
    {
      gdk_gc_set_foreground (kontext, &red);
      gdk_gc_set_line_attributes (kontext, 1, 0, 0, 0);
      w = radarbearing + M_PI;
      if (w < 0)
	w += 2 * M_PI;
      if (w > (2 * M_PI))
	w -= 2 * M_PI;
/*  	  g_print ("\nRadarbearing: %g w: %g", radarbearing, w); */
#define KURZW 1.2
      kurz = cos (KURZW);
      poly[0].x = PSIZE / 2 + PSIZE / 2.5 * cos (w);	/* x */
      poly[0].y = PSIZE / 2 + PSIZE / 2.5 * sin (w);	/* y */
      poly[1].x = PSIZE / 2 + PSIZE / 2 * cos (w + M_PI_2);
      poly[1].y = PSIZE / 2 + PSIZE / 2 * sin (w + M_PI_2);
      poly[2].x = PSIZE / 2 + PSIZE / 2.5 * cos (w + M_PI);
      poly[2].y = PSIZE / 2 + PSIZE / 2.5 * sin (w + M_PI);
      poly[3].x = PSIZE / 2 + PSIZE / 2 * kurz * cos (w + M_PI);
      poly[3].y = PSIZE / 2 + PSIZE / 2 * kurz * sin (w + M_PI);
      poly[4].x = PSIZE / 2 + PSIZE / 2 * cos (w + M_PI + KURZW);
      poly[4].y = PSIZE / 2 + PSIZE / 2 * sin (w + M_PI + KURZW);
      poly[5].x = PSIZE / 2 + PSIZE / 2 * cos (w + 2 * M_PI - KURZW);
      poly[5].y = PSIZE / 2 + PSIZE / 2 * sin (w + 2 * M_PI - KURZW);
      poly[6].x = PSIZE / 2 + PSIZE / 2 * kurz * cos (w);
      poly[6].y = PSIZE / 2 + PSIZE / 2 * kurz * sin (w);
      poly[7].x = poly[0].x;
      poly[7].y = poly[0].y;
      gdk_draw_polygon (drawable_bearing, kontext, 1, (GdkPoint *) poly, 8);
    }

  gdk_gc_set_foreground (kontext, &black);

/* compass */
//      /* added by zwerg (Daniel Wernle)
  w = -direction + M_PI;

  j = 0;

  for (line_count = 0; line_count < 12; line_count++)
    {
      gdk_gc_set_foreground (compasskontext, &red);

      if (!(line_count % 3))
	{
	  gdk_gc_set_line_attributes (compasskontext, 2, 0, 0, 0);


	  l = 10;
	  l2 = -18;
	  {
	    /* prints in pango */
	    PangoFontDescription *pfd;
	    PangoLayout *wplabellayout;
	    g_utf8_strncpy (txt, txtp, 1);
	    txtp = g_utf8_next_char (txtp);
	    wplabellayout =
	      gtk_widget_create_pango_layout (drawing_bearing, txt);
//KCFX  
	    if (pdamode)
	      pfd = pango_font_description_from_string ("Sans 7");
	    else
	      pfd = pango_font_description_from_string ("Sans 10");
	    pango_layout_set_font_description (wplabellayout, pfd);

	    gdk_draw_layout_with_colors (drawable_bearing, compasskontext,
					 (PSIZE / 2.0 +
					  (PSIZE) / 3.5 *
					  (cos (w + M_PI_2))) - l / 2,
					 (PSIZE / 2.0 +
					  (PSIZE) / 3.5 *
					  (sin (w + M_PI_2))) + l2 / 2,
					 wplabellayout, &black, NULL);
	    if (wplabellayout != NULL)
	      g_object_unref (G_OBJECT (wplabellayout));
	    /* freeing PangoFontDescription, cause it has been copied by prev. call */
	    pango_font_description_free (pfd);

	  }
	  gdk_gc_set_foreground (compasskontext, &red);
	}
      else
	{
	  gdk_gc_set_foreground (compasskontext, &black);
	  gdk_gc_set_line_attributes (compasskontext, 1, 0, 0, 0);
	}

/*       gdk_draw_line (drawable_bearing, compasskontext, */
/* 		     PSIZE / 2.0 + (PSIZE) / 2.75 * (cos (w + M_PI_2)), */
/* 		     PSIZE / 2.0 + (PSIZE) / 2.75 * (sin (w + M_PI_2)), */
/* 		     PSIZE / 2.0 + (PSIZE) / 2.0 * (cos (w + M_PI_2)), */
/* 		     PSIZE / 2.0 + (PSIZE) / 2.0 * (sin (w + M_PI_2))); */

      w = w + M_PI / 6.0;

    }

  w = bearing + M_PI;

#define TRIANGLEFACTOR 0.75
  gdk_gc_set_foreground (compasskontext, &black);

  gdk_gc_set_line_attributes (compasskontext, 1, 0, 0, 0);
/*   gdk_draw_arc (drawable_bearing, compasskontext, 0, 0, 0, PSIZE, PSIZE, */
/* 		0, 360 * 64); */

  poly[0].x = PSIZE / 2 + (PSIZE) / 2.3 * (cos (w + M_PI_2));
  poly[0].y = PSIZE / 2 + (PSIZE) / 2.3 * (sin (w + M_PI_2));
  poly[1].x = PSIZE / 2 + (PSIZE) / 9 * (cos (w + M_PI));
  poly[1].y = PSIZE / 2 + (PSIZE) / 9 * (sin (w + M_PI));
  poly[2].x = PSIZE / 2 + PSIZE / 10 * (cos (w + M_PI_2));
  poly[2].y = PSIZE / 2 + PSIZE / 10 * (sin (w + M_PI_2));
  poly[3].x = PSIZE / 2 - (PSIZE) / 9 * (cos (w + M_PI));
  poly[3].y = PSIZE / 2 - (PSIZE) / 9 * (sin (w + M_PI));
  poly[4].x = poly[0].x;
  poly[4].y = poly[0].y;

  gdk_gc_set_foreground (compasskontext, &black);
  gdk_gc_set_line_attributes (compasskontext, 2, 0, 0, 0);

  gdk_draw_polygon (drawable_bearing, compasskontext, 1,
		    (GdkPoint *) poly, 5);

  gdk_gc_set_foreground (kontext, &black);


//  von Zwerg eingefügt ende


  return TRUE;

}

/* Copy Image from loaded map */
#include "f_expose_cb.c"

/*  This is called in simulation mode, it moves the position to the  */
/*  selected destination */
gint
simulated_pos (GtkWidget * widget, guint * datum)
{
  gdouble ACCELMAX, ACCEL;
  gdouble secs, tx, ty, lastdirection;


  if (!simfollow)
    return TRUE;

  ACCELMAX = 0.00002 + dist / 30000.0;
  ACCEL = ACCELMAX / 20.0;
  long_diff += ACCEL * sin (angle_to_destination);
  lat_diff += ACCEL * cos (angle_to_destination);
  if (long_diff > ACCELMAX)
    long_diff = ACCELMAX;
  if (long_diff < -ACCELMAX)
    long_diff = -ACCELMAX;
  if (lat_diff > ACCELMAX)
    lat_diff = ACCELMAX;
  if (lat_diff < -ACCELMAX)
    lat_diff = -ACCELMAX;


  current_lat += lat_diff;
  current_long += long_diff;
  secs = g_timer_elapsed (timer, 0);
  if (secs >= 1.0)
    {
      g_timer_stop (timer);
      g_timer_start (timer);
      tx =
	(2 * R * M_PI / 360) * cos (M_PI * current_lat / 180.0) *
	(current_long - old_long);
      ty = (2 * R * M_PI / 360) * (current_lat - old_lat);
#define MINSPEED 1.0
      if (((fabs (tx)) > MINSPEED) || (((fabs (ty)) > MINSPEED)))
	{
	  lastdirection = direction;
	  if (ty == 0)
	    direction = 0.0;
	  else
	    direction = atan (tx / ty);
	  if (!finite (direction))
	    direction = lastdirection;

	  if (ty < 0)
	    direction = M_PI + direction;
	  if (direction >= (2 * M_PI))
	    direction -= 2 * M_PI;
	  if (direction < 0)
	    direction += 2 * M_PI;
	  groundspeed = milesconv * sqrt (tx * tx + ty * ty) * 3.6 / secs;
	}
      else
	groundspeed = 0.0;
      if (groundspeed > 999)
	groundspeed = 999;
      old_lat = current_lat;
      old_long = current_long;
      if (mydebug)
	g_print ("\nTime: %f", secs);
    }

  return TRUE;
}


/*  open serial port or pty master or file for NMEA output */
FILE *
opennmea (const char *name)
{
  struct termios tios;
  FILE *const out = fopen (name, "w");
  if (out == NULL)
    {
      perror (_("can't open NMEA output file"));
      exit (1);
    }

  if (tcgetattr (fileno (out), &tios))
    return out;			/* not a terminal, oh well */

  tios.c_iflag = 0;
  tios.c_oflag = 0;
  tios.c_cflag = CS8 | CLOCAL;
  tios.c_lflag = 0;
  tios.c_cc[VMIN] = 1;
  tios.c_cc[VTIME] = 0;
  cfsetospeed (&tios, B4800);
  tcsetattr (fileno (out), TCSAFLUSH, &tios);
  return out;
}

void
write_nmea_line (const char *line)
{
  int checksum = 0;
  fprintf (nmeaout, "$%s*", line);
  while ('\0' != *line)
    checksum = checksum ^ *line++;
  fprintf (nmeaout, "%02X\r\n", checksum);
  fflush (nmeaout);
}

void
gen_nmea_coord (char *out)
{
  gdouble lat = fabs (current_lat), lon = fabs (current_long);
  g_snprintf (out, sizeof (out), ",%02d%07.5f,%c,%03d%07.5f,%c",
	      (int) floor (lat), 60 * (lat - floor (lat)),
	      (current_lat < 0 ? 'S' : 'N'),
	      (int) floor (lon), 60 * (lon - floor (lon)),
	      (current_long < 0 ? 'W' : 'E'));
}

gint
write_nmea_cb (GtkWidget * widget, guint * datum)
{
  char buffer[180];
  time_t now = time (NULL);
  struct tm *st = gmtime (&now);

  strftime (buffer, sizeof (buffer), "GPGGA,%H%M%S.000", st);
  gen_nmea_coord (buffer + strlen (buffer));
  g_strlcpy (buffer + strlen (buffer), ",1,00,0.0,,M,,,,0000",
	     sizeof (buffer) - strlen (buffer));
  write_nmea_line (buffer);

  g_strlcpy (buffer, "GPGLL", sizeof (buffer));
  gen_nmea_coord (buffer + strlen (buffer));
  strftime (buffer + strlen (buffer), 80, ",%H%M%S.000,A", st);
  write_nmea_line (buffer);

  strftime (buffer, sizeof (buffer), "GPRMC,%H%M%S.000,A", st);
  gen_nmea_coord (buffer + strlen (buffer));
  g_snprintf (buffer + strlen (buffer), sizeof (buffer), ",%.2f,%.2f",
	      groundspeed / milesconv / 1.852, direction * 180.0 / M_PI);
  strftime (buffer + strlen (buffer), 80, ",%d%m%y,,", st);
  write_nmea_line (buffer);

  g_snprintf (buffer, sizeof (buffer), "GPVTG,%.2f,T,,M,%.2f,N,%.2f,K",
	      direction * 180.0 / M_PI,
	      groundspeed / milesconv / 1.852, groundspeed / milesconv);
  write_nmea_line (buffer);

  return TRUE;
}


/*  We load the map */
void
loadmap (char *filename)
{
  gchar mappath[600];
  GdkPixbuf *limage;
  guchar *lpixels, *pixels;
  int i, j, k;

  if (maploaded)
    gdk_pixbuf_unref (image);

  if (strstr (filename, "/map_")){
    mapistopo = FALSE;
  } else if (strstr (filename, "/top_")){
    mapistopo = TRUE;
  } else if (!(strncmp (filename, "top_", 4)))
    mapistopo = TRUE;
  else
    mapistopo = FALSE;

  if ((strcmp (filename, "top_GPSWORLD.jpg")) == 0)
    {
      g_snprintf (mappath, sizeof (mappath), "%s/gpsdrive/%s", DATADIR,
		  "top_GPSWORLD.jpg");
      limage = gdk_pixbuf_new_from_file (mappath, NULL);
      if (limage == NULL)
	havedefaultmap = FALSE;
    }
  else
    {
      g_strlcpy (mappath, mapdir, sizeof (mappath));
      g_strlcat (mappath, filename, sizeof (mappath));
      limage = gdk_pixbuf_new_from_file (mappath, NULL);
    }

  if (limage == NULL)
    {
      GString *error;
      error = g_string_new (NULL);
      g_string_sprintf (error, "\n%s\n%s\n",
			_(" Mapfile could not be loaded:"), mappath);
      error_popup ((gpointer *) error->str);
      g_string_free (error, TRUE);
      maploaded = FALSE;
      return;
    }


  if (!gdk_pixbuf_get_has_alpha (limage))
    image = limage;
  else
    {
      image = gdk_pixbuf_new (GDK_COLORSPACE_RGB, 0, 8, 1280, 1024);
      if (image == NULL)
	{
	  fprintf (stderr,
		   "can't get image  gdk_pixbuf_new (GDK_COLORSPACE_RGB, 0, 8, 1280, 1024)\n");
	  exit (1);
	}
      lpixels = gdk_pixbuf_get_pixels (limage);
      pixels = gdk_pixbuf_get_pixels (image);
      if (pixels == NULL)
	{
	  fprintf (stderr,
		   "can't get pixels pixels = gdk_pixbuf_get_pixels (image);\n");
	  exit (1);
	}
      j = k = 0;
      for (i = 0; i < (1280 * 1024); i++)
	{
	  memcpy ((pixels + j), (lpixels + k), 3);
	  j += 3;
	  k += 4;
	}
      gdk_pixbuf_unref (limage);

    }

  expose_cb (NULL, NULL);
  iszoomed = FALSE;
/*        zoom = 1; */
  xoff = yoff = 0;
  rebuildtracklist ();
  if (!maploaded)
    display_status (_("Map found!"));

  maploaded = TRUE;
/*  draw minimap */
  if (miniimage)
    gdk_pixbuf_unref (miniimage);

  miniimage = gdk_pixbuf_new (GDK_COLORSPACE_RGB, 0, 8, 128, 103);

  gdk_pixbuf_scale (image, miniimage, 0, 0, 128, 103,
		    0, 0, 0.1, 0.10, GDK_INTERP_TILES);
  expose_mini_cb (NULL, 0);



}


void
loadfriendsicon (void)
{
  gchar mappath[400];

  g_snprintf (mappath, sizeof (mappath), "%s/gpsdrive/%s", DATADIR,
	      "friendsicon.png");
  friendsimage = gdk_pixbuf_new_from_file (mappath, NULL);
  if (friendsimage == NULL)
    {
      friendsimage = gdk_pixbuf_new_from_file ("friendsicon.png", NULL);
    }
  if (friendsimage == NULL)
    {
      GString *error;
      error = g_string_new (NULL);
      g_string_sprintf (error, "\n%s\n%s\n",
			_(" Friendsicon could not be loaded:"), mappath);
      fprintf (stderr,
	       _
	       ("\nWarning: unable to load friendsicon!\nPlease install the program as root with:\nmake install\n\n"));

      error_popup ((gpointer *) error->str);
      g_string_free (error, TRUE);
    }
  friendspixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, 1, 8, 39, 24);
  gdk_pixbuf_scale (friendsimage, friendspixbuf, 0, 0, 39, 24,
		    0, 0, 1, 1, GDK_INTERP_BILINEAR);

}

gint
zoom_cb (GtkWidget * widget, guint datum)
{

  if (iszoomed == FALSE)	/* needed to be sure the old zoom is made */
    return TRUE;
  iszoomed = FALSE;
  if (datum == 1)
    {
      if (zoom >= 16)
	{
	  iszoomed = TRUE;
	  return TRUE;
	}
      zoom *= 2;
      xoff *= 2;
      yoff *= 2;
    }
  else
    {
      if (zoom <= 1)
	{
	  zoom = 1;
	  iszoomed = TRUE;
	}
      else
	{
	  zoom /= 2;
	  xoff /= 2;
	  yoff /= 2;
	}
    }
  if (importactive)
    {
      expose_cb (NULL, 0);
      expose_mini_cb (NULL, 0);
    }
  return TRUE;
}


gint
scalerbt_cb (GtkWidget * widget, guint datum)
{
  gint val, oldval, old2val;
  gchar oldfilename[200];

  g_strlcpy (oldfilename, mapfilename, sizeof (oldfilename));
  val = (GTK_ADJUSTMENT (adj)->value);
  old2val = val;

  do
    {
      oldval = val;
      if (datum == 1)
	{
	  gtk_adjustment_set_value (GTK_ADJUSTMENT (adj), val + 1);
	}
      else
	{
	  gtk_adjustment_set_value (GTK_ADJUSTMENT (adj), val - 1);
	}
      val = (GTK_ADJUSTMENT (adj)->value);

      testnewmap ();
    }
  while (((strcmp (oldfilename, mapfilename)) == 0) && (val != oldval));

  if ((strcmp (oldfilename, mapfilename)) == 0)
    val = old2val;
  gtk_adjustment_set_value (GTK_ADJUSTMENT (adj), val);
  expose_cb (NULL, 0);
  expose_mini_cb (NULL, 0);

  needtosave = TRUE;

  return TRUE;
}

/*  destroy sel_target window */
gint
sel_targetweg_cb (GtkWidget * widget, guint datum)
{
/*   gtk_timeout_remove (selwptimeout); */
  gtk_widget_destroy (GTK_WIDGET (gotowindow));
  setwpactive = FALSE;

  return FALSE;
}

/*  cancel sel_route window */
gint
sel_routecancel_cb (GtkWidget * widget, guint datum)
{
  gchar str[200];


  gtk_widget_destroy (GTK_WIDGET (routewindow));

  g_snprintf (str, sizeof (str), "%s: %s", _("To"), targetname);
  gtk_frame_set_label (GTK_FRAME (destframe), str);
  createroute = FALSE;
  routemode = FALSE;
  routepointer = routeitems = 0;
  gtk_widget_set_sensitive (create_route_button, TRUE);
  return FALSE;
}

/*  destroy sel_route window but continue routing*/
gint
sel_routeclose_cb (GtkWidget * widget, guint datum)
{
  gtk_widget_destroy (GTK_WIDGET (routewindow));
  gtk_widget_set_sensitive (create_route_button, TRUE);
  return FALSE;
}

gint
do_route_cb (GtkWidget * widget, guint datum)
{

  gtk_widget_destroy (GTK_WIDGET (routewindow));
  gtk_widget_set_sensitive (create_route_button, TRUE);
  createroute = FALSE;
  routemode = TRUE;
  setroutetarget (NULL, -1);
  return FALSE;
}

gint
dlstatusaway_cb (GtkWidget * widget, guint datum)
{


  downloadwindowactive = downloadactive = FALSE;
  gtk_widget_set_sensitive (downloadbt, TRUE);

  return FALSE;
}

gint
importaway_cb (GtkWidget * widget, guint datum)
{
  importactive = FALSE;
  gtk_widget_destroy (widget);
  g_strlcpy (oldfilename, "XXXXXXXXXXXXXXXXXX", sizeof (oldfilename));
  return FALSE;
}

/*  cancel button pressed or widget destroy in download_cb */
gint
downloadaway_cb (GtkWidget * widget, guint datum)
{
  downloadwindowactive = downloadactive = FALSE;
  gtk_widget_destroy (widget);
  expose_mini_cb (NULL, 0);
  gtk_widget_set_sensitive (downloadbt, TRUE);

/*    gdk_window_set_cursor (drawing_area->window, 0); */
/*    gdk_cursor_destroy (cursor); */
  return FALSE;
}

#ifdef DONTKNOWWHATTHISSHITWAS
gint
testnewwp_cb (GtkWidget * widget, guint datum)
{
  gint e;
  e = kill (datum, 0);
  if (e != 0)
    {
      loadwaypoints ();
      return FALSE;
    }
  return TRUE;
}

#endif

gint
other_select_cb (GtkWidget * widget, guint datum)
{
  gint i, e;
  gchar buff[300], mappath[500];
  struct stat buf;

  i = 0;
  do
    {
      if (mapdir[strlen (mapdir) - 1] != '/')
	g_strlcat (mapdir, "/", sizeof (mapdir));

      g_strlcpy (mappath, mapdir, sizeof (mappath));

      g_snprintf (downloadfilename, sizeof (downloadfilename),
		  "%smap_file%04d.gif", mappath, i++);
      e = stat (downloadfilename, &buf);
    }
  while (e == 0);
  g_snprintf (buff, sizeof (buff), "map_file%04d.gif", i - 1);
  gtk_entry_set_text (GTK_ENTRY (dltext4), buff);

  return TRUE;
}





gint
dlscale_cb (GtkWidget * widget, guint datum)
{
  G_CONST_RETURN gchar *sc;
  gchar t[100], t2[10];
  gdouble f;
/* PORTING */
  sc = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (dltext3)->entry));

  f = g_strtod (sc, NULL);

  g_strlcpy (t2, "km", sizeof (t2));

  if (milesflag)
    g_strlcpy (t2, "mi", sizeof (t2));
  if (nauticflag)
    g_strlcpy (t2, "nmi", sizeof (t2));

  g_snprintf (t, sizeof (t), "%.3f x %.3f %s",
	      milesconv * 1.280 * f / PIXELFACT,
	      milesconv * 1.024 * f / PIXELFACT, t2);
  gtk_entry_set_text (GTK_ENTRY (cover), t);
  return TRUE;
}


gint
download_cb (GtkWidget * widget, guint datum)
{
  GtkWidget *mainbox;
  GtkWidget *knopf2, *knopf, *knopf3, *knopf4, *knopf5, *knopf6, *knopf7;
  GtkWidget *table, *table2, *knopf8;
  gchar buff[300], mappath[500];
  GList *list = NULL;
  GSList *gr;
  gint i, e;
  struct stat buf;
  gchar scalewanted_str[100];
  GtkTooltips *tooltips;

  gtk_widget_set_sensitive (downloadbt, FALSE);

  for (i = 0; i < 30; i++)
    list = g_list_append (list, slist[i]);
  downloadwindow = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (downloadwindow),
			_("Select coordinates and scale"));
  gtk_container_set_border_width (GTK_CONTAINER (downloadwindow), 5);
  mainbox = gtk_vbox_new (TRUE, 2);
  knopf = gtk_button_new_with_label (_("Download map"));
  gtk_signal_connect (GTK_OBJECT (knopf), "clicked",
		      GTK_SIGNAL_FUNC (downloadsetparm), (gpointer) 1);
  knopf2 = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
  gtk_signal_connect_object (GTK_OBJECT (knopf2), "clicked",
			     GTK_SIGNAL_FUNC
			     (downloadaway_cb), GTK_OBJECT (downloadwindow));
  gtk_signal_connect_object (GTK_OBJECT (downloadwindow), "delete_event",
			     GTK_SIGNAL_FUNC (downloadaway_cb),
			     GTK_OBJECT (downloadwindow));
  cover = gtk_entry_new ();
  gtk_editable_set_editable (GTK_EDITABLE (cover), FALSE);
  gtk_signal_connect (GTK_OBJECT (cover), "changed",
		      GTK_SIGNAL_FUNC (downloadsetparm), (gpointer) 0);
  gtk_box_pack_start (GTK_BOX
		      (GTK_DIALOG (downloadwindow)->
		       action_area), knopf, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX
		      (GTK_DIALOG (downloadwindow)->
		       action_area), knopf2, TRUE, TRUE, 2);
  GTK_WIDGET_SET_FLAGS (knopf, GTK_CAN_DEFAULT);
  GTK_WIDGET_SET_FLAGS (knopf2, GTK_CAN_DEFAULT);
  table = gtk_table_new (8, 2, FALSE);
  gtk_box_pack_start (GTK_BOX
		      (GTK_DIALOG (downloadwindow)->vbox),
		      table, TRUE, TRUE, 2);
  knopf3 = gtk_label_new (_("Latitude"));
  gtk_table_attach_defaults (GTK_TABLE (table), knopf3, 0, 1, 0, 1);
  knopf4 = gtk_label_new (_("Longitude"));
  gtk_table_attach_defaults (GTK_TABLE (table), knopf4, 0, 1, 1, 2);
  knopf8 = gtk_label_new (_("Map covers"));
  gtk_table_attach_defaults (GTK_TABLE (table), knopf8, 0, 1, 2, 3);
  gtk_table_attach_defaults (GTK_TABLE (table), cover, 1, 2, 2, 3);

  knopf5 = gtk_label_new (_("Scale"));
  gtk_table_attach_defaults (GTK_TABLE (table), knopf5, 0, 1, 3, 4);
  knopf6 = gtk_label_new (_("Map file name"));
  gtk_table_attach_defaults (GTK_TABLE (table), knopf6, 0, 1, 4, 5);
  dltext1 = gtk_entry_new ();
  gtk_signal_connect (GTK_OBJECT (dltext1), "changed",
		      GTK_SIGNAL_FUNC (downloadsetparm), (gpointer) 0);

  gtk_table_attach_defaults (GTK_TABLE (table), dltext1, 1, 2, 0, 1);
  g_snprintf (buff, sizeof (buff), "%.5f", current_lat);
  if (minsecmode)
    decimaltomin (buff, 1);

  gtk_entry_set_text (GTK_ENTRY (dltext1), buff);
  dltext2 = gtk_entry_new ();
  gtk_signal_connect (GTK_OBJECT (dltext2), "changed",
		      GTK_SIGNAL_FUNC (downloadsetparm), (gpointer) 0);
  gtk_table_attach_defaults (GTK_TABLE (table), dltext2, 1, 2, 1, 2);
  g_snprintf (buff, sizeof (buff), "%.5f", current_long);
  if (minsecmode)
    decimaltomin (buff, 0);
  gtk_entry_set_text (GTK_ENTRY (dltext2), buff);
  dltext3 = gtk_combo_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), dltext3, 1, 2, 3, 4);
  gtk_combo_set_popdown_strings (GTK_COMBO (dltext3), (GList *) list);
  g_snprintf (scalewanted_str, sizeof (scalewanted_str), "%d", scalewanted);
  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (dltext3)->entry),
		      scalewanted_str);
  gtk_signal_connect (GTK_OBJECT (GTK_COMBO (dltext3)->entry), "changed",
		      GTK_SIGNAL_FUNC (downloadsetparm), (gpointer) 0);

  dltext4 = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), dltext4, 1, 2, 4, 5);
  gtk_signal_connect (GTK_OBJECT (GTK_COMBO (dltext3)->entry), "changed",
		      GTK_SIGNAL_FUNC (dlscale_cb), 0);
  gtk_signal_connect (GTK_OBJECT (dltext4), "changed",
		      GTK_SIGNAL_FUNC (dlscale_cb), 0);

  table2 = gtk_table_new (2, 1, FALSE);	//nested table w/ three columns
  gtk_table_attach_defaults (GTK_TABLE (table), table2, 0, 3, 5, 6);
  gtk_widget_show (table2);

  radio1 = gtk_radio_button_new_with_label (NULL, _("Expedia Germany"));
  gtk_table_attach_defaults (GTK_TABLE (table2), radio1, 0, 1, 0, 1);
  gr = gtk_radio_button_group (GTK_RADIO_BUTTON (radio1));
  gtk_signal_connect (GTK_OBJECT (radio1), "clicked",
		      GTK_SIGNAL_FUNC (other_select_cb), 0);

  radio2 = gtk_radio_button_new_with_label (gr, _("Expedia USA"));
  gtk_table_attach_defaults (GTK_TABLE (table2), radio2, 1, 2, 0, 1);
  gr = gtk_radio_button_group (GTK_RADIO_BUTTON (radio2));
  gtk_signal_connect (GTK_OBJECT (radio2), "clicked",
		      GTK_SIGNAL_FUNC (other_select_cb), 0);



  tooltips = gtk_tooltips_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), radio1,
			_
			("If selected, you download the map from the german expedia server (expedia.de)"),
			NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), radio2,
			_
			("If selected, you download the map from the U.S. expedia server (expedia.com)"),
			NULL);

/* disable mapblast */
/*   gtk_widget_set_sensitive (radio1, FALSE); */
/*   defaultserver = 1; */

/*   gtk_table_attach_defaults (GTK_TABLE (table), radio2, 1, 2, 5, 6); */
  if ((defaultserver < 0) && (defaultserver > 1))
    defaultserver = 0;
  switch (defaultserver)
    {
    case 0:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio1), TRUE);
      other_select_cb (NULL, 0);
      break;
    case 1:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio2), TRUE);
      other_select_cb (NULL, 0);
      break;
    }

  if (!haveproxy)
    g_snprintf (buff, sizeof (buff), "%s",
		_("You can also select the position\n"
		  "with a mouse click on the map."));
  else
    g_snprintf (buff, sizeof (buff), "%s\n\n%s    %s %d",
		_("You can also select the position\n"
		  "with a mouse click on the map."),
		_("Using Proxy and port:"), proxy, proxyport);
  knopf7 = gtk_label_new (buff);
  gtk_table_attach_defaults (GTK_TABLE (table), knopf7, 0, 2, 6, 7);
  myprogress = gtk_progress_bar_new ();
  gtk_progress_set_format_string (GTK_PROGRESS (myprogress), "%p%%");
  gtk_progress_set_show_text (GTK_PROGRESS (myprogress), TRUE);
  gtk_progress_bar_update (GTK_PROGRESS_BAR (myprogress), 0.0);
  gtk_table_attach_defaults (GTK_TABLE (table), myprogress, 0, 2, 7, 8);
  gtk_label_set_justify (GTK_LABEL (knopf6), GTK_JUSTIFY_RIGHT);
  gtk_label_set_justify (GTK_LABEL (knopf3), GTK_JUSTIFY_RIGHT);
  gtk_label_set_justify (GTK_LABEL (knopf4), GTK_JUSTIFY_RIGHT);
  gtk_label_set_justify (GTK_LABEL (knopf5), GTK_JUSTIFY_RIGHT);
  gtk_label_set_justify (GTK_LABEL (knopf6), GTK_JUSTIFY_RIGHT);
  i = 0;
  do
    {
      if (mapdir[strlen (mapdir) - 1] != '/')
	g_strlcat (mapdir, "/", sizeof (mapdir));

      g_strlcpy (mappath, mapdir, sizeof (mappath));

      g_snprintf (downloadfilename, sizeof (downloadfilename),
		  "%smap_file%04d.gif", mappath, i++);
      e = stat (downloadfilename, &buf);
    }
  while (e == 0);
  g_snprintf (buff, sizeof (buff), "map_file%04d.gif", i - 1);
  gtk_entry_set_text (GTK_ENTRY (dltext4), buff);
  gtk_window_set_default (GTK_WINDOW (downloadwindow), knopf);
  gtk_window_set_transient_for (GTK_WINDOW (downloadwindow),
				GTK_WINDOW (mainwindow));
  gtk_window_set_position (GTK_WINDOW (downloadwindow), GTK_WIN_POS_CENTER);
  gtk_widget_show_all (downloadwindow);
  downloadwindowactive = TRUE;
  downloadsetparm (NULL, 0);

/*    cursor = gdk_cursor_new (GDK_CROSS); */
/*    gdk_window_set_cursor (drawing_area->window, cursor); */
  return TRUE;
}

gint
downloadsetparm (GtkWidget * widget, guint datum)
{
  G_CONST_RETURN gchar *s, *sc;
  gchar longi[100], lat[100], hostname[100], region[10];
  gdouble f, nlongi;
  gint ns;

  float Lat, Long;
  char sctext[40];

  if (!downloadwindowactive)
    return TRUE;

  expedia_de = FALSE;

  if (GTK_TOGGLE_BUTTON (radio2)->active)
    expedia = TRUE;
  else
    expedia = FALSE;
  if (GTK_TOGGLE_BUTTON (radio1)->active)
    {
      expedia = TRUE;
      expedia_de = TRUE;
    }



  s = gtk_entry_get_text (GTK_ENTRY (dltext1));
  g_strlcpy (lat, s, sizeof (lat));
  checkinput (lat);
  g_strlcpy (newmaplat, lat, sizeof (newmaplat));
  g_strdelimit (lat, ",", '.');
  Lat = atof (lat);
  s = gtk_entry_get_text (GTK_ENTRY (dltext2));
  g_strlcpy (longi, s, sizeof (longi));
  checkinput (longi);
  nlongi = g_strtod (longi, NULL);
  Long = atof (longi);

  g_strlcpy (newmaplongi, longi, sizeof (newmaplongi));
  g_strdelimit (longi, ",", '.');
  sc = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (dltext3)->entry));
  g_strlcpy (sctext, sc, sizeof (sctext));
  g_strlcpy (newmapsc, sctext, sizeof (newmapsc));

/*   g_print ("\nnewmaplat: %s, newmaplongi: %s newmapsc: %s", newmaplat, */
/* 	   newmaplongi, newmapsc); */

  if (datum == 0)
    return TRUE;
  if (expedia)
    {
      if (expedia_de)
	g_snprintf (hostname, sizeof (hostname), "%s", WEBSERVER4);
      else
	g_snprintf (hostname, sizeof (hostname), "%s", WEBSERVER2);
    }

  if (!expedia)
    g_snprintf (hostname, sizeof (hostname), "%s", WEBSERVER);

/*   if (expedia) */
/*     { */
/*       f = g_strtod (sc, NULL); */
/*       ns = f / EXPEDIAFACT; */
/*       g_snprintf ((char *) sc, "%d", ns); */
/*       g_snprintf (newmapsc, "%d", (int) (ns * EXPEDIAFACT)); */
/*     } */
  if (expedia)
    {
      int scales[11] = { 1, 3, 6, 12, 25, 50, 150, 800, 2000, 7000, 12000 };
      int i, found = 5;
      double di = 999999;
      f = g_strtod (sctext, NULL);
      ns = f / EXPEDIAFACT;
      for (i = 0; i < 11; i++)
	if (abs (ns - scales[i]) < di)
	  {
	    di = abs (ns - scales[i]);
	    found = i;
	  }
      ns = scales[found];
      g_snprintf (sctext, sizeof (sctext), "%d", ns);
      g_snprintf (newmapsc, sizeof (newmapsc), "%d",
		  (int) (ns * EXPEDIAFACT));
    }
  if (debug)
    printf ("\nsctext: %s,newmapsc: %s\n", sctext, newmapsc);

/*   new URL (08/28/2002) */
/* http://www.mapblast.com/myblastd/MakeMap.d?&CT=48.0:12.2:100000&IC=&W=1280&H=1024&FAM=myblast&LB= */
/*   new URL (April 2003)
 *   http://www.vicinity.com/gif?&CT=45:-93:10000&IC=&W=1024&H=800&FAM=myblast&LB=
 */

  if (!expedia)
    g_snprintf (writebuff, sizeof (writebuff),
		"GET http://%s/gif?&CT=%s:%s:%s&IC=&W=1280&H=1024&FAM=myblast&LB= HTTP/1.0\r\nUser-Agent: Wget/1.6\r\nHost: %s\r\nAccept: */*\r\nConnection: Keep-Alive\r\n\r\n",
		WEBSERVER, lat, longi, sctext, hostname);
  if (expedia)
    {
      if (nlongi > (-30))
	g_strlcpy (region, "EUR0809", sizeof (region));
      else
	g_strlcpy (region, "USA0409", sizeof (region));

      if (expedia_de)
	g_snprintf (writebuff, sizeof (writebuff),
		    "GET http://%s/pub/agent.dll?qscr=mrdt&ID=3XNsF.&CenP=%s,%s&Lang=%s&Alti=%s&Size=1280,1024&Offs=0.000000,0.000000& HTTP/1.1\r\nUser-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0)\r\nHost: %s\r\nAccept: */*\r\nCookie: jscript=1\r\n\r\n",
		    WEBSERVER4, lat, longi, region, sctext, hostname);
      else
	g_snprintf (writebuff, sizeof (writebuff),
		    "GET http://%s/pub/agent.dll?qscr=mrdt&ID=3XNsF.&CenP=%s,%s&Lang=%s&Alti=%s&Size=1280,1024&Offs=0.000000,0.000000& HTTP/1.1\r\nUser-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0)\r\nHost: %s\r\nAccept: */*\r\nCookie: jscript=1\r\n\r\n",
		    WEBSERVER2, lat, longi, region, sctext, hostname);
    }

  if (debug)
    g_print ("\nDownload URL: %s\n", writebuff);

  if (!expedia)
    downloadstart_cb (widget, 0);
  else
    {
      char url[2000], url2[2000], hn[200], *p;

      p = getexpediaurl (widget);
      if (p == NULL)
	{
	  return FALSE;
	}

      g_strlcpy (url, p, sizeof (url));
      if (debug)
	printf ("\n%s\n", url);
      p = strstr (url, "Location: ");
      if (p == NULL)
	{
	  if (debug)
	    printf
	      ("http data error, could not find 'Location:' sub string\n");
	  return FALSE;
	}
      g_strlcpy (url2, (p + 10), sizeof (url2));
      p = strstr (url2, "\n");
      if (p == NULL)
	{
	  if (debug)
	    printf ("http data error, could not find new line\n");
	  return FALSE;
	}

      url2[p - url2] = 0;
      if (debug)
	printf ("\n**********\n%s\n", url2);
      g_strlcpy (hn, (url2 + 7), sizeof (hn));
      p = strstr (hn, "/");
      if (p == NULL)
	{
	  if (debug)
	    printf ("http request error, could not find forward slash\n");
	  return FALSE;
	}

      hn[p - hn] = 0;
      g_strlcpy (url, (url2 + strlen (hn) + 7), sizeof (url));
      url[strlen (url) - 1] = 0;
      g_strlcpy (actualhostname, hn, sizeof (actualhostname));
      if (debug)
	printf ("\nhn: %s, url: %s", hn, url);
      
      if(haveproxy==TRUE)
	{
   		// Format the GET request correctly for the proxy server
		g_snprintf (url2, sizeof (url2), "GET http://%s/%s HTTP/1.1\r\n", hn,url);
   	}
	else
	{
		g_snprintf (url2, sizeof (url2), "GET %s HTTP/1.1\r\n", url);
	}
      
      g_strlcat (url2, "Host: ", sizeof (url2));
      g_strlcat (url2, hn, sizeof (url2));
      g_strlcat (url2, "\r\n", sizeof (url2));
      g_strlcat (url2,
		 "User-Agent: Mozilla/5.0 Galeon/1.2.8 (X11; Linux i686; U;) Gecko/20030317\r\n",
		 sizeof (url2));
      g_strlcat (url2,
		 "Accept: text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,video/x-mng,image/png,image/jpeg,image/gif;q=0.2,text/css,*/*;q=0.1\r\n",
		 sizeof (url2));
      g_strlcat (url2, "Accept-Language: de, en;q=0.50\r\n", sizeof (url2));
      g_strlcat (url2, "Accept-Encoding: gzip, deflate, compress;q=0.9\r\n",
		 sizeof (url2));
      g_strlcat (url2,
		 "Accept-Charset: ISO-8859-15, utf-8;q=0.66, *;q=0.66\r\n",
		 sizeof (url2));
      g_strlcat (url2, "Keep-Alive: 300\r\n", sizeof (url2));
      g_strlcat (url2, "Connection: keep-alive\r\n\r\n", sizeof (url2));

      g_strlcpy (writebuff, url2, sizeof (writebuff));
      if (debug)
	printf ("\nurl2:\n%s\n**********\n\n%s\n-----------------\n", url2,
		writebuff);

      downloadstart_cb (widget, 0);

/*       exit (1); */
    }
  return TRUE;
}


char *
getexpediaurl (GtkWidget * widget)
{
  struct sockaddr_in server;
  struct hostent *server_data;
  gchar str[100], sn[1000];
  gchar tmpbuff[9000];
  gint e;
  static char url[8000];

/*  open socket to port80 */
  if ((dlsock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
      perror (_("can't open socket for port 80"));
      if (expedia_de)
	g_snprintf (str, sizeof (str), _("Connecting to %s FAILED!"),
		    (expedia) ? WEBSERVER4 : WEBSERVER);
      else
	g_snprintf (str, sizeof (str), _("Connecting to %s FAILED!"),
		    (expedia) ? WEBSERVER2 : WEBSERVER);
      gtk_statusbar_pop (GTK_STATUSBAR (status), statusid);
      gtk_statusbar_push (GTK_STATUSBAR (status), statusid, str);
      gtk_widget_destroy (downloadwindow);
      gtk_timeout_add (3000, (GtkFunction) dlstatusaway_cb, widget);
      return (NULL);
    }

  server.sin_family = AF_INET;
/*  We retrieve the IP address of the server from its name: */
  if (haveproxy)
    g_strlcpy (sn, proxy, sizeof (sn));
  else
    {
      if (expedia_de)
	g_strlcpy (sn, (expedia) ? WEBSERVER4 : WEBSERVER, sizeof (sn));
      else
	g_strlcpy (sn, (expedia) ? WEBSERVER2 : WEBSERVER, sizeof (sn));
    }
  if ((server_data = gethostbyname (sn)) == NULL)
    {
      perror (_("Can't resolve webserver address"));
      if (expedia_de)
	g_snprintf (str, sizeof (str), _("Connecting to %s FAILED!"),
		    (expedia) ? WEBSERVER4 : WEBSERVER);
      else
	g_snprintf (str, sizeof (str), _("Connecting to %s FAILED!"),
		    (expedia) ? WEBSERVER2 : WEBSERVER);
      gtk_statusbar_pop (GTK_STATUSBAR (status), statusid);
      gtk_statusbar_push (GTK_STATUSBAR (status), statusid, str);
      gtk_widget_destroy (downloadwindow);
      gtk_timeout_add (3000, (GtkFunction) dlstatusaway_cb, widget);
      return (NULL);
    }
  memcpy (&server.sin_addr, server_data->h_addr, server_data->h_length);
  server.sin_port = htons (proxyport);
/*  We initiate the connection  */
  if (connect (dlsock, (struct sockaddr *) &server, sizeof server) < 0)
    {
      perror (_("unable to connect to Website"));
      if (expedia_de)
	g_snprintf (str, sizeof (str), _("Connecting to %s FAILED!"),
		    (expedia) ? WEBSERVER4 : WEBSERVER);
      else
	g_snprintf (str, sizeof (str), _("Connecting to %s FAILED!"),
		    (expedia) ? WEBSERVER2 : WEBSERVER);
      gtk_statusbar_pop (GTK_STATUSBAR (status), statusid);
      gtk_statusbar_push (GTK_STATUSBAR (status), statusid, str);
      gtk_widget_destroy (downloadwindow);
      gtk_timeout_add (3000, (GtkFunction) dlstatusaway_cb, widget);
      return (NULL);
    }

  write (dlsock, writebuff, strlen (writebuff));

  FD_ZERO (&readmask);
  FD_SET (dlsock, &readmask);
  timeout.tv_sec = 0;
  timeout.tv_usec = 100000;
  if (select (FD_SETSIZE, &readmask, NULL, NULL, &timeout) < 0)
    {
      perror ("select() call");
    }

  g_strlcpy (url, "Fehler!!!!", sizeof (url));
  memset (tmpbuff, 0, 8192);
  if ((e = read (dlsock, tmpbuff, 8000)) < 0)
    perror (_("read from Webserver"));
  if (debug)
    g_print ("\nLoaded %d Bytes\n", e);
  if (e > 0)
    g_strlcpy (url, tmpbuff, sizeof (url));
  else
    {
      perror ("getexpediaurl");
      fprintf (stderr, "error while reading from exedia\n");
      exit (1);
    }
  close (dlsock);
  return url;

}

gint
downloadstart_cb (GtkWidget * widget, guint datum)
{
  struct sockaddr_in server;
  struct hostent *server_data;
  gchar str[100], sn[1000];


  downloadfilelen = 0;
  downloadactive = TRUE;
  if (!expedia)
    g_snprintf (str, sizeof (str), _("Connecting to %s"), WEBSERVER);
  if (expedia)
    {
      if (expedia_de)
	g_snprintf (str, sizeof (str), _("Connecting to %s"), WEBSERVER4);
      else
	g_snprintf (str, sizeof (str), _("Connecting to %s"), WEBSERVER2);
    }

  gtk_statusbar_pop (GTK_STATUSBAR (status), statusid);
  gtk_statusbar_push (GTK_STATUSBAR (status), statusid, str);
  while (gtk_events_pending ())
    gtk_main_iteration ();
/*  open socket to port80 */
  if ((dlsock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
      perror (_("can't open socket for port 80"));
      if (!expedia)
	g_snprintf (str, sizeof (str), _("Connecting to %s FAILED!"),
		    WEBSERVER);
      if (expedia)
	{
	  if (expedia_de)
	    g_snprintf (str, sizeof (str), _("Connecting to %s FAILED!"),
			WEBSERVER4);
	  else
	    g_snprintf (str, sizeof (str), _("Connecting to %s FAILED!"),
			WEBSERVER2);
	}

      gtk_statusbar_pop (GTK_STATUSBAR (status), statusid);
      gtk_statusbar_push (GTK_STATUSBAR (status), statusid, str);
      gtk_widget_destroy (downloadwindow);
      gtk_timeout_add (6000, (GtkFunction) dlstatusaway_cb, widget);
      return (FALSE);
    }

  server.sin_family = AF_INET;
/*  We retrieve the IP address of the server from its name: */
  if (haveproxy)
    g_strlcpy (sn, proxy, sizeof (sn));
  else
    {
      if (expedia)
	{
	  if (expedia_de)
	    g_strlcpy (sn, WEBSERVER4, sizeof (sn));
	  else
	    g_strlcpy (sn, WEBSERVER2, sizeof (sn));
	}

      if (!expedia)
	g_strlcpy (sn, WEBSERVER, sizeof (sn));
    }

  if (expedia==TRUE && haveproxy==FALSE)
    g_strlcpy (sn, actualhostname, sizeof (sn));

  if ((server_data = gethostbyname (sn)) == NULL)
    {
      perror (_("Can't resolve webserver address"));
      if (!expedia)
	g_snprintf (str, sizeof (str), _("Connecting to %s FAILED!"),
		    WEBSERVER);
      if (expedia)
	{
	  if (expedia_de)
	    g_snprintf (str, sizeof (str), _("Connecting to %s FAILED!"),
			WEBSERVER4);
	  else
	    g_snprintf (str, sizeof (str), _("Connecting to %s FAILED!"),
			WEBSERVER2);
	}

      gtk_statusbar_pop (GTK_STATUSBAR (status), statusid);
      gtk_statusbar_push (GTK_STATUSBAR (status), statusid, str);
      gtk_widget_destroy (downloadwindow);
      gtk_timeout_add (3000, (GtkFunction) dlstatusaway_cb, widget);
      return (FALSE);
    }
  memcpy (&server.sin_addr, server_data->h_addr, server_data->h_length);
  server.sin_port = htons (proxyport);
/*  We initiate the connection  */
  if (connect (dlsock, (struct sockaddr *) &server, sizeof server) < 0)
    {
      perror (_("unable to connect to Website"));
      if (!expedia)
	g_snprintf (str, sizeof (str), _("Connecting to %s FAILED!"),
		    WEBSERVER);
      if (expedia)
	{
	  if (expedia_de)
	    g_snprintf (str, sizeof (str), _("Connecting to %s FAILED!"),
			WEBSERVER4);
	  else
	    g_snprintf (str, sizeof (str), _("Connecting to %s FAILED!"),
			WEBSERVER2);
	}

      gtk_statusbar_pop (GTK_STATUSBAR (status), statusid);
      gtk_statusbar_push (GTK_STATUSBAR (status), statusid, str);
      gtk_widget_destroy (downloadwindow);
      gtk_timeout_add (3000, (GtkFunction) dlstatusaway_cb, widget);
      return (FALSE);
    }

  write (dlsock, writebuff, strlen (writebuff));
  dlbuff = g_new0 (gchar, 8192);
  dlpstart = NULL;
  dldiff = dlcount = 0;
  if (!expedia)
    g_snprintf (str, sizeof (str), _("Now connected to %s"), WEBSERVER);
  if (expedia)
    {
      if (expedia_de)
	g_snprintf (str, sizeof (str), _("Now connected to %s"), WEBSERVER4);
      else
	g_snprintf (str, sizeof (str), _("Now connected to %s"), WEBSERVER2);
    }

  gtk_statusbar_pop (GTK_STATUSBAR (status), statusid);
  gtk_statusbar_push (GTK_STATUSBAR (status), statusid, str);
  gtk_timeout_add (100, (GtkFunction) downloadslave_cb, widget);
  return TRUE;
}

gint
downloadslave_cb (GtkWidget * widget, guint datum)
{
  gchar tmpbuff[9000], str[100], *p;
  gint e, fd;
  gchar nn[] = "\r\n\r\n";
  gdouble f;
  G_CONST_RETURN gchar *s;

  if (!downloadwindowactive)
    return FALSE;


  FD_ZERO (&readmask);
  FD_SET (dlsock, &readmask);
  timeout.tv_sec = 0;
  timeout.tv_usec = 100000;
  if (select (FD_SETSIZE, &readmask, NULL, NULL, &timeout) < 0)
    {
      perror ("select() call");
    }

  if (FD_ISSET (dlsock, &readmask))
    {

      memset (tmpbuff, 0, 8192);
      if ((e = read (dlsock, tmpbuff, 8000)) < 0)
	perror (_("read from Webserver"));
      if (debug)
	g_print ("\nLoaded %d Bytes\n", e);
      if (e > 0)
	{
/*  in dlbuff we have all download data */
	  memcpy ((dlbuff + dlcount), tmpbuff, e);
/*  in dlcount we have the number of download bytes */
	  dlcount += e;
/* now we try to get the filelength and begin of the gif image data */
	  if (dlpstart == NULL)
	    {
/*  CONTENT-LENGTH string should hopefully be in the first 4kB */
	      memcpy (tmpbuff, dlbuff, 4096);
/*  We make of this a null terminated string */
	      tmpbuff[4096] = 0;
	      g_strup (tmpbuff);
	      p = strstr (tmpbuff, "CONTENT-LENGTH:");
	      if (p != NULL)
		{
		  sscanf (p, "%s %d", str, &downloadfilelen);
/*  now we look for 2 cr/lf which is the end of the header */
		  dlpstart = strstr (tmpbuff, nn);
		  dldiff = dlpstart - tmpbuff + 4;
/* 		  g_print ("\ncontent-length: %d", downloadfilelen); */
		}
	      else if (dlcount > 1000)
		{
/*  Seems there is no CONTENT-LENGTH field in expedia.com*/
		  dlpstart = strstr (tmpbuff, nn);
		  dldiff = dlpstart - tmpbuff + 4;
		  downloadfilelen = 200000;
/* 		  g_print ("\ncontent-length: %d", downloadfilelen); */
		}
	    }
/*  Now we have the length and begin of the gif image data */
	  if ((downloadfilelen != 0) && (dlpstart != NULL))
	    {
	      dlbuff = g_renew (gchar, dlbuff, dlcount + 8192);
	      f = (dlcount - dldiff) / (gdouble) downloadfilelen;
	      if (f > 1.0)
		f = 1.0;
	      gtk_progress_bar_update (GTK_PROGRESS_BAR (myprogress), f);
	      g_snprintf (str, sizeof (str), _("Downloaded %d kBytes"),
			  (dlcount - dldiff) / 1024);
	      gtk_statusbar_pop (GTK_STATUSBAR (status), statusid);
	      gtk_statusbar_push (GTK_STATUSBAR (status), statusid, str);
	      while (gtk_events_pending ())
		gtk_main_iteration ();
	    }

	}
      if ((e == 0) || ((downloadfilelen + dldiff) == dlcount))
	{

	  if (downloadfilelen == 0)
	    g_snprintf (str, sizeof (str), _("Download FAILED!"));
	  else
	    g_snprintf (str, sizeof (str), _("Download finished, got %dkB"),
			dlcount / 1024);
	  gtk_statusbar_pop (GTK_STATUSBAR (status), statusid);
	  gtk_statusbar_push (GTK_STATUSBAR (status), statusid, str);
	  close (dlsock);
	  if (downloadfilelen != 0)
	    {
	      s = gtk_entry_get_text (GTK_ENTRY (dltext4));
	      if (mapdir[strlen (mapdir) - 1] != '/')
		g_strlcat (mapdir, "/", sizeof (mapdir));

	      g_strlcpy (downloadfilename, mapdir, sizeof (downloadfilename));

	      g_strlcat (downloadfilename, s, sizeof (downloadfilename));
	      fd = open (downloadfilename, O_RDWR | O_TRUNC | O_CREAT, 0644);
	      if (fd < 1)
		{
		  perror (downloadfilename);
		  gtk_timeout_add (3000, (GtkFunction) dlstatusaway_cb,
				   widget);

		  return FALSE;
		}
	      write (fd, dlbuff + dldiff, dlcount - dldiff);
	      close (fd);
/* 	      g_free (maps); */
	      loadmapconfig ();
	      maps = g_renew (mapsstruct, maps, (nrmaps + 2));
	      g_strlcpy ((maps + nrmaps)->filename,
			 g_basename (downloadfilename), 200);
	      (maps + nrmaps)->lat = g_strtod (newmaplat, NULL);
	      (maps + nrmaps)->lon = g_strtod (newmaplongi, NULL);
	      (maps + nrmaps)->scale = strtol (newmapsc, NULL, 0);
	      nrmaps++;
	      havenasa = -1;
	      savemapconfig ();
	    }
	  downloadwindowactive = FALSE;
	  gtk_widget_set_sensitive (downloadbt, TRUE);
	  gtk_widget_destroy (downloadwindow);
	  gtk_timeout_add (3000, (GtkFunction) dlstatusaway_cb, widget);

	  return FALSE;
	}
    }
  else
    {

      return TRUE;
    }


  return TRUE;
}

gint
import2_cb (GtkWidget * widget, gpointer datum)
{
  G_CONST_RETURN gchar *s;

  s = gtk_entry_get_text (GTK_ENTRY (dltext1));
  checkinput ((gchar *) s);
  imports[0].lat = g_strtod (s, NULL);
  s = gtk_entry_get_text (GTK_ENTRY (dltext2));
  checkinput ((gchar *) s);
  imports[0].lon = g_strtod (s, NULL);
  s = gtk_entry_get_text (GTK_ENTRY (dltext5));
  imports[0].x = strtol (s, NULL, 0);
  s = gtk_entry_get_text (GTK_ENTRY (dltext6));
  imports[0].y = strtol (s, NULL, 0);

  gtk_widget_destroy (widget);
  import1_cb (NULL, 2);

  return TRUE;
}

gint
import3_cb (GtkWidget * widget, gpointer datum)
{
  G_CONST_RETURN gchar *s;
  gdouble tx, ty, scale, latmax, latmin, latcenter, longmax, longmin;
  gdouble longcenter;
  gdouble px, py;

  s = gtk_entry_get_text (GTK_ENTRY (dltext1));
  checkinput ((gchar *) s);
  imports[1].lat = g_strtod (s, NULL);
  s = gtk_entry_get_text (GTK_ENTRY (dltext2));
  checkinput ((gchar *) s);
  imports[1].lon = g_strtod (s, NULL);
  s = gtk_entry_get_text (GTK_ENTRY (dltext5));
  imports[1].x = strtol (s, NULL, 0);
  s = gtk_entry_get_text (GTK_ENTRY (dltext6));
  imports[1].y = strtol (s, NULL, 0);
  gtk_widget_destroy (widget);

/*  Calc coordinates and scale */

  tx =
    (2 * R * M_PI / 360) * cos (M_PI * imports[0].lat / 180.0) *
    (imports[0].lon - imports[1].lon);
  ty = (2 * R * M_PI / 360) * (imports[0].lat - imports[1].lat);
/*  ty is meter */
  px = abs (imports[0].x - imports[1].x);
  py = abs (imports[0].y - imports[1].y);
  if (px > py)
    scale = fabs (tx) * PIXELFACT / px;
  else
    scale = fabs (ty) * PIXELFACT / py;
  px = imports[0].x - imports[1].x;
  py = imports[0].y - imports[1].y;
  py = -py;

  latmin =
    imports[0].lat - (imports[0].lat - imports[1].lat) * (1024 -
							  imports[0].y) / py;
  latmax = latmin + (imports[0].lat - imports[1].lat) * 1024.0 / py;
  latcenter = (latmax + latmin) / 2.0;

  longmin =
    imports[0].lon - imports[0].x * (imports[0].lon - imports[1].lon) / px;

  longmax = longmin + 1280.0 * (imports[0].lon - imports[1].lon) / px;
  longcenter = (longmax + longmin) / 2.0;

  if (debug)
    g_print
      ("\nImport: scale: %g, latmitte: %g, latmin: %g, "
       "latmax: %g\n longmin: %g, longmax: %g, longmitte: %g\n",
       scale, latcenter, latmin, latmax, longmin, longmax, longcenter);

  if (strlen (importfilename) > 4)
    {
      maps = g_renew (mapsstruct, maps, (nrmaps + 2));
      g_strlcpy ((maps + nrmaps)->filename, importfilename, 200);
      (maps + nrmaps)->lat = latcenter;
      (maps + nrmaps)->lon = longcenter;
      (maps + nrmaps)->scale = scale;
      nrmaps++;
      havenasa = -1;

      savemapconfig ();
    }

  importactive = FALSE;
  g_strlcpy (oldfilename, "XXXAFHSGFAERGXXXXXX", sizeof (oldfilename));

  return TRUE;
}


gint
nimmfile (GtkWidget * widget, gpointer datum)
{
  G_CONST_RETURN gchar *buf;

  buf = gtk_file_selection_get_filename (datum);
  gtk_entry_set_text (GTK_ENTRY (dltext7), g_basename (buf));
  g_strlcpy (importfilename, g_basename (buf), sizeof (importfilename));

  gtk_widget_destroy (datum);
  loadmap ((char *) g_basename (buf));
  return (TRUE);
}

gint
importfb_cb (GtkWidget * widget, guint datum)
{
  GtkWidget *fdialog;
  gchar buf[1000];
  fdialog = gtk_file_selection_new (_("Select a map file"));
  gtk_window_set_modal (GTK_WINDOW (fdialog), TRUE);
  gtk_window_set_transient_for (GTK_WINDOW (fdialog),
				GTK_WINDOW (mainwindow));

  gtk_signal_connect (GTK_OBJECT
		      (GTK_FILE_SELECTION (fdialog)->ok_button),
		      "clicked", GTK_SIGNAL_FUNC (nimmfile),
		      GTK_OBJECT (fdialog));
  gtk_signal_connect_object (GTK_OBJECT
			     (GTK_FILE_SELECTION (fdialog)->
			      cancel_button), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT (fdialog));


  g_strlcpy (buf, homedir, sizeof (buf));

  gtk_file_selection_complete (GTK_FILE_SELECTION (fdialog), buf);
  gtk_widget_show (fdialog);
  xoff = 0;
  yoff = 0;
  zoom = 1;
  iszoomed = FALSE;

  return TRUE;
}


gint
importshift_cb (GtkWidget * widget, guint datum)
{
  switch (datum)
    {
    case 1:
      yoff -= SCREEN_Y_2;
      break;
    case 4:
      yoff += SCREEN_Y_2;
      break;
    case 2:
      xoff -= SCREEN_X_2;
      break;
    case 3:
      xoff += SCREEN_X_2;
      break;
    }
  iszoomed = FALSE;
  expose_cb (NULL, 0);
  expose_mini_cb (NULL, 0);

  return TRUE;
}


gint
import1_cb (GtkWidget * widget, guint datum)
{
  GtkWidget *mainbox, *window;
  GtkWidget *knopf2, *knopf, *knopf3, *knopf4, *knopf6;
  GtkWidget *table, *knopf9, *knopf10, *knopf11, *s1, *s2, *s3, *s4;
  GtkWidget *s5, *s6;
  gchar buff[1300];
  GtkWidget *text;
  GtkWidget *hbox;
  gchar *thetext1 = _("How to calibrate your own maps? "
		      "First, the map file\nmust be copied into the");
  gchar *thetext1a = _("\ndirectory as .gif, .jpg or .png file "
		       "and must have\nthe size 1280x1024. The file names must be\n"
		       "map_* for street maps or top_* for topographical maps!\n"
		       "Load the file, select coordinates "
		       "from waypoint list or\ntype them in. "
		       "Then click on the accept button.");
  gchar *thetext2 =
    _("Now do the same for your second point and click on the\n"
      "finish button. The map can be used now.");

  window = gtk_dialog_new ();
  if (datum == 1)
    gtk_window_set_title (GTK_WINDOW (window), _("Import Assistant. Step 1"));
  else
    gtk_window_set_title (GTK_WINDOW (window), _("Import Assistant. Step 2"));

  gtk_container_set_border_width (GTK_CONTAINER (window), 5);
  mainbox = gtk_vbox_new (TRUE, 2);
  if (datum == 1)
    knopf = gtk_button_new_with_label (_("Accept first point"));
  else
    knopf = gtk_button_new_with_label (_("Finish"));
  if (datum == 1)
    gtk_signal_connect_object (GTK_OBJECT (knopf), "clicked",
			       GTK_SIGNAL_FUNC (import2_cb),
			       GTK_OBJECT (window));
  else
    {
      gtk_signal_connect_object (GTK_OBJECT (knopf), "clicked",
				 GTK_SIGNAL_FUNC (import3_cb),
				 GTK_OBJECT (window));
    }

  knopf2 = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
  gtk_signal_connect_object (GTK_OBJECT (knopf2), "clicked",
			     GTK_SIGNAL_FUNC
			     (importaway_cb), GTK_OBJECT (window));
  gtk_signal_connect_object (GTK_OBJECT (window),
			     "delete_event",
			     GTK_SIGNAL_FUNC
			     (importaway_cb), GTK_OBJECT (window));

  s1 = gtk_button_new_with_label (_("Go up"));
  gtk_signal_connect (GTK_OBJECT (s1), "clicked",
		      GTK_SIGNAL_FUNC (importshift_cb), (gpointer) 1);
  s2 = gtk_button_new_with_label (_("Go left"));
  gtk_signal_connect (GTK_OBJECT (s2), "clicked",
		      GTK_SIGNAL_FUNC (importshift_cb), (gpointer) 2);
  s3 = gtk_button_new_with_label (_("Go right"));
  gtk_signal_connect (GTK_OBJECT (s3), "clicked",
		      GTK_SIGNAL_FUNC (importshift_cb), (gpointer) 3);
  s4 = gtk_button_new_with_label (_("Go down"));
  gtk_signal_connect (GTK_OBJECT (s4), "clicked",
		      GTK_SIGNAL_FUNC (importshift_cb), (gpointer) 4);
  s5 = gtk_button_new_with_label (_("Zoom in"));
  gtk_signal_connect (GTK_OBJECT (s5), "clicked",
		      GTK_SIGNAL_FUNC (zoom_cb), (gpointer) 1);
  s6 = gtk_button_new_with_label (_("Zoom out"));
  gtk_signal_connect (GTK_OBJECT (s6), "clicked",
		      GTK_SIGNAL_FUNC (zoom_cb), (gpointer) 2);

  gtk_box_pack_start (GTK_BOX
		      (GTK_DIALOG (window)->
		       action_area), knopf, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX
		      (GTK_DIALOG (window)->
		       action_area), knopf2, TRUE, TRUE, 2);
  GTK_WIDGET_SET_FLAGS (knopf, GTK_CAN_DEFAULT);
  GTK_WIDGET_SET_FLAGS (knopf2, GTK_CAN_DEFAULT);
  table = gtk_table_new (6, 4, TRUE);
  gtk_box_pack_start (GTK_BOX
		      (GTK_DIALOG (window)->vbox), table, TRUE, TRUE, 2);
  knopf3 = gtk_label_new (_("Latitude"));
  gtk_table_attach_defaults (GTK_TABLE (table), knopf3, 0, 1, 0, 1);
  knopf4 = gtk_label_new (_("Longitude"));
  gtk_table_attach_defaults (GTK_TABLE (table), knopf4, 0, 1, 1, 2);
  knopf9 = gtk_label_new (_("Screen X"));
  gtk_table_attach_defaults (GTK_TABLE (table), knopf9, 2, 3, 0, 1);
  knopf10 = gtk_label_new (_("Screen Y"));
  gtk_table_attach_defaults (GTK_TABLE (table), knopf10, 2, 3, 1, 2);
  knopf6 = gtk_button_new_with_label (_("Browse waypoint"));
  gtk_signal_connect (GTK_OBJECT (knopf6), "clicked",
		      GTK_SIGNAL_FUNC (sel_target_cb), (gpointer) 1);

  gtk_table_attach_defaults (GTK_TABLE (table), knopf6, 0, 1, 2, 3);
  dltext1 = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), dltext1, 1, 2, 0, 1);
  g_snprintf (buff, sizeof (buff), "%.5f", current_lat);
  if (minsecmode)
    decimaltomin (buff, 1);
  gtk_entry_set_text (GTK_ENTRY (dltext1), buff);
  dltext2 = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), dltext2, 1, 2, 1, 2);
  g_snprintf (buff, sizeof (buff), "%.5f", current_long);
  if (minsecmode)
    decimaltomin (buff, 0);
  gtk_entry_set_text (GTK_ENTRY (dltext2), buff);

  dltext5 = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), dltext5, 3, 4, 0, 1);

  dltext6 = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), dltext6, 3, 4, 1, 2);

  dltext4 = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), dltext4, 1, 2, 2, 3);
  dltext7 = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), dltext7, 3, 4, 2, 3);

  if (datum == 1)
    {
      knopf11 = gtk_button_new_with_label (_("Browse filename"));
      gtk_signal_connect_object (GTK_OBJECT (knopf11), "clicked",
				 GTK_SIGNAL_FUNC (importfb_cb), 0);
      gtk_table_attach_defaults (GTK_TABLE (table), knopf11, 2, 3, 2, 3);
    }
  else
    gtk_entry_set_text (GTK_ENTRY (dltext7), importfilename);


  gtk_entry_set_editable (GTK_ENTRY (dltext7), FALSE);
  gtk_entry_set_editable (GTK_ENTRY (dltext4), FALSE);
  gtk_entry_set_editable (GTK_ENTRY (dltext5), FALSE);
  gtk_entry_set_editable (GTK_ENTRY (dltext6), FALSE);

  text = gtk_label_new ("");


  if (datum == 1)
    g_snprintf (buff, sizeof (buff),
		"<span font_family=\"Arial\" size=\"10000\">%s <span color=\"red\"> %s</span> %s</span>",
		thetext1, mapdir, thetext1a);
  else
    g_snprintf (buff, sizeof (buff),
		"<span font_family=\"Arial\" size=\"10000\">%s</span>",
		thetext2);

  gtk_label_set_text (GTK_LABEL (text), buff);
  gtk_label_set_use_markup (GTK_LABEL (text), TRUE);

  hbox = gtk_hbox_new (FALSE, 3);
  gtk_box_pack_start (GTK_BOX (hbox), text, TRUE, TRUE, 0);
/*   gtk_box_pack_start (GTK_BOX (hbox), scrollbar, FALSE, FALSE, 0); */

  gtk_table_attach_defaults (GTK_TABLE (table), hbox, 2, 4, 3, 6);
  gtk_table_attach_defaults (GTK_TABLE (table), s1, 0, 1, 3, 4);
  gtk_table_attach_defaults (GTK_TABLE (table), s5, 1, 2, 3, 4);
  gtk_table_attach_defaults (GTK_TABLE (table), s2, 0, 1, 4, 5);
  gtk_table_attach_defaults (GTK_TABLE (table), s3, 1, 2, 4, 5);
  gtk_table_attach_defaults (GTK_TABLE (table), s4, 0, 1, 5, 6);
  gtk_table_attach_defaults (GTK_TABLE (table), s6, 1, 2, 5, 6);
  gtk_table_set_row_spacings (GTK_TABLE (table), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table), 3);
/*    gtk_label_set_justify (GTK_LABEL (knopf6), GTK_JUSTIFY_RIGHT); */
/*    gtk_label_set_justify (GTK_LABEL (knopf3), GTK_JUSTIFY_RIGHT); */
/*    gtk_label_set_justify (GTK_LABEL (knopf4), GTK_JUSTIFY_RIGHT); */
/*    gtk_label_set_justify (GTK_LABEL (knopf6), GTK_JUSTIFY_RIGHT); */
  gtk_window_set_default (GTK_WINDOW (window), knopf);
  gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (mainwindow));
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  gtk_widget_show_all (window);
  importactive = TRUE;

  return TRUE;
}

gint
setup2_cb (GtkWidget * widget, guint datum)
{
  gtk_widget_destroy (GTK_WIDGET (datum));
  return TRUE;
}

gint
wpfileselect_cb (GtkWidget * widget, guint datum)
{
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (setupfn[datum])))
    if ((strcmp (activewpfile, (names + datum)->n)))
      {
	g_strlcpy (activewpfile, (names + datum)->n, sizeof (activewpfile));
	if (debug)
	  g_print ("\nactivewpfile: %s", activewpfile);
	loadwaypoints ();
	iszoomed = FALSE;
      }
  needtosave = TRUE;
  return TRUE;
}


gint
setup_cb (GtkWidget * widget, guint datum)
{
  GtkWidget *notebook, *vbox, *cancel;
  GtkWidget *window = NULL;
  gint i;

  gtk_widget_set_sensitive (setupbt, FALSE);

  mainsetup ();
  infos ();
  trip ();

  window = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (window), _("GpsDrive Control"));
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (mainwindow));

  cancel = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
  GTK_WIDGET_SET_FLAGS (cancel, GTK_CAN_DEFAULT);
  gtk_window_set_default (GTK_WINDOW (window), cancel);
/*   GTK_WIDGET_SET_FLAGS (cancel, GTK_HAS_FOCUS); */


  gtk_signal_connect ((GTK_OBJECT (window)), "delete_event",
		      GTK_SIGNAL_FUNC (removesetutc), 0);
  gtk_signal_connect_object ((GTK_OBJECT (window)), "delete_event",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT (window));
  gtk_signal_connect ((GTK_OBJECT (window)), "destroy",
		      GTK_SIGNAL_FUNC (removesetutc), 0);
  gtk_signal_connect_object ((GTK_OBJECT (window)), "destroy",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT (window));



  gtk_signal_connect ((GTK_OBJECT (cancel)), "clicked",
		      GTK_SIGNAL_FUNC (removesetutc), 0);

  gtk_signal_connect_object ((GTK_OBJECT (cancel)), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT (window));

  gtk_container_border_width (GTK_CONTAINER (window), 2 * PADDING);
  vbox = gtk_vbox_new (FALSE, 2 * PADDING);
/*     table = gtk_table_new(2,1,TRUE); */
/*   gtk_container_add (GTK_CONTAINER (window), vbox); */
  gtk_box_pack_start (GTK_BOX
		      (GTK_DIALOG (window)->vbox), vbox, TRUE, TRUE, 2);

  /* Create a new notebook, place the position of the tabs */
  settingsnotebook = notebook = gtk_notebook_new ();
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook), TRUE);
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
  gtk_notebook_set_show_border (GTK_NOTEBOOK (notebook), TRUE);
  gtk_notebook_popup_enable (GTK_NOTEBOOK (notebook));
/*     gtk_table_attach_defaults(GTK_TABLE(table), notebook, 0,1,0,1); */
  gtk_box_pack_start (GTK_BOX (vbox), notebook, TRUE, TRUE, 2 * PADDING);
/*   gtk_box_pack_start (GTK_BOX (vbox), cancel, FALSE, FALSE, 2 * PADDING); */
  gtk_box_pack_start (GTK_BOX
		      (GTK_DIALOG (window)->action_area),
		      cancel, TRUE, TRUE, 2);

  gtk_widget_show (notebook);
/*   g_print("\nmod_setupcounter: %d",mod_setupcounter); */
//KCFX
  for (i = 0; i <= mod_setupcounter; i++)
    {
      if (i > 4)
	setupfunction[i] ();

      gtk_widget_show_all (setupentry[i]);
      gtk_notebook_append_page (GTK_NOTEBOOK (notebook), setupentry[i],
				setupentrylabel[i]);
    }
  gtk_notebook_set_page (GTK_NOTEBOOK (notebook), lastnotebook);
  gtk_widget_show_all (window);

  return TRUE;
}



/*  switching between kilometers and miles */
gint
miles_cb (GtkWidget * widget, guint datum)
{
  gchar s1[80];
/*      1=miles, 2=metric, 3=nautic */
  switch (datum)
    {
    case 1:
      milesconv = KM2MILES;
      milesflag = TRUE;
      nauticflag = FALSE;
      metricflag = FALSE;
      break;
    case 2:
      milesconv = 1.0;
      milesflag = FALSE;
      nauticflag = FALSE;
      metricflag = TRUE;
      break;
    case 3:
      milesconv = KM2NAUTIC;
      milesflag = FALSE;
      nauticflag = TRUE;
      metricflag = FALSE;
      break;

    }
  needtosave = TRUE;
  if (pdamode)
    {
      if (milesflag)
	g_snprintf (s1, sizeof (s1), "[%s]", _("mi/h"));
      else if (nauticflag)
	g_snprintf (s1, sizeof (s1), "[%s]", _("knots"));
      else
	g_snprintf (s1, sizeof (s1), "[%s]", _("km/h"));
    }
  else
    {
      if (milesflag)
	g_snprintf (s1, sizeof (s1), "%s [%s]", _("Speed"), _("mi/h"));
      else if (nauticflag)
	g_snprintf (s1, sizeof (s1), "%s [%s]", _("Speed"), _("knots"));
      else
	g_snprintf (s1, sizeof (s1), "%s [%s]", _("Speed"), _("km/h"));
    }

  gtk_frame_set_label (GTK_FRAME (frame_speed), s1);
  return TRUE;
}

/*  switching nightmode */
gint
night_cb (GtkWidget * widget, guint datum)
{

  switch (datum)
    {
    case 0:
      nightmode = 0;
      break;
    case 1:
      nightmode = 1;
      break;
    case 2:
      nightmode = 2;
      break;

    }

  needtosave = TRUE;
  return TRUE;
}



gint
defaultserver_cb (GtkWidget * widget, guint datum)
{

  expedia_de = FALSE;
  switch (datum)
    {
    case 1:
      expedia_de = TRUE;
      defaultserver = 0;
      break;
    case 2:
      defaultserver = 1;
      break;
    }
  needtosave = TRUE;
  return TRUE;
}


/*  switching shadow on/off */
gint
shadow_cb (GtkWidget * widget, guint datum)
{
  shadow = !shadow;
  needtosave = TRUE;
  return TRUE;
}

/*  switching shadow on/off */
gint
etch_cb (GtkWidget * widget, guint datum)
{
  int stype;
  etch = !etch;
  if (etch)
    stype = GTK_SHADOW_IN;
  else
    stype = GTK_SHADOW_ETCHED_OUT;
  gtk_frame_set_shadow_type (GTK_FRAME (frame_bearing), stype);
  gtk_frame_set_shadow_type (GTK_FRAME (frame_target), stype);
  gtk_frame_set_shadow_type (GTK_FRAME (frame_speed), stype);
  gtk_frame_set_shadow_type (GTK_FRAME (frame_altitude), stype);
  gtk_frame_set_shadow_type (GTK_FRAME (frame_wp), stype);
  gtk_frame_set_shadow_type (GTK_FRAME (frame_sats), stype);
  gtk_frame_set_shadow_type (GTK_FRAME (lf1), stype);
  gtk_frame_set_shadow_type (GTK_FRAME (lf2), stype);
  gtk_frame_set_shadow_type (GTK_FRAME (lf3), stype);
  gtk_frame_set_shadow_type (GTK_FRAME (lf4), stype);
  gtk_frame_set_shadow_type (GTK_FRAME (lf5), stype);
  gtk_frame_set_shadow_type (GTK_FRAME (lf6), stype);
  gtk_frame_set_shadow_type (GTK_FRAME (lf7), stype);
  gtk_frame_set_shadow_type (GTK_FRAME (lf8), stype);
  gtk_frame_set_shadow_type (GTK_FRAME (frame_toogles), stype);
  gtk_frame_set_shadow_type (GTK_FRAME (frame_maptype), stype);

  if (havebattery)
    gtk_frame_set_shadow_type (GTK_FRAME (fbat), stype);
  if (havetemperature)
    gtk_frame_set_shadow_type (GTK_FRAME (ftem), stype);

  needtosave = TRUE;
  return TRUE;
}

/*  switching shadow on/off */
gint
drawgrid_cb (GtkWidget * widget, guint datum)
{
  drawgrid = !drawgrid;
  needtosave = TRUE;
  return TRUE;
}

/*  switching slow cpu on/off */
gint
slowcpu_cb (GtkWidget * widget, guint datum)
{
  G_CONST_RETURN gchar *sc;
  gchar dummy[100];
  sc = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (slowcpubt)->entry));
  sscanf (sc, "%d%s", &cpuload, dummy);
  if (cpuload == 0)
    cpuload = 40;


  needtosave = TRUE;
  return TRUE;
}

/*  switching decimal or degree, minutes, seconds mode */
gint
minsec_cb (GtkWidget * widget, guint datum)
{
  minsecmode = !minsecmode;
  needtosave = TRUE;
  return TRUE;
}

/*  switching sat level/sat position display */
gint
satpos_cb (GtkWidget * widget, guint datum)
{
  satposmode = !satposmode;
  needtosave = TRUE;
  expose_sats_cb (NULL, 0);
  return TRUE;
}

/*  switching simfollow on/off */
gint
simfollow_cb (GtkWidget * widget, guint datum)
{
  simfollow = !simfollow;
  needtosave = TRUE;
  return TRUE;
}

/*  switching testgarmin on/off */
gint
testgarmin_cb (GtkWidget * widget, guint datum)
{
  testgarmin = !testgarmin;
  needtosave = TRUE;
  return TRUE;
}

/*  should I use DGPS-IP? */
gint
usedgps_cb (GtkWidget * widget, guint datum)
{
  usedgps = !usedgps;
  needtosave = TRUE;
  return TRUE;
}

gint
earthmate_cb (GtkWidget * widget, guint datum)
{
  earthmate = !earthmate;
  needtosave = TRUE;
  return TRUE;
}


gint
serialdev_cb (GtkWidget * widget, guint datum)
{
  G_CONST_RETURN gchar *s;

  s = g_strstrip ((char *) gtk_entry_get_text (GTK_ENTRY (serialbt)));
  g_strlcpy (serialdev, s, sizeof (serialdev));
  needtosave = TRUE;
  return TRUE;
}


gint
mapdir_cb (GtkWidget * widget, guint datum)
{
  G_CONST_RETURN gchar *s;

  s = g_strstrip ((char *) gtk_entry_get_text (GTK_ENTRY (mapdirbt)));
  g_strlcpy (mapdir, s, sizeof (mapdir));
  needtosave = TRUE;
  needreloadmapconfig = TRUE;
  gtk_timeout_add (2000, (GtkFunction) loadmapconfig, 0);
  return TRUE;
}


gint
maptoggle_cb (GtkWidget * widget, guint datum)
{
  displaymap_map = !displaymap_map;
  if (displaymap_map)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (maptogglebt), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (maptogglebt), FALSE);
  needtosave = TRUE;
  return TRUE;
}

gint
dotripmeter (GtkWidget * widget, guint datum)
{
  gdouble d;

  d = calcdist (trip_long, trip_lat);
  trip_long = current_long;
  trip_lat = current_lat;
  if (!((d >= 0.0) && (d < (1000.0 * TRIPMETERTIMEOUT / 3600.0))))
    {
      fprintf (stderr,
	       _("\ndistance jump is more then 1000km/h speed, ignoring\n"));
      return TRUE;
    }
/* we want always have metric system stored */
  d /= milesconv;
  tripodometer += d;
  if (groundspeed * milesconv > tripmaxspeed)
    tripmaxspeed = groundspeed / milesconv;
  tripavspeedcount++;
  tripavspeed += groundspeed / milesconv;
  return TRUE;
}

gint
bestmap_cb (GtkWidget * widget, guint datum)
{
  if (datum == 1)
    scaleprefered = !scaleprefered;
  if (!scaleprefered)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bestmapbt), TRUE);
      gtk_widget_set_sensitive (scalerrbt, FALSE);
      gtk_widget_set_sensitive (scalerlbt, FALSE);
      if (scaler)
	gtk_widget_set_sensitive (scaler, FALSE);
    }
  else
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bestmapbt), FALSE);
      gtk_widget_set_sensitive (scalerrbt, TRUE);
      gtk_widget_set_sensitive (scalerlbt, TRUE);
      if (scaler)
	gtk_widget_set_sensitive (scaler, TRUE);
    }
  needtosave = TRUE;
  return TRUE;
}

gint
savetrack_cb (GtkWidget * widget, guint datum)
{
  savetrack = !savetrack;
  if (savetrack)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (savetrackbt), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (savetrackbt), FALSE);
  needtosave = TRUE;
  return TRUE;
}

gint
topotoggle_cb (GtkWidget * widget, guint datum)
{
  displaymap_top = !displaymap_top;
  if (displaymap_top)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (topotogglebt), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (topotogglebt), FALSE);
  needtosave = TRUE;
  return TRUE;
}


gint
mute_cb (GtkWidget * widget, guint datum)
{
  muteflag = !muteflag;
  if (muteflag)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mutebt), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mutebt), FALSE);
  needtosave = TRUE;
  return TRUE;
}

gint
sql_cb (GtkWidget * widget, guint datum)
{
  sqlflag = !sqlflag;
  if (sqlflag)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sqlbt), TRUE);
      getsqltypelist ();
      getsqldata ();
    }
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sqlbt), FALSE);
  needtosave = TRUE;
  loadwaypoints ();
  return TRUE;
}

gint
track_cb (GtkWidget * widget, guint datum)
{
  trackflag = !trackflag;
  if (trackflag)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (trackbt), TRUE);
      rebuildtracklist ();
    }
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (trackbt), FALSE);
  needtosave = TRUE;
  return TRUE;
}

gint
wp_cb (GtkWidget * widget, guint datum)
{
  wpflag = !wpflag;
  if (wpflag)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wpbt), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wpbt), FALSE);
  needtosave = TRUE;
  return TRUE;
}

gint
pos_cb (GtkWidget * widget, guint datum)
{
  posmode = !posmode;
  if (posmode)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (posbt), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (posbt), FALSE);
  posmode_x = current_long;
  posmode_y = current_lat;

  return TRUE;
}

/*  if a waypoint is selected set the target_* variables */
gint
setwp_cb (GtkWidget * widget, guint datum)
{
  gchar b[100], str[200], buf[1000], buf2[1000];
  gchar *p, *tn;
  p = b;

  deleteline = datum;
  if (dontsetwp)
    return TRUE;

  gtk_clist_get_text (GTK_CLIST (mylist), datum, 0, &p);
  if (createroute)
    {
/*        g_print ("\nroute: %s", p); */
      thisrouteline = atol (p) - 1;
      insertroutepoints ();
      return TRUE;
    }
  thisline = atol (p);
/*    g_print ("%d\n", thisline); */
  gtk_clist_get_text (GTK_CLIST (mylist), datum, 1, &p);
  g_strlcpy (targetname, p, sizeof (targetname));


  g_snprintf (str, sizeof (str), "%s: %s", _("To"), targetname);
  tn = g_strdelimit (str, "_", ' ');
  gtk_frame_set_label (GTK_FRAME (destframe), tn);
  gtk_clist_get_text (GTK_CLIST (mylist), datum, 2, &p);
  checkinput (p);
  target_lat = atof (p);
  gtk_clist_get_text (GTK_CLIST (mylist), datum, 3, &p);
  checkinput (p);
  target_long = atof (p);
/*    gtk_timeout_add (5000, (GtkFunction) sel_targetweg_cb, widget); */
  g_timer_stop (disttimer);
  g_timer_start (disttimer);
  olddist = dist;
/*   posmode = FALSE; */
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (posbt), FALSE);

  tn = g_strdelimit (targetname, "_", ' ');
  g_strlcpy (buf2, "", sizeof (buf2));
  if (tn[0] == '*')
    {
      g_strlcpy (buf2, "das mobile Ziel ", sizeof (buf2));
      g_strlcat (buf2, (tn + 1), sizeof (buf2));
    }
  else
    g_strlcat (buf2, tn, sizeof (buf2));

  switch (voicelang)
    {
    case english:
      g_snprintf (buf, sizeof (buf), "New target is %s", buf2);
      break;
    case spanish:
      g_snprintf (buf, sizeof (buf), "Destinación definida: %s", buf2);
      break;
    case german:
      g_snprintf (buf, sizeof (buf), "Neues Ziel ist %s", buf2);
    }
  speech_out_speek (buf);
  saytarget = TRUE;
  routenearest = 9999999;

  return TRUE;
}

gint
accepttext (GtkWidget * widget, gpointer data)
{
  GtkTextIter start, end;
  gchar *p;
  GtkWidget *wi;

  gtk_text_buffer_get_bounds (getmessagebuffer, &start, &end);

  gtk_text_buffer_apply_tag_by_name (getmessagebuffer, "word_wrap", &start,
				     &end);

  p = gtk_text_buffer_get_text (getmessagebuffer, &start, &end, FALSE);

  strncpy (messagesendtext, p, 300);
  messagesendtext[301] = 0;
  if (debug)
    fprintf (stderr, "\nmessage:\n%s\n", messagesendtext);
  gtk_widget_destroy (widget);
  wi = gtk_item_factory_get_item (item_factory, N_("/Misc. Menu/Messages"));
  statuslock = TRUE;
  gtk_statusbar_push (GTK_STATUSBAR (status), statusid,
		      _("Sending message to friends server..."));
  gtk_widget_set_sensitive (wi, FALSE);
  return TRUE;
}

gint
textstatus (GtkWidget * widget, gpointer * datum)
{
  gint i;
  GtkTextIter start, end;
  gchar str[20];

  gtk_text_buffer_get_bounds (getmessagebuffer, &start, &end);
  gtk_text_buffer_apply_tag_by_name (getmessagebuffer, "word_wrap", &start,
				     &end);
  i = gtk_text_iter_get_offset (&end);
  if (i >= 300)
    {
      gdk_beep ();
/* gtk_text_buffer_delete (getmessagebuffer, &end-1, &end); */
    }
  g_snprintf (str, sizeof (str), "%d/300", i);

  gtk_statusbar_pop (GTK_STATUSBAR (messagestatusbar), messagestatusbarid);
  gtk_statusbar_push (GTK_STATUSBAR (messagestatusbar), messagestatusbarid,
		      str);
  return TRUE;
}

gint
setmessage_cb (GtkWidget * widget, guint datum)
{
  gchar b[100];
  gchar *p;
  int i;
  gchar titlestr[60];
  static GtkWidget *window = NULL;
  GtkWidget *ok, *cancel;
  GtkWidget *vpaned;
  GtkWidget *view1;
  GtkWidget *sw, *hbox, *vbox;
  GtkTextIter iter;
  gchar pre[180];
  time_t t;
  struct tm *ts;
  GtkTooltips *tooltips;

  p = b;

  gtk_clist_get_text (GTK_CLIST (mylist), datum, 0, &p);
  g_strlcpy (messagename, p, sizeof (messagename));
  for (i = 0; (size_t) i < strlen (messagename); i++)
    if (messagename[i] == ' ')
      messagename[i] = '_';

  gtk_widget_destroy (GTK_WIDGET (messagewindow));


/* create window to enter text */

  window = gtk_dialog_new ();
  gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (mainwindow));

  cancel = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
  gtk_signal_connect_object ((GTK_OBJECT (window)), "delete_event",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT (window));
  gtk_signal_connect_object ((GTK_OBJECT (window)), "destroy",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT (window));

  ok = gtk_button_new_from_stock (GTK_STOCK_APPLY);
  gtk_window_set_default_size (GTK_WINDOW (window), 320, 240);
  g_snprintf (titlestr, sizeof (titlestr), "%s %s", _("Message for:"),
	      messagename);
  gtk_window_set_title (GTK_WINDOW (window), titlestr);
  gtk_container_set_border_width (GTK_CONTAINER (window), 0);
  vpaned = gtk_vpaned_new ();
  gtk_container_set_border_width (GTK_CONTAINER (vpaned), 5);

  vbox = gtk_vbox_new (FALSE, 3);
  hbox = gtk_hbutton_box_new ();

  messagestatusbar = gtk_statusbar_new ();
  messagestatusbarid =
    gtk_statusbar_get_context_id (GTK_STATUSBAR (messagestatusbar),
				  "message");

  gtk_box_pack_start (GTK_BOX
		      (GTK_DIALOG (window)->vbox), vbox, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), vpaned, TRUE, TRUE, 3);
  gtk_box_pack_start (GTK_BOX (vbox), messagestatusbar, FALSE, FALSE, 3);
/*   gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3); */
  gtk_box_pack_start (GTK_BOX
		      (GTK_DIALOG (window)->action_area),
		      hbox, TRUE, TRUE, 2);

  gtk_box_pack_start (GTK_BOX (hbox), ok, TRUE, TRUE, 3);
  gtk_box_pack_start (GTK_BOX (hbox), cancel, TRUE, TRUE, 3);
  view1 = gtk_text_view_new ();

  getmessagebuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view1));
  g_signal_connect (GTK_TEXT_BUFFER (getmessagebuffer),
		    "changed", G_CALLBACK (textstatus), status);

  gtk_text_buffer_get_iter_at_offset (getmessagebuffer, &iter, 0);

  gtk_text_buffer_create_tag (getmessagebuffer, "word_wrap",
			      "wrap_mode", GTK_WRAP_WORD, NULL);
  gtk_text_buffer_insert_with_tags_by_name (getmessagebuffer, &iter,
					    "", -1, "word_wrap", NULL);

  time (&t);
  ts = localtime (&t);
  g_snprintf (pre, sizeof (pre), _("Date: %s"), asctime (ts));
  gtk_text_buffer_insert (getmessagebuffer, &iter, pre, -1);

  gtk_signal_connect_object ((GTK_OBJECT (ok)), "clicked",
			     GTK_SIGNAL_FUNC (accepttext),
			     GTK_OBJECT (window));

  gtk_signal_connect_object ((GTK_OBJECT (cancel)), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT (window));
  tooltips = gtk_tooltips_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), ok,
			_
			("Sends your text to to selected computer using the friends server"),
			NULL);

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_paned_add1 (GTK_PANED (vpaned), sw);

  gtk_container_add (GTK_CONTAINER (sw), view1);

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_show_all (window);


  return TRUE;
}


gint
delwp_cb (GtkWidget * widget, guint datum)
{
  gint i, j;
  gchar *p;

  i = deleteline;
  if (debug)
    g_print ("\nremove line %d", i);
  gtk_clist_get_text (GTK_CLIST (mylist), i, 0, &p);
  j = atol (p) - 1;
  gtk_clist_remove (GTK_CLIST (mylist), i);
  if (debug)
    g_print ("\nremove entry %d", j);

  deletesqldata ((wayp + j)->sqlnr);
  for (i = j; i < (maxwp - 1); i++)
    *(wayp + i) = *(wayp + i + 1);
  maxwp--;
  savewaypoints ();
  gtk_clist_get_text (GTK_CLIST (mylist), deleteline, 0, &p);
  thisline = atol (p);

  return TRUE;
}

gint
jumpwp_cb (GtkWidget * widget, guint datum)
{
  gint i;
  gchar *p;

  i = deleteline;
  gtk_clist_get_text (GTK_CLIST (mylist), i, 2, &p);
  current_lat = atof (p);
  gtk_clist_get_text (GTK_CLIST (mylist), i, 3, &p);
  current_long = atof (p);

  if ((!posmode) && (!simmode))
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (posbt), TRUE);
  getsqldata ();
  reinsertwp_cb (NULL, 0);
  sel_targetweg_cb (NULL, 0);
  return TRUE;
}


/*  set reference point for map calibration */
gint
setrefpoint_cb (GtkWidget * widget, guint datum)
{
  gchar b[100];
  gchar *p;
  p = b;
  gtk_clist_get_text (GTK_CLIST (mylist), datum, 1, &p);
  gtk_entry_set_text (GTK_ENTRY (dltext4), p);
  gtk_clist_get_text (GTK_CLIST (mylist), datum, 2, &p);
  gtk_entry_set_text (GTK_ENTRY (dltext1), p);

  gtk_clist_get_text (GTK_CLIST (mylist), datum, 3, &p);
  gtk_entry_set_text (GTK_ENTRY (dltext2), p);

  return TRUE;
}



gint
scaler_cb (GtkAdjustment * adj, gdouble * datum)
{
  gchar s2[100];
  scalewanted = nlist[(gint) rint (adj->value)];
  g_snprintf (s2, sizeof (s2), "1:%d", scalewanted);
  gtk_label_set_text (GTK_LABEL (l8), s2);
  if (debug)
    g_print ("\nScaler: %d", scalewanted);
  needtosave = TRUE;

  return TRUE;
}

/*  Let user select a destination from the waypoint list */

gint
key_cb (GtkWidget * widget, GdkEventKey * event)
{
  gdouble lat, lon;
  gint x, y;
  GdkModifierType state;

/*    g_print ("\nevent:%x key:%c\n",event->keyval,event->keyval); */
  if ((toupper (event->keyval)) == 'X')
    {
      wplat = current_lat;
      wplon = current_long;
      addwaypoint_cb (NULL, NULL);
    }

// xxxxxxxxxxxxxxxxxxxxxxxxxxx

  if ((toupper (event->keyval)) == 'Y')
    {

      gdk_window_get_pointer (drawing_area->window, &x, &y, &state);

      calcxytopos (x, y, &lat, &lon, zoom);


/*  Add mouse position as waypoint */
      wplat = lat;
      wplon = lon;
      addwaypoint_cb (NULL, 0);


    }
// xxxxxxxxxxxxxxxxxxxxxxxxxx

  if (((toupper (event->keyval)) == 'J') && routemode)
    {
      forcenextroutepoint = TRUE;
    }

  if ((toupper (event->keyval)) == 'N')
    {
      if (nightmode != 0)
	{
/*  light on for 30 seconds  */
	  if (disableisnight == TRUE)
	    {
/* 	      gtk_timeout_remove (nighttimer); */
	      disableisnight = FALSE;
	    }
	  else
	    lighton ();
	}
    }

/*   From Russell Mirov: */
  if (pdamode)
    {
      if (event->keyval == 0xFF52)
	scalerbt_cb (NULL, 1);	/* RNM */
      if (event->keyval == 0xFF54)
	scalerbt_cb (NULL, 2);	/* RNM */
    }


  return 0;
}

gint
mapclick_cb (GtkWidget * widget, GdkEventButton * event)
{
  gint x, y;
  gdouble lon, lat, vali;
  GdkModifierType state;
  gchar s[200];

/*   printf("\n bin in mapclick\n"); */

  if (event->button)
    gdk_window_get_pointer (event->window, &x, &y, &state);
  else
    {
      x = event->x;
      y = event->y;
      state = event->state;
    }
  if (state == 0)
    return 0;
  calcxytopos (x, y, &lat, &lon, zoom);

  if (downloadwindowactive || importactive)
    {
      if (downloadwindowactive)
	{
	  g_snprintf (s, sizeof (s), "%.5f", lat);
	  if (minsecmode)
	    decimaltomin (s, 1);
	  gtk_entry_set_text (GTK_ENTRY (dltext1), s);
	  g_snprintf (s, sizeof (s), "%.5f", lon);
	  if (minsecmode)
	    decimaltomin (s, 0);
	  gtk_entry_set_text (GTK_ENTRY (dltext2), s);
	  downloadsetparm (NULL, 0);
	}
      else
	{
	  g_snprintf (s, sizeof (s), "%d", x + SCREEN_X_2 + xoff);
	  gtk_entry_set_text (GTK_ENTRY (dltext5), s);
	  g_snprintf (s, sizeof (s), "%d", y + SCREEN_Y_2 + yoff);
	  gtk_entry_set_text (GTK_ENTRY (dltext6), s);

	}
    }
  else
    {
/*        g_print ("\nstate: %x x:%d y:%d", state, x, y); */
      vali = (GTK_ADJUSTMENT (adj)->value);
/*  Left mouse button + shift key */
      if ((state & (GDK_BUTTON1_MASK | GDK_SHIFT_MASK)) ==
	  (GDK_BUTTON1_MASK | GDK_SHIFT_MASK))
	{
	  scalerbt_cb (NULL, 2);
	  return TRUE;
	}
/*  Add mouse position as waypoint */
/*  Left mouse button + control key */
      if ((state & (GDK_BUTTON1_MASK | GDK_CONTROL_MASK)) ==
	  (GDK_BUTTON1_MASK | GDK_CONTROL_MASK))
	{
	  wplat = lat;
	  wplon = lon;
	  addwaypoint_cb (NULL, 0);
	  return TRUE;
	}
/*  Add current position as waypoint */
/*  Right mouse button + control key */
      if ((state & (GDK_BUTTON3_MASK | GDK_CONTROL_MASK)) ==
	  (GDK_BUTTON3_MASK | GDK_CONTROL_MASK))
	{
	  wplat = current_lat;
	  wplon = current_long;

	  addwaypoint_cb (NULL, 0);
	  return TRUE;
	}
/*  Right mouse button + shift key */
      if ((state & (GDK_BUTTON3_MASK | GDK_SHIFT_MASK)) ==
	  (GDK_BUTTON3_MASK | GDK_SHIFT_MASK))
	{
	  scalerbt_cb (NULL, 1);
	  return TRUE;
	}

/*  Left mouse button */
      if ((state & GDK_BUTTON1_MASK) == GDK_BUTTON1_MASK)
	{
	  if (posmode)
	    {
	      posmode_x = lon;
	      posmode_y = lat;
	      rebuildtracklist ();
	      if (onemousebutton)
		gtk_timeout_add (10000, (GtkFunction) posmodeoff_cb, 0);
	    }
	}
/*  Middle mouse button */
      if ((state & GDK_BUTTON2_MASK) == GDK_BUTTON2_MASK)
	{
	  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (posbt), FALSE);

	  rebuildtracklist ();
	}
/*  Right mouse button */
      if ((state & GDK_BUTTON3_MASK) == GDK_BUTTON3_MASK)
	{
/* set  as target */
/* only if RIGHT mouse button clicked */
	  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (posbt), FALSE);
	  rebuildtracklist ();
	  g_strlcpy (targetname, _("SELECTED"), sizeof (targetname));
	  g_snprintf (s, sizeof (s), "%s: %s", _("To"), targetname);
	  gtk_frame_set_label (GTK_FRAME (destframe), s);
	  target_lat = lat;
	  target_long = lon;
	  g_timer_stop (disttimer);
	  g_timer_start (disttimer);
	  olddist = dist;
	}
    }

/*    g_print ("\nx: %d, y: %d", x, y); */
  return TRUE;
}


gint
minimapclick_cb (GtkWidget * widget, GdkEventMotion * event)
{
  gint x, y, px, py;
  gdouble dif, lon, lat;
  GdkModifierType state;

  if (event->is_hint)
    gdk_window_get_pointer (event->window, &x, &y, &state);
  else
    {
      x = event->x;
      y = event->y;
      state = event->state;
    }
  if (state == 0)
    return 0;
#define MINISCREEN_X_2 64
#define MINISCREEN_Y_2 51
  px = (MINISCREEN_X_2 - x) * 10 * pixelfact;
  py = (-MINISCREEN_Y_2 + y) * 10 * pixelfact;
  lat = zero_lat - py / (Ra[(int) (100 + current_lat)] * M_PI / 180.0);
  lat = zero_lat - py / (Ra[(int) (100 + lat)] * M_PI / 180.0);
  lon =
    zero_long -
    px / ((Ra[(int) (100 + lat)] * M_PI / 180.0) * cos (M_PI * lat / 180.0));

  if (mapistopo == FALSE)
    {
      dif = lat * (1 - (cos ((M_PI * fabs (lon - zero_long)) / 180.0)));
      lat = lat - dif / 1.5;
    }
  else
    dif = 0;
  lon =
    zero_long -
    px / ((Ra[(int) (100 + lat)] * M_PI / 180.0) * cos (M_PI * lat / 180.0));


/*        g_print ("\nstate: %x x:%d y:%d", state, x, y); */

/*  Left mouse button */
  if ((state & GDK_BUTTON1_MASK) == GDK_BUTTON1_MASK)
    {
      if (posmode)
	{
	  posmode_x = lon;
	  posmode_y = lat;
	  rebuildtracklist ();
	  if (onemousebutton)
	    gtk_timeout_add (10000, (GtkFunction) posmodeoff_cb, 0);
	}
    }
/*  Middle mouse button */
  if ((state & GDK_BUTTON2_MASK) == GDK_BUTTON2_MASK)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (posbt), FALSE);
      rebuildtracklist ();
    }

/*    g_print ("\nx: %d, y: %d", x, y); */
  return TRUE;
}


gint
addwaypointchange_cb (GtkWidget * widget, guint datum)
{
  gchar *s;
  gdouble lo, la;

  s = g_strstrip ((char *) gtk_entry_get_text (GTK_ENTRY (addwaypoint1)));
  checkinput (s);
  lo = g_strtod (s, NULL);
  if ((lo > -181) && (lo < 181))
    wplon = lo;
  s = g_strstrip ((char *) gtk_entry_get_text (GTK_ENTRY (addwaypoint2)));
  checkinput (s);
  la = g_strtod (s, NULL);
  if ((la > -181) && (la < 181))
    wplat = la;

  return TRUE;
}

gint
addwaypoint (GtkWidget * widget, guint datum)
{
  gint i;
  G_CONST_RETURN gchar *s, *s2;

  s = gtk_entry_get_text (GTK_ENTRY (wptext1));
  s2 = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (wptext2)->entry));
  if (sqlflag)
    {
      insertsqldata (wplat, wplon, (char *) s, (char *) s2);
      getsqltypelist ();
      getsqldata ();
    }
  else
    {
      i = maxwp;
      (wayp + i)->lat = wplat;
      (wayp + i)->lon = wplon;
      g_strdelimit ((char *) s, " ", '_');
/*  limit waypoint name to 20 chars */
      g_strlcpy ((wayp + i)->name, s, 40);
      (wayp + i)->name[20] = 0;
      g_strlcpy ((wayp + i)->typ, s2, 40);
      (wayp + i)->wlan = 0;

      maxwp++;
      if (maxwp >= wpsize)
	{
	  wpsize += 1000;
	  wayp = g_renew (wpstruct, wayp, wpsize);
	}
      savewaypoints ();
    }
  gtk_widget_destroy (GTK_WIDGET (datum));
  markwaypoint = FALSE;
  return TRUE;
}

/*  destroy sel_target window */
gint
addwaypointdestroy_cb (GtkWidget * widget, guint datum)
{

  gtk_widget_destroy (GTK_WIDGET (addwaypointwindow));
  markwaypoint = FALSE;

  return FALSE;
}

gint
addwaypoint_cb (GtkWidget * widget, gpointer datum)
{
  GtkWidget *window, *l1, *l2, *t1, *t2, *hbox3, *hbox4, *hbox5;
  GtkWidget *vbox, *hbox, *hbox2, *button, *button2, *label, *label2;
  gchar buff[40];
  GList *list = NULL;
  gint i;

  if (sqlflag)
    {
      getsqltypelist ();
      for (i = 0; i < dbtypelistcount; i++)
	list = g_list_append (list, dbtypelist[i]);
    }

  addwaypointwindow = window = gtk_dialog_new ();
  gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (mainwindow));
  gotowindow = window;
  markwaypoint = TRUE;

  gtk_window_set_modal (GTK_WINDOW (window), TRUE);
  gtk_window_set_title (GTK_WINDOW (window), _("Add waypoint name"));



  button = gtk_button_new_from_stock (GTK_STOCK_APPLY);
  button2 = gtk_button_new_from_stock (GTK_STOCK_CANCEL);

  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  GTK_WIDGET_SET_FLAGS (button2, GTK_CAN_DEFAULT);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (addwaypoint), GTK_OBJECT (window));
  gtk_signal_connect (GTK_OBJECT (button2), "clicked",
		      GTK_SIGNAL_FUNC (addwaypointdestroy_cb), 0);
  gtk_signal_connect (GTK_OBJECT (window),
		      "delete_event",
		      GTK_SIGNAL_FUNC (addwaypointdestroy_cb), 0);

  vbox = gtk_vbox_new (TRUE, 2);
  gtk_box_pack_start (GTK_BOX
		      (GTK_DIALOG (window)->vbox), vbox, TRUE, TRUE, 2);

  hbox = gtk_hbox_new (TRUE, 2);
  hbox2 = gtk_hbox_new (TRUE, 2);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (hbox), button2, TRUE, TRUE, 2);

  addwaypoint1 = t1 = gtk_entry_new_with_max_length (20);
  g_snprintf (buff, sizeof (buff), "%.5f", wplon);
  if (minsecmode)
    decimaltomin (buff, 0);
  gtk_entry_set_text (GTK_ENTRY (t1), buff);
  l1 = gtk_label_new (_("Longitude"));
  addwaypoint2 = t2 = gtk_entry_new_with_max_length (20);
  g_snprintf (buff, sizeof (buff), "%.5f", wplat);
  if (minsecmode)
    decimaltomin (buff, 1);
  gtk_entry_set_text (GTK_ENTRY (t2), buff);
  l2 = gtk_label_new (_("Latitude"));

  gtk_signal_connect (GTK_OBJECT (t1),
		      "changed", GTK_SIGNAL_FUNC (addwaypointchange_cb),
		      (gpointer) 1);
  gtk_signal_connect (GTK_OBJECT (t2),
		      "changed", GTK_SIGNAL_FUNC (addwaypointchange_cb),
		      (gpointer) 2);

  hbox3 = gtk_hbox_new (TRUE, 2);
  hbox4 = gtk_hbox_new (TRUE, 2);
  hbox5 = gtk_hbox_new (TRUE, 2);

  gtk_box_pack_start (GTK_BOX (hbox3), l2, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (hbox3), t2, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (hbox4), l1, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (hbox4), t1, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox3, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox4, TRUE, TRUE, 2);


  wptext1 = gtk_entry_new ();
  label = gtk_label_new (_(" Waypoint name: "));
  gtk_window_set_default (GTK_WINDOW (window), button);
  gtk_window_set_focus (GTK_WINDOW (window), wptext1);

  gtk_box_pack_start (GTK_BOX (hbox2), label, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (hbox2), wptext1, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox2, TRUE, TRUE, 2);


/*   wptext2 = gtk_entry_new (); */
  wptext2 = gtk_combo_new ();
  gtk_combo_set_popdown_strings (GTK_COMBO (wptext2), (GList *) list);

  label2 = gtk_label_new (_(" Waypoint type: "));

  gtk_box_pack_start (GTK_BOX (hbox5), label2, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (hbox5), wptext2, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox5, TRUE, TRUE, 2);

/*   gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 2); */
  gtk_box_pack_start (GTK_BOX
		      (GTK_DIALOG (window)->action_area),
		      hbox, TRUE, TRUE, 2);


  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  gtk_widget_show_all (window);
  return TRUE;
}


void
insertwaypoints (gint mobile)
{
  gint i, j;
  gchar *text[5], text0[20], text1[20], text2[20], text3[20], name[40];
  gdouble la, lo, dist;
  time_t ti, tif;

/*  insert waypoint into the clist */

  if (!mobile)
    for (i = 0; i < maxwp; i++)
      {
	(wayp + i)->dist = calcdist ((wayp + i)->lon, (wayp + i)->lat);

	text[1] = (wayp + i)->name;

	g_snprintf (text0, sizeof (text0), "%02d", i + 1);
	g_snprintf (text1, sizeof (text1), "%8.5f", (wayp + i)->lat);
	if (minsecmode)
	  decimaltomin (text1, 1);
	g_snprintf (text2, sizeof (text2), "%8.5f", (wayp + i)->lon);
	if (minsecmode)
	  decimaltomin (text2, 0);
	g_snprintf (text3, sizeof (text3), "%9.3f", (wayp + i)->dist);
	text[0] = text0;
	text[2] = text1;
	text[3] = text2;
	text[4] = text3;
	j = gtk_clist_append (GTK_CLIST (mylist), (gchar **) text);
	gtk_clist_set_foreground (GTK_CLIST (mylist), j, &black);
      }

  for (i = 0; i < maxfriends; i++)
    {
      ti = time (NULL);
      tif = atol ((friends + i)->timesec);
      if ((ti - maxfriendssecs) > tif)
	continue;

      if (mobile)
	g_strlcpy (name, "", sizeof (name));
      else
	g_strlcpy (name, "*", sizeof (name));
      g_strlcat (name, (friends + i)->name, sizeof (name));
      g_snprintf (text0, sizeof (text0), "%d", i + maxwp + 1);
      la = g_strtod ((friends + i)->lat, NULL);
      lo = g_strtod ((friends + i)->longi, NULL);
      g_snprintf (text1, sizeof (text1), "%8.5f", la);
      g_snprintf (text2, sizeof (text2), "%8.5f", lo);
      if (minsecmode)
	decimaltomin (text1, 1);

      if (minsecmode)
	decimaltomin (text2, 0);

      if (!mobile)
	{
	  text[0] = text0;
	  text[1] = name;
	  text[2] = text1;
	  text[3] = text2;
	  dist = calcdist (lo, la);
	  g_snprintf (text3, sizeof (text3), "%9.3f", dist);
	  text[4] = text3;
	}
      else
	{
	  text[0] = name;
	  text[1] = text1;
	  text[2] = text2;
	  dist = calcdist (lo, la);
	  g_snprintf (text3, sizeof (text3), "%9.3f", dist);
	  text[3] = text3;
	}

      j = gtk_clist_append (GTK_CLIST (mylist), (gchar **) text);
      if (mobile)
	gtk_clist_set_foreground (GTK_CLIST (mylist), j, &black);
      else
	gtk_clist_set_foreground (GTK_CLIST (mylist), j, &red);
    }

/*  we want te columns sorted by distance from current position */
  gtk_clist_set_sort_column (GTK_CLIST (mylist), (gint) sortcolumn);
  gtk_clist_sort (GTK_CLIST (mylist));
}

gint
setsortcolumn (GtkWidget * w, gpointer datum)
{
  sortflag = !sortflag;
  sortcolumn = (gint) datum;

  if (sortflag)
    gtk_clist_set_sort_type (GTK_CLIST (mylist), GTK_SORT_ASCENDING);
  else
    gtk_clist_set_sort_type (GTK_CLIST (mylist), GTK_SORT_DESCENDING);

  if (w != messagewindow)
    reinsertwp_cb (NULL, 0);
  else
    {
      gtk_clist_set_sort_column (GTK_CLIST (mylist), (gint) sortcolumn);
      gtk_clist_sort (GTK_CLIST (mylist));
    }
  return TRUE;
}


void
insertroutepoints ()
{
  gint i, j;
  gchar *text[5], text0[20], text1[20], text2[20], text3[20];

  i = thisrouteline;
  (wayp + i)->dist = calcdist ((wayp + i)->lon, (wayp + i)->lat);
  text[1] = (wayp + i)->name;
  g_snprintf (text0, sizeof (text0), "%d", i + 1);
  g_snprintf (text1, sizeof (text1), "%8.5f", (wayp + i)->lat);
  if (minsecmode)
    decimaltomin (text1, 1);
  g_snprintf (text2, sizeof (text2), "%8.5f", (wayp + i)->lon);
  if (minsecmode)
    decimaltomin (text2, 0);
  g_snprintf (text3, sizeof (text3), "%9.3f", (wayp + i)->dist);
  text[0] = text0;
  text[2] = text1;
  text[3] = text2;
  text[4] = text3;
  j = gtk_clist_append (GTK_CLIST (myroutelist), (gchar **) text);
  gtk_clist_set_foreground (GTK_CLIST (myroutelist), j, &black);
  g_strlcpy ((routelist + routeitems)->name, (wayp + i)->name, 40);
  (routelist + routeitems)->lat = (wayp + i)->lat;
  (routelist + routeitems)->lon = (wayp + i)->lon;
  routeitems++;
  gtk_widget_set_sensitive (select_route_button, TRUE);

}

void
insertallroutepoints ()
{
  gint i, j;
  gchar *text[5], text0[20], text1[20], text2[20], text3[20];

  for (i = 0; i < maxwp; i++)
    {
      (wayp + i)->dist = calcdist ((wayp + i)->lon, (wayp + i)->lat);
      text[1] = (wayp + i)->name;
      g_snprintf (text0, sizeof (text0), "%d", i + 1);
      g_snprintf (text1, sizeof (text1), "%8.5f", (wayp + i)->lat);
      if (minsecmode)
	decimaltomin (text1, 1);
      g_snprintf (text2, sizeof (text2), "%8.5f", (wayp + i)->lon);
      if (minsecmode)
	decimaltomin (text2, 0);
      g_snprintf (text3, sizeof (text3), "%9.3f", (wayp + i)->dist);
      text[0] = text0;
      text[2] = text1;
      text[3] = text2;
      text[4] = text3;
      j = gtk_clist_append (GTK_CLIST (myroutelist), (gchar **) text);
      gtk_clist_set_foreground (GTK_CLIST (myroutelist), j, &black);
      g_strlcpy ((routelist + routeitems)->name, (wayp + i)->name, 40);
      (routelist + routeitems)->lat = (wayp + i)->lat;
      (routelist + routeitems)->lon = (wayp + i)->lon;
      routeitems++;
    }
  gtk_widget_set_sensitive (select_route_button, TRUE);

}

gint
reinsertwp_cb (GtkWidget * widget, guint datum)
{
  gint i, j, k, val;
  gchar *p;
  GtkAdjustment *ad;

/*  update routine for select target window */
  k = 0;
  ad = gtk_clist_get_vadjustment (GTK_CLIST (mylist));
  val = (GTK_ADJUSTMENT (ad)->value);

  gtk_clist_freeze (GTK_CLIST (mylist));
  gtk_clist_clear (GTK_CLIST (mylist));
  insertwaypoints (FALSE);
  for (i = 0; i < maxwp; i++)
    {
      gtk_clist_get_text (GTK_CLIST (mylist), i, 0, &p);
      j = atol (p);
      if (thisline == j)
	{
	  k = i;
	  break;
	}
    }
  gtk_adjustment_set_value (GTK_ADJUSTMENT (ad), val);
  dontsetwp = TRUE;
  gtk_clist_select_row (GTK_CLIST (mylist), k, 0);
  dontsetwp = FALSE;
  gtk_clist_thaw (GTK_CLIST (mylist));
  return TRUE;
}

gint
click_clist (GtkWidget * widget, GdkEventButton * event, gpointer data)
{

  g_print ("\nclist: %d, data: %d", event->button, thisline);
  if ((event->button == 3))
    {

      return TRUE;
    }

  return FALSE;

}

gint
sel_message_cb (GtkWidget * widget, guint datum)
{
  GtkWidget *window;
  gchar *tabeltitel1[] = {
    _("Name"), _("Latitude"), _("Longitude"), _("Distance"),
    NULL
  };
  GtkWidget *scrwindow, *vbox, *button;

  window = gtk_dialog_new ();
/*    gtk_window_set_policy(GTK_WINDOW(window), TRUE, TRUE, TRUE); */
  gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (mainwindow));
  messagewindow = window;
  gtk_window_set_title (GTK_WINDOW (window),
			_("Please select message recipient"));

  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);

  if (pdamode)
    gtk_window_set_default_size (GTK_WINDOW (window), real_screen_x,
				 real_screen_y);
  else
    gtk_window_set_default_size (GTK_WINDOW (window), 400, 360);

  mylist = gtk_clist_new_with_titles (4, tabeltitel1);

  gtk_signal_connect_object (GTK_OBJECT
			     (GTK_CLIST (mylist)),
			     "click-column", GTK_SIGNAL_FUNC (setsortcolumn),
			     GTK_OBJECT (window));

  gtk_signal_connect (GTK_OBJECT (GTK_CLIST (mylist)),
		      "select-row",
		      GTK_SIGNAL_FUNC (setmessage_cb), GTK_OBJECT (mylist));


  button = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_window_set_default (GTK_WINDOW (window), button);
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     GTK_SIGNAL_FUNC
			     (gtk_widget_destroy), GTK_OBJECT (window));
  gtk_signal_connect_object (GTK_OBJECT (window),
			     "delete_event",
			     GTK_SIGNAL_FUNC
			     (gtk_widget_destroy), GTK_OBJECT (window));

  insertwaypoints (TRUE);
  gtk_clist_set_column_justification (GTK_CLIST (mylist), 3,
				      GTK_JUSTIFY_RIGHT);
  gtk_clist_set_column_auto_resize (GTK_CLIST (mylist), 0, TRUE);
  gtk_clist_set_column_auto_resize (GTK_CLIST (mylist), 1, TRUE);
  gtk_clist_set_column_auto_resize (GTK_CLIST (mylist), 2, TRUE);
  gtk_clist_set_column_auto_resize (GTK_CLIST (mylist), 3, TRUE);


  scrwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (scrwindow), mylist);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW
				  (scrwindow),
				  (GtkPolicyType)
				  GTK_POLICY_AUTOMATIC,
				  (GtkPolicyType) GTK_POLICY_AUTOMATIC);
  vbox = gtk_vbox_new (FALSE, 2 * PADDING);
  gtk_box_pack_start (GTK_BOX
		      (GTK_DIALOG (window)->vbox), vbox, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), scrwindow, TRUE, TRUE, 2 * PADDING);

  gtk_box_pack_start (GTK_BOX
		      (GTK_DIALOG (window)->action_area),
		      button, TRUE, TRUE, 2);

  gtk_widget_show_all (window);
  return TRUE;
}



gint
sel_target_cb (GtkWidget * widget, guint datum)
{
  GtkWidget *window;
  gchar *tabeltitel1[] = { "#",
    _("Waypoint"), _("Latitude"), _("Longitude"), _("Distance"),
    NULL
  };
  GtkWidget *scrwindow, *vbox, *button, *hbox, *deletebt, *gotobt;
  GtkTooltips *tooltips;

  if (setwpactive)
    return TRUE;


  setwpactive = TRUE;
  window = gtk_dialog_new ();
/*    gtk_window_set_policy(GTK_WINDOW(window), TRUE, TRUE, TRUE); */
  gotowindow = window;
  gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (mainwindow));

  if (datum == 1)
    {
      gtk_window_set_modal (GTK_WINDOW (window), TRUE);
      gtk_window_set_title (GTK_WINDOW (window), _("Select reference point"));
    }
  else
    gtk_window_set_title (GTK_WINDOW (window),
			  _("Please select your destination"));
  if (pdamode)
    gtk_window_set_default_size (GTK_WINDOW (window), real_screen_x,
				 real_screen_y);
  else
    gtk_window_set_default_size (GTK_WINDOW (window), 400, 360);

  mylist = gtk_clist_new_with_titles (5, tabeltitel1);
  if (datum == 1)
    gtk_signal_connect (GTK_OBJECT (GTK_CLIST (mylist)),
			"select-row",
			GTK_SIGNAL_FUNC (setrefpoint_cb),
			GTK_OBJECT (mylist));
  else
    {
      gtk_signal_connect (GTK_OBJECT (GTK_CLIST (mylist)),
			  "select-row",
			  GTK_SIGNAL_FUNC (setwp_cb), GTK_OBJECT (mylist));
/*       gtk_signal_connect (GTK_OBJECT (mylist), "button-release-event", */
/* 			  GTK_SIGNAL_FUNC (click_clist), NULL); */
    }

  gtk_signal_connect (GTK_OBJECT
		      (GTK_CLIST (mylist)),
		      "click-column", GTK_SIGNAL_FUNC (setsortcolumn), 0);

  if (datum != 1)
    {
      if (routemode)
	create_route_button = gtk_button_new_with_label (_("Edit route"));
      else
	create_route_button = gtk_button_new_with_label (_("Create route"));
      GTK_WIDGET_SET_FLAGS (create_route_button, GTK_CAN_DEFAULT);
      gtk_signal_connect (GTK_OBJECT (create_route_button), "clicked",
			  GTK_SIGNAL_FUNC (create_route_cb), 0);
    }

  deletebt = gtk_button_new_from_stock (GTK_STOCK_DELETE);
  GTK_WIDGET_SET_FLAGS (deletebt, GTK_CAN_DEFAULT);
  gtk_signal_connect (GTK_OBJECT (deletebt), "clicked",
		      GTK_SIGNAL_FUNC (delwp_cb), 0);

  gotobt = gtk_button_new_from_stock (GTK_STOCK_JUMP_TO);
  GTK_WIDGET_SET_FLAGS (gotobt, GTK_CAN_DEFAULT);
  gtk_signal_connect (GTK_OBJECT (gotobt), "clicked",
		      GTK_SIGNAL_FUNC (jumpwp_cb), 0);

/*   button = gtk_button_new_with_label (_("Close")); */
  button = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_window_set_default (GTK_WINDOW (window), button);
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     GTK_SIGNAL_FUNC
			     (sel_targetweg_cb), GTK_OBJECT (window));
  gtk_signal_connect_object (GTK_OBJECT (window),
			     "delete_event",
			     GTK_SIGNAL_FUNC
			     (sel_targetweg_cb), GTK_OBJECT (window));

  insertwaypoints (FALSE);
  gtk_clist_set_column_justification (GTK_CLIST (mylist), 4,
				      GTK_JUSTIFY_RIGHT);
  gtk_clist_set_column_justification (GTK_CLIST (mylist), 0,
				      GTK_JUSTIFY_RIGHT);
  gtk_clist_set_column_auto_resize (GTK_CLIST (mylist), 0, TRUE);
  gtk_clist_set_column_auto_resize (GTK_CLIST (mylist), 1, TRUE);
  gtk_clist_set_column_auto_resize (GTK_CLIST (mylist), 2, TRUE);
  gtk_clist_set_column_auto_resize (GTK_CLIST (mylist), 3, TRUE);
  gtk_clist_set_column_auto_resize (GTK_CLIST (mylist), 4, TRUE);

  scrwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (scrwindow), mylist);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW
				  (scrwindow),
				  (GtkPolicyType)
				  GTK_POLICY_AUTOMATIC,
				  (GtkPolicyType) GTK_POLICY_AUTOMATIC);
  vbox = gtk_vbox_new (FALSE, 2 * PADDING);
  gtk_box_pack_start (GTK_BOX
		      (GTK_DIALOG (window)->vbox), vbox, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), scrwindow, TRUE, TRUE, 2 * PADDING);
  hbox = gtk_hbutton_box_new ();
  gtk_box_pack_start (GTK_BOX
		      (GTK_DIALOG (window)->action_area),
		      hbox, TRUE, TRUE, 2);

  if (datum != 1)
    {
      gtk_box_pack_start (GTK_BOX (hbox), create_route_button, TRUE, TRUE,
			  2 * PADDING);
      gtk_box_pack_start (GTK_BOX (hbox), deletebt, TRUE, TRUE, 2 * PADDING);
      gtk_box_pack_start (GTK_BOX (hbox), gotobt, TRUE, TRUE, 2 * PADDING);
    }
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 2 * PADDING);
/*    gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER); */

/*   I remove this, because you can sort by mouseclick now */
/*   selwptimeout = gtk_timeout_add (30000, (GtkFunction) reinsertwp_cb, 0); */
  tooltips = gtk_tooltips_new ();
  if (!createroute)
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), create_route_button,
			  _
			  ("Create a route using some waypoints from this list"),
			  NULL);
  if (setwpactive)
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), deletebt,
			  _
			  ("Delete the selected waypoint from the waypoint list"),
			  NULL);
  if (setwpactive)
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), gotobt,
			  _("Jump to the selected waypoint"), NULL);

  gtk_widget_show_all (window);

  return TRUE;
}


gint
create_route_cb (GtkWidget * widget, guint datum)
{
  GtkWidget *window;
  gchar *tabeltitel1[] = { "#",
    _("Waypoint"), _("Latitude"), _("Longitude"), _("Distance"),
    NULL
  };
  GtkWidget *scrwindow, *vbox, *button, *button3, *hbox, *hbox2, *l1;
  gint i, j;
  gchar *text[5], text0[20], text1[20], text2[20], text3[20];
  GtkTooltips *tooltips;

  createroute = TRUE;
  window = gtk_dialog_new ();
  routewindow = window;
/*    gtk_window_set_policy(GTK_WINDOW(window), TRUE, TRUE, TRUE); */
  gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (mainwindow));

  gtk_window_set_title (GTK_WINDOW (window), _("Define route"));
  gtk_window_set_default_size (GTK_WINDOW (window), 320, 320);
  myroutelist = gtk_clist_new_with_titles (5, tabeltitel1);
  gtk_signal_connect (GTK_OBJECT (GTK_CLIST (myroutelist)),
		      "select-row",
		      GTK_SIGNAL_FUNC (setroutetarget),
		      GTK_OBJECT (myroutelist));

  select_route_button = gtk_button_new_with_label (_("Start route"));
  gtk_widget_set_sensitive (select_route_button, FALSE);

  GTK_WIDGET_SET_FLAGS (select_route_button, GTK_CAN_DEFAULT);
  gtk_signal_connect (GTK_OBJECT (select_route_button), "clicked",
		      GTK_SIGNAL_FUNC (do_route_cb), 0);
  gtk_window_set_default (GTK_WINDOW (window), select_route_button);

  create_route2_button =
    gtk_button_new_with_label (_("Take all WP as route"));
  GTK_WIDGET_SET_FLAGS (create_route2_button, GTK_CAN_DEFAULT);
  gtk_signal_connect (GTK_OBJECT (create_route2_button), "clicked",
		      GTK_SIGNAL_FUNC (insertallroutepoints), 0);

  button = gtk_button_new_with_label (_("Abort route"));
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     GTK_SIGNAL_FUNC
			     (sel_routecancel_cb), GTK_OBJECT (window));
  gtk_signal_connect_object (GTK_OBJECT (window),
			     "delete_event",
			     GTK_SIGNAL_FUNC
			     (sel_routeclose_cb), GTK_OBJECT (window));

/*   button3 = gtk_button_new_with_label (_("Close")); */
  button3 = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
  GTK_WIDGET_SET_FLAGS (button3, GTK_CAN_DEFAULT);
  gtk_signal_connect_object (GTK_OBJECT (button3), "clicked",
			     GTK_SIGNAL_FUNC
			     (sel_routeclose_cb), GTK_OBJECT (window));


  gtk_clist_set_column_justification (GTK_CLIST (myroutelist), 4,
				      GTK_JUSTIFY_RIGHT);
  gtk_clist_set_column_justification (GTK_CLIST (myroutelist), 0,
				      GTK_JUSTIFY_RIGHT);
  gtk_clist_set_column_auto_resize (GTK_CLIST (myroutelist), 0, TRUE);
  gtk_clist_set_column_auto_resize (GTK_CLIST (myroutelist), 1, TRUE);
  gtk_clist_set_column_auto_resize (GTK_CLIST (myroutelist), 2, TRUE);
  gtk_clist_set_column_auto_resize (GTK_CLIST (myroutelist), 3, TRUE);
  gtk_clist_set_column_auto_resize (GTK_CLIST (myroutelist), 4, TRUE);

  scrwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (scrwindow), myroutelist);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW
				  (scrwindow),
				  (GtkPolicyType)
				  GTK_POLICY_AUTOMATIC,
				  (GtkPolicyType) GTK_POLICY_AUTOMATIC);
  vbox = gtk_vbox_new (FALSE, 2);
/*   gtk_container_add (GTK_CONTAINER (window), vbox); */
  gtk_box_pack_start (GTK_BOX
		      (GTK_DIALOG (window)->vbox), vbox, TRUE, TRUE, 2);

  gtk_box_pack_start (GTK_BOX (vbox), scrwindow, TRUE, TRUE, 2);
  hbox = gtk_hbox_new (TRUE, 2);
  hbox2 = gtk_hbox_new (TRUE, 2);
  if (!routemode)
    l1 = gtk_label_new (_("Click on waypoints list\nto add waypoints"));
  else
    l1 = gtk_label_new (_("Click on list item\nto select next waypoint"));
  gtk_box_pack_start (GTK_BOX (vbox), l1, FALSE, FALSE, 2);
/*   gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2); */
  gtk_box_pack_start (GTK_BOX
		      (GTK_DIALOG (window)->action_area),
		      hbox, TRUE, TRUE, 2);

  gtk_box_pack_start (GTK_BOX (hbox), select_route_button, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (hbox), create_route2_button, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox2, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (hbox), button3, TRUE, TRUE, 2);
  gtk_widget_set_sensitive (create_route_button, FALSE);

  if (routemode)
    {
      gtk_widget_set_sensitive (select_route_button, FALSE);
      gtk_clist_clear (GTK_CLIST (myroutelist));
      for (i = 0; i < routeitems; i++)
	{
	  (routelist + i)->dist =
	    calcdist ((routelist + i)->lon, (routelist + i)->lat);
	  text[1] = (routelist + i)->name;
	  g_snprintf (text0, sizeof (text0), "%d", i + 1);
	  g_snprintf (text1, sizeof (text1), "%8.5f", (routelist + i)->lat);
	  g_snprintf (text2, sizeof (text2), "%8.5f",
		      (routelist + i)->lon);
	  g_snprintf (text3, sizeof (text3), "%9.3f", (routelist + i)->dist);
	  text[0] = text0;
	  text[2] = text1;
	  text[3] = text2;
	  text[4] = text3;
	  j = gtk_clist_append (GTK_CLIST (myroutelist), (gchar **) text);
	  gtk_clist_set_foreground (GTK_CLIST (myroutelist), j, &black);
	}
    }
  else
    routeitems = 0;
  tooltips = gtk_tooltips_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), create_route2_button,
			_
			("Create a route from all waypoints. Sorted with order in file, not distance."),
			NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), select_route_button,
			_
			("Click here to start your journey. GpsDrive guides you through the waypoints in this list."),
			NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), button,
			_("Abort your journey"), NULL);

  gtk_widget_show_all (window);

  return TRUE;
}



void
usage ()
{

/*** Mod by Arms */
  g_print ("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
	   "\nCopyright (c) 2001-2004 Fritz Ganter <ganter@ganter.at>"
	   "\n              Website: http://www.gpsdrive.de\n\n",
	   _("-v    show version\n"),
	   _("-h    print this help\n"),
	   _("-d    turn on debug info\n"),
	   _("-D    turn on lot of debug info\n"),
	   _("-e    use Festival-Lite (flite) for speech output\n"),
	   _("-t    set serial device for GPS i.e. /dev/ttyS1\n"),
	   _("-o    serial device, pty master, or file for NMEA *output*\n"),
	   _("-f X  Select friends server, X is i.e. www.gpsdrive.cc\n"),
	   _("-n    Disable use of direct serial connection\n"),
	   _("-l X  Select language of the voice,\n"
	     "      X may be english, spanish or german\n"),
	   _("-s X  set height of the screen, if autodetection\n"
	     "      don't satisfy you, X is i.e. 768,600,480,200\n"),
/*** Mod by Arms */
	   _("-r X  set width of the screen, only with -s\n"),
	   _
	   ("-1    have only 1 button mouse, for example using touchscreen\n"),
	   _("-a    don't display battery status (i.e. broken APM)\n"),
	   _
	   ("-b X  Servername for NMEA server (if gpsd runs on another host)\n"),
	   _
	   ("-c X  set start position in simulation mode to waypoint name X\n"),
	   _("-x    create separate window for menu\n"),
	   _("-p    set settings for PDA (iPAQ, Yopy...)\n"),
	   _
	   ("-i    ignore NMEA checksum (risky, only for broken GPS receivers\n"),
	   _("-q    disable SQL support\n"),
	   _("-F    force display of position even it is invalid\n"),
	   _("-S    don't show splash screen\n"),
	   _("-E    print out data received from direct serial connection\n"),
	   _
	   ("-W x  set x to 1 to switch WAAS/EGNOS on, set to 0 to switch off\n"),
	   _("-H X  correct altitude, adding this value to altitude\n"),
	   _("-z    don't display zoom factor and scale\n\n"));

}

/*  load the definitions of the map files */

gint
loadmapconfig ()
{
  gchar mappath[400];
  FILE *st;
  gint i;
  gchar buf[1512], s1[40], s2[40], s3[40], filename[100];
  gint p, e;

  init_nasa_mapfile ();
  if (mapdir[strlen (mapdir) - 1] != '/')
    g_strlcat (mapdir, "/", sizeof (mapdir));

  g_strlcpy (mappath, mapdir, sizeof (mappath));
  g_strlcat (mappath, "map_koord.txt", sizeof (mappath));
  st = fopen (mappath, "r");
  if (st == NULL)
    {
      mkdir (homedir, 0777);
      st = fopen (mappath, "w+");
      if (st == NULL)
	{
	  perror (mappath);
	  return FALSE;
	}
      st = freopen (mappath, "r", st);
      if (st == NULL)
	{
	  perror (mappath);
	  return FALSE;
	}

    }
  if (nrmaps > 0)
    g_free (maps);

  maps = g_new (mapsstruct, 1);
  i = nrmaps = 0;
  havenasa = -1;
  while ((p = fgets (buf, 1512, st) != 0))
    {
      e = sscanf (buf, "%s %s %s %s", filename, s1, s2, s3);
      if (e == 4)
	{
	  g_strdelimit (s1, ",", '.');
	  g_strdelimit (s2, ",", '.');
	  g_strdelimit (s3, ",", '.');
	  g_strlcpy ((maps + i)->filename, filename, 200);
	  (maps + i)->lat = g_strtod (s1, NULL);
	  (maps + i)->lon = g_strtod (s2, NULL);
	  (maps + i)->scale = strtol (s3, NULL, 0);
	  i++;
	  nrmaps = i;
	  havenasa = -1;
	  maps = g_renew (mapsstruct, maps, (i + 2));
	}
    }
  fclose (st);
  needreloadmapconfig = FALSE;
  return FALSE;
}

/*  write the definitions of the map files */
/* Attention! program  writes decimal point as set in locale
   i.eg 4.678 is in Germany 4,678 !!! */
void
savemapconfig ()
{
  gchar mappath[400];
  FILE *st;
  gint i;

  if (mapdir[strlen (mapdir) - 1] != '/')
    g_strlcat (mapdir, "/", sizeof (mapdir));

  g_strlcpy (mappath, mapdir, sizeof (mappath));
  g_strlcat (mappath, "map_koord.txt", sizeof (mappath));
  st = fopen (mappath, "w");
  if (st == NULL)
    {
      perror (mappath);
      exit (2);
    }

  for (i = 0; i < nrmaps; i++)
    {
      fprintf (st, "%s %.5f %.5f %ld\n", (maps + i)->filename,
	       (maps + i)->lat, (maps + i)->lon, (maps + i)->scale);
    }

  fclose (st);
}


/* Load track file and displays it */
gint
loadtrack_cb (GtkWidget * widget, gpointer datum)
{
  GtkWidget *fdialog;
  gchar buf[1000];
  fdialog = gtk_file_selection_new (_("Select a track file"));
  gtk_window_set_modal (GTK_WINDOW (fdialog), TRUE);
  gtk_window_set_transient_for (GTK_WINDOW (fdialog),
				GTK_WINDOW (mainwindow));

  gtk_signal_connect (GTK_OBJECT
		      (GTK_FILE_SELECTION (fdialog)->ok_button),
		      "clicked", GTK_SIGNAL_FUNC (gettrackfile),
		      GTK_OBJECT (fdialog));
  gtk_signal_connect_object (GTK_OBJECT
			     (GTK_FILE_SELECTION (fdialog)->
			      cancel_button), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT (fdialog));


  g_strlcpy (buf, homedir, sizeof (buf));
  g_strlcat (buf, "track*.sav", sizeof (buf));
  gtk_file_selection_complete (GTK_FILE_SELECTION (fdialog), buf);
  gtk_widget_show (fdialog);

  return TRUE;
}


#include "f_savewaypoints.c"
#include "f_loadwaypoints.c"
#include "f_initgps.c"
#include "f_usr2handler.c"
#include "f_main.c"
