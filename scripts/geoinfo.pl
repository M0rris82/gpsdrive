#!/usr/bin/perl
# Handling of POI/Streets Vector Data:
# For now:
# Retrieve POI Data from Different Sources
# And import them into mySQL for use with gpsdrive

# Get version number from version-control system, as integer 
   
my $Version = '$Revision$'; 
$Version =~ s/\$Revision:\s*(\d+)\s*\$/$1/; 

my $VERSION ="geoinfo.pl (c) Joerg Ostertag
Initial Version (Jan,2005) by Joerg Ostertag <joerg.ostertag\@rechengilde.de>
Version 0.9-$Version
";

BEGIN {
    my $dir = $0;
    $dir =~s,[^/]+/[^/]+$,,;
    unshift(@INC,"$dir/perl_lib");

    # For Debug Purpose in the build Directory
    unshift(@INC,"./perl_lib");
    unshift(@INC,"./osm/perl_lib");
    unshift(@INC,"./scripts/osm/perl_lib");
    unshift(@INC,"../scripts/osm/perl_lib");

    # For Windows
    unshift(@INC,"/cygdrive/c/Dokumente und Einstellungen/ulfl/Eigene Dateien/svn.gpsdrive.cc/scripts/osm/perl_lib");

    # For DSL (Damned Small Linux)
    unshift(@INC,"/opt/gpsdrive/share/perl5");
    unshift(@INC,"/opt/gpsdrive");

};

use strict;
use warnings;

#use POI::KismetXml;
use Data::Dumper;

#use Date::Manip qw(ParseDate DateCalc UnixDate);
use File::Basename;
use File::Copy;
use File::Path;
use Getopt::Long;
use HTTP::Request;
use IO::File;
use Pod::Usage;

use Utils::Debug;

use Geo::Gpsdrive::DBFuncs;
use Geo::Gpsdrive::Utils;
use Geo::Gpsdrive::Gps;

use Geo::Gpsdrive::DB_Defaults;
use Geo::Gpsdrive::GpsDrive;
use Geo::Gpsdrive::JiGLE;
use Geo::Gpsdrive::Kismet;
use Geo::Gpsdrive::OSM;
use Geo::Gpsdrive::FON;
use Geo::Gpsdrive::PocketGpsPoi;
use Geo::Gpsdrive::Way_Txt;
use Geo::Gpsdrive::getstreet;
use Geo::Gpsdrive::gettraffic;

my ($man,$help);

our $CONFIG_DIR    = "$ENV{'HOME'}/.gpsdrive"; # Should we allow config of this?
our $CONFIG_FILE   = "$CONFIG_DIR/gpsdriverc";
our $MIRROR_DIR   = "$CONFIG_DIR/MIRROR";
our $UNPACK_DIR   = "$CONFIG_DIR/UNPACK";
our $GPSDRIVE_DB_NAME = "geoinfo";

our $development_version = (`id` =~ m/tweety/);
our $no_delete;

my @osm_files            = ();
my @fon_countries;
my $do_mapsource_points  = 0; 
my $do_cameras           = 0;
my $do_all               = 0;
my $do_create_db         = 0;
my $do_gpsdrive_tracks   = 0;
my $do_kismet_tracks     = 0;
my $do_jigle             = 0;
my $do_import_defaults   = 0;
my $do_import_way_txt    = 0;
my $do_traffic;
my $show_traffic;
our $do_delete_db_content = 0;
our $do_collect_init_data = 0;
our $street;
our $ort;
our $plz;
our $thread;
our $type;
our $sql;
our $file;


our ($lat_min,$lat_max,$lon_min,$lon_max) = (0,0,0,0);

our $lang                = 'de';
our $db_user             = $ENV{DBUSER} || '';
our $db_password         = $ENV{DBPASS} || '';

Geo::Gpsdrive::DBFuncs::db_read_mysql_sys_pwd();

$db_user           ||= 'gast';
$db_password       ||= 'gast';

our $db_host             = $ENV{DBHOST} || 'localhost';
#$db_host = 'host=localhost;mysql_socket=/home/tweety/.gpsdrive/mysql/mysqld.socket';
my $areas_todo;
my $do_list_areas=0;
my $do_show_version=0;

# Set defaults and get options from command line
Getopt::Long::Configure('no_ignore_case');
pod2usage(1)
    unless @ARGV;
GetOptions ( 
	     'create-db'           => \$do_create_db,
	     'fill-defaults'       => \$do_import_defaults,
	     'openstreetmap:s@'     => \@osm_files,
	     'osm:s@'               => \@osm_files,
	     'osm_polite=s'        => \$Geo::Gpsdrive::OSM::OSM_polite,
	     'fon:s@'              => \@fon_countries,
	     'mapsource_points=s'  => \$do_mapsource_points,
	     'cameras'             => \$do_cameras,
	     'gpsdrive-tracks'     => \$do_gpsdrive_tracks,
	     'kismet-tracks=s'     => \$do_kismet_tracks,
	     'jigle=s'     	   => \$do_jigle,
	     'import-way-txt'      => \$do_import_way_txt,
	     'all'                 => \$do_all,
	     'u=s'                 => \$db_user,
	     'p=s'                 => \$db_password,
	     'db-name=s'           => \$GPSDRIVE_DB_NAME,
	     'db-user=s'           => \$db_user,
	     'db-password=s'       => \$db_password,
	     'db-host=s'           => \$db_host,
	     'delete-db-content'   => \$do_delete_db_content,
	     'collect-init-data'   => \$do_collect_init_data,
	     'lat_min=s'           => \$lat_min,      
	     'lat_max=s'           => \$lat_max,
	     'lon_min=s'           => \$lon_min,      
	     'lon_max=s'           => \$lon_max,
	     'lat-min=s'           => \$lat_min,      
	     'lat-max=s'           => \$lat_max,
	     'lon-min=s'           => \$lon_min,      
	     'lon-max=s'           => \$lon_max,
	     'area=s'              => \$areas_todo,
	     'list-areas'          => \$do_list_areas,
	     'no-delete'           => \$no_delete,
	     'd+'                  => \$DEBUG,
	     'debug+'              => \$DEBUG,      
	     'verbose'             => \$VERBOSE,
	     'v+'                  => \$VERBOSE,
	     'debug_range=s'       => \$debug_range,      
	     'no-mirror'           => \$no_mirror,
	     'proxy=s'             => \$PROXY,
	     'MAN'                 => \$man, 
	     'man'                 => \$man, 
	     'street=s'		   => \$street, #need for getstreet
	     'city=s'		   => \$ort,
	     'zip=s'		   => \$plz,
	     'sql'		   => \$sql,
	     'thread'		   => \$thread,
	     'file'		   => \$file,
	     'get-traffic'	   => \$do_traffic,
	     'show-traffic'	   => \$show_traffic,
	     'h|help|x'            => \$help, 
	     'lang=s'              => \$lang, 
	     'version'             => \$do_show_version, 
	     )
    or pod2usage(1);


if ( $do_show_version ) {
    print "$VERSION\n";
};

if ( $do_all ) {
    $do_create_db
	= @osm_files
	= $do_gpsdrive_tracks
	= $do_cameras
	= $do_jigle
	= $do_import_defaults
	= 1;
    if ( $ENV{'LANG'} =~ /de_DE/ ) {
	print "\n";
	print "=============================================================================\n";
	print "I assume i'm in Germany (LANG='$ENV{'LANG'}')\n";

    }
}

if ( $do_list_areas ) {
    print Geo::Filter::Area->list_areas()."\n";
}

pod2usage(1) if $help;
pod2usage(-verbose=>2) if $man;

########################################################################################
########################################################################################
########################################################################################
#
#                     Main
#
########################################################################################
########################################################################################
########################################################################################


Geo::Gpsdrive::DBFuncs::create_db()
    if $do_create_db;

Geo::Gpsdrive::DB_Defaults::fill_defaults()
    if $do_import_defaults || $do_create_db;

# Get and Unpack openstreetmap  http://www.openstreetmap.org/
Geo::Gpsdrive::OSM::import_Data($areas_todo,@osm_files) 
    if ( @osm_files );

# Import FON file into poi Database
@fon_countries=qw(DE AT)
    if @fon_countries && $fon_countries[0] eq ''; ## No country given
Geo::Gpsdrive::FON::import_Data(@fon_countries) 
    if ( @fon_countries );

Geo::Gpsdrive::gettraffic::gettraffic()
	if $do_traffic;

Geo::Gpsdrive::gettraffic::showtraffic()
	if $show_traffic;

# Convert MapSource Waypoints to gpsdrive POI
Geo::Gpsdrive::mapsource::import_Data()
    if ( $do_mapsource_points );

# Get and Unpack POCKETGPS_DIR      http://www.pocketgpspoi.com
Geo::Gpsdrive::PocketGpsPoi::import_Data()
    if ( $do_cameras );

# Import Waypoints from Way.txt
Geo::Gpsdrive::Way_Txt::import_Data()
    if $do_import_way_txt;

# extract street Data from all tracks
Geo::Gpsdrive::GpsDrive::import_Data() 
    if ( $do_gpsdrive_tracks );

# extract street Data from all tracks
Geo::Gpsdrive::Kismet::import_Data($do_kismet_tracks) 
    if ( $do_kismet_tracks );

# extract WLAN Points from JiGLE Data
Geo::Gpsdrive::JiGLE::import_Data($do_jigle) 
    if ( $do_jigle );


__END__

=head1 NAME

B<geoinfo.pl> Version 0.9

=head1 DESCRIPTION

B<geoinfo.pl> is a program to download and convert waypoints 
and other geospacial data for use with the new gpsdrive 
POI Database. 
You need to have mySQL Support.

This program is completely experimental, but some data 
can already be retrieved with it.

So: Have Fun, improve it and send me fixes :-))

WARNING: 
    This program replaces some/all waypoints of desire. 
    So any changes made to the database may be overwritten!!!
    If you have any self collected/changed Data do a backup first!!


=head1 SYNOPSIS

B<Common usages:>

geoinfo.pl [-d] [-v] [-h] [--mapsource_points='Filename']

=head1 OPTIONS

=over 2

=item B<--man> Complete documentation

Complete documentation


=item B<--create-db>

Try creating the tables inside the geoinfo database. 
This also fills the database with some predefined types.
and imports wour way*.txt Files.
This also creates and modified the old waypoints table.
This implies --fill-defaults

=item B<--fill-defaults>

Fill the Databases with useful defaults. This option is 
needed before you can import any of the other importers.


=item B<--openstreetmap> B<--osm>[=filename]

Download and import openstreetmap Data. 
If no parameter is given. planet.osm is downloaded and then imported. 
If a filename is given this Filename is imported. The file needs to 
be a *.osm File. You can get a osm File by saving your edited Data whis josm.
Currently we only read Nodes from OSM. But the rules in icons.xml are a 
little bit outdated. So not every POI from osm is really imported. If you
like more POIs imported from OSM, please update icons.xml.

=item B<--all>

Triggers all of the above


=item B<--fon=filename>

Download and Import Fonero Accesspoint positions from http://www.fon.com/


=item B<--collect-init-data>

Collects default data and writes them into the default Files.
This option is normally used by the maintainer to create the 
Defaults for filling the DB.


=item B<--gpsdrive-tracks>

Read all gpsdrive Tracks 
and insert into  streets DB


=item B<--import-way-txt>

Read all gpsdrive way*.txt 
and insert into  poi DB


=item B<--kismet-tracks=Directory>

Read all Kismet .gps Files in 'Directory', extract the Tracks
and insert them into streets DB


=item B<--jigle=Directory>

Read all jigle Files in directory, extract the Wavelan points
and insert them into POI DB

See http://www.wigle.net/ for the jiggle client

=item B<--get-traffic>

use for get traffic information from http://www.freiefahrt.info/rdstmc.do and write it in mysql

This feature is not completels implemented yet.

=item B<--show-traffic>

show traffic from mysql database

This feature is not completels implemented yet.

=item B<--street --ort [--plz] [--sql]>

use this for download street coordinates at the moment only germany is supported

Example: ./geoinfo.pl --street "Straﬂe des Friedens" --ort Teutschenthal --plz 06179 --sql

--street is needed for the streetname, if the name has more then one word you have to use ""

--ort is for the name of the city

--plz is not evertime necessary only in a big city or if the there are more citys with the same name or a similarly name
whitout --sql the result will be write in ~/.gpsdrive/way.txt

=item B<--lat_min --lat_max --lon_min --lon_max>

For example for debug reasons for some inserts limit to the 
given rectangle.
This feature is not implemented on all insert statements yet.
The values must be set to a none 0 value to be accepted.

=item B<--no-delete>

Does not delete the old entries in the DB prior to inserting the new ones.

=item B<--db-name>

Name of Database to use; default is geoinfo

=item B<--db-user>

username to connect to mySQL database. Default is gast


=item B<--db-password>

password for user to connect to mySQL database. Default is gast

=item B<--db-host>

hostname for  connecting to your mySQL database. Default is localhost

=item B<--no-mirror>

Do not try mirroring the files from the original Server. Only use
files found on local Filesystem.

=item B<--proxy="hostname:port">

use proxy for download

=item B<--lang>

language of the POI-Type descriptions that will be used in the database.
At the moment the default is 'de' for german. If no entries are available
for the specified language, the english ones will be used.

=item B<--area=germany> Area Filter

Only read area for processing
Currently only for osm imports
to see which areas are available, use --list-areas

=item B<--list-areas>

print all areas possible

=back
