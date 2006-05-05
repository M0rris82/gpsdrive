#!/usr/bin/perl
# Handling of POI/Streets Vector Data:
# For now:
# Retrieve POI Data from Different Sources
# And import them into mySQL for use with gpsdrive

my $VERSION ="geoinfo.pl (c) Joerg Ostertag
Initial Version (Jan,2005) by Joerg Ostertag <joerg.ostertag\@rechengilde.de>
Version 1.17
";


BEGIN {
    # For Debug Purpose in the build Directory
    unshift(@INC,"./");
    unshift(@INC,"./scripts/");
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

use Geo::Gpsdrive::DBFuncs;
use Geo::Gpsdrive::Utils;
use Geo::Gpsdrive::Gps;

use Geo::Gpsdrive::DB_Defaults;
use Geo::Gpsdrive::DB_Examples;

use Geo::Gpsdrive::GpsDrive;
use Geo::Gpsdrive::JiGLE;
use Geo::Gpsdrive::Kismet;
use Geo::Gpsdrive::NGA;
use Geo::Gpsdrive::OSM;
use Geo::Gpsdrive::OpenGeoDB;
use Geo::Gpsdrive::OpenGeoDB2;
use Geo::Gpsdrive::PocketGpsPoi;
use Geo::Gpsdrive::WDB;
use Geo::Gpsdrive::Way_Txt;
use Geo::Gpsdrive::census;
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

my $do_census            = 0;
my $do_earthinfo_nga_mil = undef;
my $do_osm               = undef;
my $do_opengeodb         = 0;
my $do_opengeodb2        = 0;
my $do_wdb               = 0;
my $do_mapsource_points  = 0; 
my $do_cameras           = 0;
my $do_all               = 0;
my $do_create_db         = 0;
my $do_gpsdrive_tracks   = 0;
my $do_kismet_tracks     = 0;
my $do_jigle             = 0;
my $do_import_defaults   = 0;
my $do_import_examples   = 0;
my $do_import_way_txt    = 0;
my $do_traffic;
my $show_traffic;
our $do_delete_db_content = 0;
our $do_collect_init_data = 0;
our $street;
our $ort;
our $plz;
our $sql;
our $file;
my $do_generate_poi_type_html_page = 0;


our ($lat_min,$lat_max,$lon_min,$lon_max) = (0,0,0,0);

our $db_user             = 'gast';
our $db_password         = 'gast';

# Set defaults and get options from command line
Getopt::Long::Configure('no_ignore_case');
pod2usage(1)
    unless @ARGV;
GetOptions ( 
	     'create-db'           => \$do_create_db,
	     'fill-defaults'       => \$do_import_defaults,
	     'fill-examples'       => \$do_import_examples,
	     'census'              => \$do_census,
	     'earthinfo_nga_mil:s' => \$do_earthinfo_nga_mil,
	     'openstreetmap:s'     => \$do_osm,
	     'osm:s'               => \$do_osm,
	     'opengeodb'           => \$do_opengeodb,
	     'opengeodb2'          => \$do_opengeodb2,
	     'wdb:s'               => \$do_wdb,
	     'mapsource_points=s'  => \$do_mapsource_points,
	     'cameras'             => \$do_cameras,
	     'gpsdrive-tracks'     => \$do_gpsdrive_tracks,
	     'kismet-tracks=s'     => \$do_kismet_tracks,
	     'jigle=s'     	   => \$do_jigle,
	     'import-way-txt'      => \$do_import_way_txt,
	     'all'                 => \$do_all,
	     'debug'               => \$debug,      
	     'u=s'                 => \$db_user,
	     'p=s'                 => \$db_password,
	     'db-name=s'           => \$GPSDRIVE_DB_NAME,
	     'db-user=s'           => \$db_user,
	     'db-password=s'       => \$db_password,
	     'delete-db-content'   => \$do_delete_db_content,
	     'collect-init-data'   => \$do_collect_init_data,
	     'generate-poi-type-html-page' => \$do_generate_poi_type_html_page,
	     'lat_min=s'           => \$lat_min,      
	     'lat_max=s'           => \$lat_max,
	     'lon_min=s'           => \$lon_min,      
	     'lon_max=s'           => \$lon_max,
	     'lat-min=s'           => \$lat_min,      
	     'lat-max=s'           => \$lat_max,
	     'lon-min=s'           => \$lon_min,      
	     'lon-max=s'           => \$lon_max,
	     'no-delete'           => \$no_delete,
	     'd'                   => \$debug,      
	     'verbose+'            => \$verbose,
	     'debug_range=s'       => \$debug_range,      
	     'no-mirror'           => \$no_mirror,
	     'proxy=s'             => \$PROXY,
	     'MAN'                 => \$man, 
	     'man'                 => \$man, 
	     'street=s'		   => \$street, #need for getstreet
	     'ort=s'		   => \$ort,
	     'plz=s'		   => \$plz,
	     'sql'		   => \$sql,
	     'file'		   => \$file,
	     'get-traffic'	   => \$do_traffic,
	     'show-traffic'	   => \$show_traffic,
	     'h|help|x'            => \$help, 
	     )
    or pod2usage(1);


if ( $do_all ) {
    $do_create_db
	= $do_census
	= $do_opengeodb
	= $do_opengeodb2
	= $do_osm
	= $do_gpsdrive_tracks
	= $do_cameras
	= $do_jigle
	= $do_import_defaults
	= $do_import_examples
	= 1;
    if ( $ENV{'LANG'} =~ /de_DE/ ) {
	print "\n";
	print "=============================================================================\n";
	print "I assume i'm in Germany (LANG='$ENV{'LANG'}')\n";

	$do_earthinfo_nga_mil ||= 'gm,sz,be,au,bu,ez,da,fi,fr,gr,hu,lu,mt,mn';
	print "    --> loading only $do_earthinfo_nga_mil for NGA\n";

	$do_wdb ||="europe";
	print "    --> loading only $do_wdb for WDB\n";

	print "=============================================================================\n";
	print "\n";
    } else {
	$do_earthinfo_nga_mil = 1;
	$do_wdb =1;
    };
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

Geo::Gpsdrive::getstreet::streets()
	if $street;


Geo::Gpsdrive::gettraffic::gettraffic()
	if $do_traffic;

Geo::Gpsdrive::gettraffic::showtraffic()
	if $show_traffic;


Geo::Gpsdrive::DBFuncs::create_db()
    if $do_create_db;

Geo::Gpsdrive::DB_Defaults::fill_defaults()
    if $do_import_defaults;

Geo::Gpsdrive::DB_Examples::fill_examples()
    if $do_import_examples;

Geo::Gpsdrive::DB_Defaults::generate_poi_type_html_page()
    if $do_generate_poi_type_html_page;

# Convert MapSource Waypoints to gpsdrive POI
Geo::Gpsdrive::mapsource::import_Data()
    if ( $do_mapsource_points );

# Get and Unpack earth-info.nga.mil     http://earth-info.nga.mil/gns/html/cntyfile/gm.zip
Geo::Gpsdrive::NGA::import_Data($do_earthinfo_nga_mil)
    if ( defined $do_earthinfo_nga_mil );


# Get and Unpack opengeodb  http://www.opengeodb.de/download/
Geo::Gpsdrive::OpenGeoDB::import_Data() 
    if ( $do_opengeodb );

# Get and Unpack opengeodb2  http://www.opengeodb.de/download/
Geo::Gpsdrive::OpenGeoDB2::import_Data() 
    if ( $do_opengeodb2 );

# Get and Unpack openstreetmap  http://www.openstreetmap.org/
Geo::Gpsdrive::OSM::import_Data($do_osm) 
    if ( defined $do_osm );

# Get and Unpack wdb  http://www.evl.uic.edu/pape/data/WDB/WDB-text.tar.gz
Geo::Gpsdrive::WDB::import_Data($do_wdb)
    if ( $do_wdb );

# Get and Unpack Census Data        http://www.census.gov/geo/cob/bdy/
Geo::Gpsdrive::census::import_Data()
    if ( $do_census );

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

B<geoinfo.pl> Version 0.00001

=head1 DESCRIPTION

B<geoinfo.pl> is a program to download and convert waypoints 
and other geospacial data for use with the new gpsdrive 
POI Database. 
You need to have mySQL Support.

This Programm is completely experimental, but some Data 
can already be retrieved with it.

So: Have Fun, improve it and send me fixes :-))

WARNING: 
    This programm replaces some/all waypoints of desire. 
    So any changes made to the database may be overwritten!!!
    If you have any self collected/changed Data do a backup first!!


=head1 SYNOPSIS

B<Common usages:>

geoinfo.pl [-d] [-v] [-h] [-earthinfo_nga_mil] [--opengeodb] [--wdb=??] [--mapsource_points='Filename']

=head1 OPTIONS

=over 2

=item B<--man> Complete documentation

Complete documentation


=item B<--create-db>

Try creating the tables inside the geoinfo database. 
This also fills the database with some predefined types.
and imports wour way*.txt Files.
This also creates and modified the old waypoints table.


=item B<--fill-defaults>

Fill the Databases with usefull defaults. This option is 
needed before you can import any of the other importers.


=item B<--earthinfo_nga_mil=xx[,yy][,zz]...>

Download from earthinfo.nga.mil and import into 
mysql DB. 
These are ~2.500.000 City Names around the world delivered in
249 single Files.


Download Country File from
 http://earth-info.nga.mil/gns/html/

where xx is one or more countries. 
 poi.pl -earthinfo_nga_mil=??
 Gives you a complete list of allowed county tupels.

For more info on Countries have a look at
 http://earth-info.nga.mil/gns/html/cntry_files.html

The complete download is about ~900 MB


=item B<--openstreetmap> B<--osm>


Download and import openstreetmap Data. 
If no parameter is given. planet.osm is downloaded and then imported. 
If a filename is given this Filename is imported.


=item B<--opengeodb>

Download and import opengeodb to Point of interrests

This Database has about 20003 entries from German Towns

opengeodb-0.1.3-txt.tar.gz has 442K


=item B<--opengeodb2>

Download and import opengeodb-2.4a

This Database has about 20003 entries from German Towns

In the current release only the sql File is downloaded and
executed. This imports the Data from opengeodb into a 
Database named opengeodb. This Database is not yet used in the 
current gpsdrive Version.

Download size opengeodb-0.2.4a-UTF8-mysql.zip 1.8MB


=item B<--wdb=??>

CIA World DataBank II

Download and import WDB Data into geoinfo.streets Table

These data consists of Country Borders and Waterlines

Download is ~30 MB
Unpacking: ~119 MB

At the moment it only import europe Data for testing.
Available regions would be: 
	africa  asia  europe 
	namer(North America) samer(South America)



=item B<--mapsource_points='Filename'>

******** NO function yet *********


=item B<--cameras>

******** NO function yet *********




=item B<--fill-examples>

Fill the Databases with usefull examples. This is usefull for 
testing without a large Database included.


=item B<--all>

Triggers all of the above


=item B<--collect-init-data>

Collects default data and writes them into the default Files.
This option is normally used by the maintainer to create the 
Defaults for filling the DB.


=item B<--generate-poi-type-html-page>

Generate a html page containing all POI Icons created as default
and there descriptions.

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


=item B<--no-mirror>

Do not try mirroring the files from the original Server. Only use
files found on local Filesystem.


=item B<--proxy>

use proxy for download

=back
