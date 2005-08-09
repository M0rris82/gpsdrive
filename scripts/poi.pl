#!/usr/bin/perl
# Handling of POI Data:
# For now:
# Retrieve POI Data from Different Sources
# And import them into mySQL for use with gpsdrive
#
# $Log$
# Revision 1.14  2005/08/09 01:08:30  tweety
# Twist and bend in the Makefiles to install the DataDirectory more apropriate
# move the perl Functions to Geo::Gpsdrive::POI in /usr/share/perl5/Geo/Gpsdrive/POI
# adapt icons.txt loading according to these directories
#
# Revision 1.13  2005/05/24 08:35:25  tweety
# move track splitting to its own function +sub track_add($)
# a little bit more error handling
# earth_distance somtimes had complex inumbers as result
# implemented streets_check_if_moved_reset which is called when you toggle the draw streets button
# this way i can re-read all currently displayed streets from the DB
# fix minor array iindex counting bugs
# add some content to the comment column
#
# Revision 1.12  2005/05/14 21:21:23  tweety
# Update Index createion
# Update default Streets
# Eliminate some undefined Value
#
# Revision 1.11  2005/05/01 13:49:36  tweety
# Added more Icons
# Moved filling with defaults to DB_Defaults.pm
# Added some more default POI Types
# Added icons.html to see which icons are used
# Added more Comments
# Reformating Makefiles
# Added new options for importing from way*.txt and adding defaults
# Added more source_id and type_id
#
# Revision 1.10  2005/04/07 06:20:45  tweety
# allow lat-min also intead of lat_min for poi.pl as parameter
# add POI::DBFuncs::enable_keys to WDB.pm for speed improovement
# change to extern MYSQL mysql; in poi.c and streets.c
# handle errors for g_renewi
# start for usage of streets_type_list[streets_type_list_count].color from streets_type
#
# Revision 1.9  2005/04/06 06:39:10  tweety
# po.pl get new import method JiGLE for importing JiGLE WLAN Spots
# update TODO
#
# Revision 1.8  2005/02/22 08:18:51  tweety
# change leveing system to simpler scale marking for decission what to show on display
# column_names(DBFuncs.pm get data from Database
# added functions add_index drop_index
# added language to type Database
# for some Data split unpack and mirror Directories
# for some add lat/lon min/max to get faster import for testing
# added POI::DBFuncs::segments_add; this will later be the point to do some excerptions and combinations
# on the street data
#
# Revision 1.7  2005/02/18 08:19:39  tweety
# added reading routine for kismet street data
#
# Revision 1.6  2005/02/17 22:15:58  tweety
# added import from gpsdrive tracks to Streets DB
#
# Revision 1.5  2005/02/17 09:31:16  tweety
# minor changes
#
# Revision 1.4  2005/02/13 22:57:00  tweety
# WDB Support
#
# Revision 1.3  2005/02/07 19:05:58  tweety
# splitpoi.pl intomodules
#
# Revision 1.2  2005/02/06 21:41:47  tweety
# added some checks to DB Creation
#
# Revision 1.1  2005/02/06 00:22:14  tweety
# poi.pl to mirror and fill POI Database with Names.
# This is a early Alpha version
#
# Revision 1.0  2005/01/12 22:08:13  tweety

my $VERSION ="poi.pl (c) Joerg Ostertag
Initial Version (Jan,2005) by Joerg Ostertag <joerg.ostertag\@rechengilde.de>
Version 1.17
";


BEGIN {
    unshift(@INC,"./");
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

use Geo::Gpsdrive::POI::DBFuncs;
use Geo::Gpsdrive::POI::DB_Defaults;
use Geo::Gpsdrive::POI::Gps;
use Geo::Gpsdrive::POI::GpsDrive;
use Geo::Gpsdrive::POI::JiGLE;
use Geo::Gpsdrive::POI::Kismet;
use Geo::Gpsdrive::POI::NGA;
use Geo::Gpsdrive::POI::OpenGeoDB;
use Geo::Gpsdrive::POI::PocketGpsPoi;
use Geo::Gpsdrive::POI::Utils;
use Geo::Gpsdrive::POI::WDB;
use Geo::Gpsdrive::POI::Way_Txt;
use Geo::Gpsdrive::POI::census;

my ($man,$help);

our $CONFIG_DIR    = "$ENV{'HOME'}/.gpsdrive"; # Should we allow config of this?
our $CONFIG_FILE   = "$CONFIG_DIR/gpsdriverc";
our $MIRROR_DIR   = "$CONFIG_DIR/MIRROR";
our $UNPACK_DIR   = "$CONFIG_DIR/UNPACK";

my $do_census            = 0;
my $do_earthinfo_nga_mil = 0;
my $do_opengeodb         = 0;
my $do_wdb               = 0;
my $do_mapsource_points  = 0; 
my $do_cameras           = 0;
my $do_all               = 0;
my $do_create_db         = 0;
my $do_gpsdrive_tracks   = 0;
my $do_kismet_tracks     = 0;
my $do_jigle             = 0;
my $do_import_defaults   = 0;
my $do_import_way_txt    = 0;
our $do_delete_db_content = 0;
our $do_collect_init_data = 0;

our ($lat_min,$lat_max,$lon_min,$lon_max) = (0,0,0,0);

our $db_user             = 'gast';
our $db_password         = 'gast';

# Set defaults and get options from command line
Getopt::Long::Configure('no_ignore_case');
pod2usage(1)
    unless @ARGV;
GetOptions ( 
	     'census'              => \$do_census,
	     'earthinfo_nga_mil=s' => \$do_earthinfo_nga_mil,
	     'opengeodb'           => \$do_opengeodb,
	     'wdb'                 => \$do_wdb,
	     'mapsource_points=s'  => \$do_mapsource_points,
	     'cameras'             => \$do_cameras,
	     'gpsdrive-tracks'     => \$do_gpsdrive_tracks,
	     'kismet-tracks=s'     => \$do_kismet_tracks,
	     'jigle=s'     	   => \$do_jigle,
	     'import-way-txt'      => \$do_import_way_txt,
	     'create-db'           => \$do_create_db,
	     'fill-defaults'       => \$do_import_defaults,
	     'all'                 => \$do_all,
	     'debug'               => \$debug,      
	     'u=s'                 => \$db_user,
	     'p=s'                 => \$db_password,
	     'db-user=s'           => \$db_user,
	     'db-password=s'       => \$db_password,
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
	     'd'                   => \$debug,      
	     'v'                   => \$verbose,      
	     'debug_range=s'       => \$debug_range,      
	     'no-mirror'           => \$no_mirror,
	     'proxy=s'             => \$PROXY,
	     'MAN'                 => \$man, 
	     'man'                 => \$man, 
	     'h|help|x'            => \$help, 
	     )
    or pod2usage(1);

if ( $do_all ) {
    $do_create_db
	= $do_census
	= $do_earthinfo_nga_mil
	= $do_opengeodb
	= $do_wdb
	= $do_gpsdrive_tracks
	= $do_cameras
	= $do_jigle
	= $do_import_defaults
	= 1;
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


Geo::Gpsdrive::POI::DBFuncs::create_db()
    if $do_create_db;

Geo::Gpsdrive::POI::DB_Defaults::defaults()
    if $do_import_defaults;

Geo::Gpsdrive::POI::Way_Txt::import_Data()
    if $do_import_way_txt;


# Get and Unpack Census Data        http://www.census.gov/geo/cob/bdy/
Geo::Gpsdrive::POI::census::import_Data()
    if ( $do_census );

# Get and Unpack POCKETGPS_DIR      http://www.pocketgpspoi.com
Geo::Gpsdrive::POI::PocketGpsPoi::import_Data()
    if ( $do_cameras );

# Convert MapSource Waypoints to gpsdrive POI
Geo::Gpsdrive::POI::mapsource::import_Data()
    if ( $do_mapsource_points );


# Get and Unpack earth-info.nga.mil     http://earth-info.nga.mil/gns/html/cntyfile/gm.zip
Geo::Gpsdrive::POI::NGA::import_Data($do_earthinfo_nga_mil)
    if ( $do_earthinfo_nga_mil );


# Get and Unpack opengeodb  http://www.opengeodb.de/download/
Geo::Gpsdrive::POI::OpenGeoDB::import_Data() 
    if ( $do_opengeodb );

# Get and Unpack wdb  http://www.evl.uic.edu/pape/data/WDB/WDB-text.tar.gz
Geo::Gpsdrive::POI::WDB::import_Data() 
    if ( $do_wdb );

# extract street Data from all tracks
Geo::Gpsdrive::POI::GpsDrive::import_Data() 
    if ( $do_gpsdrive_tracks );

# extract street Data from all tracks
Geo::Gpsdrive::POI::Kismet::import_Data($do_kismet_tracks) 
    if ( $do_kismet_tracks );

# extract WLAN Points from JiGLE Data
Geo::Gpsdrive::POI::JiGLE::import_Data($do_jigle) 
    if ( $do_jigle );


__END__

=head1 NAME

B<convert_wp> Version 0.000001

=head1 DESCRIPTION

B<poi.pl> is a program to download and convert waypoints 
for use with the new gpsdrive POI Database. 
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

poi.pl [-d] [-v] [-h] [-earthinfo_nga_mil] [--opengeodb] [--wdb] [--mapsource_points='Filename']

=head1 OPTIONS

=over 8

=item B<--earthinfo_nga_mil=xx[,yy][,zz]...>

Download from earthinfo.nga.mil and import into 
mysql DB. These are ~2.500.000 City Names arround 
the world


Download Country File from
 http://earth-info.nga.mil/gns/html/

where xx is one or more countries. 
 poi.pl -earthinfo_nga_mil=??
 Gives you a complete list of allowed county tupels.

For more info on Countries have a look at
 http://earth-info.nga.mil/gns/html/cntry_files.html


The complete download is about ~180 MB


=item B<--opengeodb>

Download and import opengeodb to Point of interrests

This Database has about 20003 entries from German Towns


=item B<--wdb>

World Database

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

=item B<--create-db>

Try creating the tables inside the geoinfo database. 
This also fills the database with some predefined types.
and imports wour way*.txt Files.
This also creates and modified the old waypoints table.

=item B<--fill-defaults>

Fill the Databases with usefull defaults. This option is 
needed before you can import any of the other importers.

=item B<--collect-init-data>

Collects default data and writes them into the default Files.
This option is normally used by the maintainer to create the 
Defaults for filling the DB.

=item B<--all>


Triggers all of the above


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


=item B<--lat_min --lat_max --lon_min --lon_max>

For example for debug reasons for some inserts limit to the 
given rectangle.
This feature is not implemented on all insert statements yet.


=item B<--db-user>

username to connect to mySQL database

=item B<--db-password>

password for user to connect to mySQL database

=item B<--no-mirror>

do not try mirroring the files from the original Server

=item B<--proxy>

use proxy for download
