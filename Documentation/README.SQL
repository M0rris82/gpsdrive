Version for gpsdrive-2.x

Since version 1.29 GpsDrive supports SQL database support.

Supported databases (September 2003): MySQL 

Compiling:
----------

There is no MySQL needed for compiling, the needed library
libmysqlclient.so.10 will be loaded at runtime, if found.

Running and first initialization:
---------------------------------

If you have not already done, install the mysql server package for
your distribution.

o Start the server, mostly with: /etc/init.d/mysql start

UPDATE:
-------------------------------------------------------------------------------

There is now a new way to create the initial database but you do need
to have perl installed.  The new script is gpsdrive/bin/geoinfo.pl and
the requierd command line is discussed below. Check out geoinfo --help
to see all of its options.

-------------------------------------------------------------------------------

o To create a initial database with included Perl file 'geoinfo.pl'.
  If this is a new database and you haven't created a root user yet
  something like the following should do the trick.  Otherwise use the
  MYSQL Administrator.

  /usr/bin/mysqladmin -u root password 'enter-your-good-new-password-here'

  Once you have done that run the perl script like this.
  geoinfo.pl --db-user=root --db-password='enter-your-good-new-password-here'


Update:
-------

If you already have the database and you update GpsDrive, make sure
your table structure is updated with the above geoinfo.pl script.

Security:
---------

GpsDrive uses the username 'gast' and the password 'gast' as default
to access the table 'waypoints' in the database 'geoinfo'. If you want
to change this in the SQL database, change this in GpsDrive also. Edit
the file $HOME/.gpsdrive/gpsdriverc lines for dbuser and dbpass.

Be aware that the MySQL password for this database is stored in
cleartext in gpsdriverc!!!

Importing existing waypoint files:
----------------------------------
I also provided the script 'wp2sql', which creates the .sql input file
from gpsdrive waypointfiles. 

Change it for your needs first!!!!

Then do 

mysql -u gast -pgast <wp.sql

You can also import ASCII files with the 'mysqlimport' program.

=======================================================================
IMPORTANT:

Select the 'Use SQL' button to use the SQL generated waypoints.

=======================================================================

Waypoint types:
---------------

You can define freely your waypoint type names. But there are special
names, which results in special icons display in GpsDrive:

WLAN	     : open WLAN accesspoint
WLAN-WEP     : crypted WLAN accesspoint
Rest	     : a restaurant
McDonalds    : a kind of restaurant
BurgerKing   : another kind of restaurant
Hotel	     : A hotel
Shop	     : Shop
Monu	     : Monument, touristic attraction
Speedtrap    : Speedtrap (Radar), gives warning if this is in front of
	       you
Nightclub    : Pssst...
Airport	     : Airport
Golf	     : Golf course
Gasstation   : Gasstation
Cafe	     : coffee bar
Geocache     : Geocache point

The names are case insensitive.

You can also use your own icons, see man page for details.

Backing up and restoring your database
--------------------------------------

Sven added the two shell scripts gpssql_backup.sh and
gpssql_restore.sh for very simple use. Just read the comments within
those scripts for usage information.


Editing your waypoints:
-----------------------

You can use "mysqlcc" (http://www.mysql.com/downloads/gui-mycc.html),
"phpmyadmin" or OpenOffice to edit the waypoints.

Here is a little HowTo from Charles Curley <charlescurley@charlescurley.com>

One of the really nice things about OpenOffice.org
(http://OpenOffice.org/) is that you can use databases as data sources
for documents and spreadsheets, rather like MS Access and MS Office. I
just set up the GpsDrive waypoints MySQL database as a data source.

I did it on Fedora Core Linux. This should work on other Linux disties
as well.

For Openoffice 1.x:

First, get John McCreesh's "OpenOffice.org 1.0, ODBC, and MySQL
'How-to'" (http://www.unixodbc.org/doc/OOoMySQL9.pdf), read it through
page 7, then and do exactly what the man says, step by step. His
instructions are for Red Hat 9, and I had no problems. His
instructions are for OpenOffice.org 1.0, but I had no problems on
OpenOffice.org 1.1.

I used exactly his [MySQL] stanza in /etc/odbcinst.ini. For odbc.ini,
I made two changes. First, I used ~/.odbc.ini instead of /etc/odbc.ini
for tighter security. Second, I used a customized stanza for the
geoinfo database:

[geoinfo]
Trace    = Off
TraceFile= stderr
Driver   = MySQL
SERVER   = localhost
Database = geoinfo
USER     = gast
PASSWORD = gast
PORT     = 3306

Third, in the ODBC tab of the Data Source Administration window, check
"password required" and enter a user name of "gast".

For OpenOffice.org 2.0.2:

Follow this writeup:

http://www.linuxgangster.org/modules.php?name=Content&file=viewarticle&id=10

Then add the following to your odbc.ini:

[MySQL-geoinfo]
Description = MySQL geoinfo database
Driver = MySQL
Server = localhost
Database = geoinfo
Port = 3306

Testing:

To test the ODBC connection to MySQL, you run isql like so:

isql <dbname> <user> <password>
isql geoinfo gast gast

and in isql, run:

select * on waypoints

The result is a nice GUI tool for editing one's waypoints.
