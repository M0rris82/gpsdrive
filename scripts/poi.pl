#!/usr/bin/perl
# Handling of POI Data:
# For now:
# Retrieve POI Data from Different Sources
# And import them into mySQL for use with gpsdrive
#
# $Log$
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
use POI::Gps;
use HTTP::Request;
use IO::File;
use Pod::Usage;

use POI::DBFuncs;
use POI::NGA;
use POI::OpenGeoDB;
use POI::WDB;
use POI::PocketGpsPoi;
use POI::Utils;
use POI::census;
use POI::GpsDrive;
use POI::Kismet;
use POI::JiGLE;

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
	     'create-db'           => \$do_create_db,
	     'all'                 => \$do_all,
	     'debug'               => \$debug,      
	     'u=s'                 => \$db_user,
	     'p=s'                 => \$db_password,
	     'db-user=s'           => \$db_user,
	     'db-password=s'       => \$db_password,
	     'lat_min=s'           => \$lat_min,      
	     'lat_max=s'           => \$lat_max,
	     'lon_min=s'           => \$lon_min,      
	     'lon_max=s'           => \$lon_max,
	     'd'                   => \$debug,      
	     'v'                   => \$verbose,      
	     'debug_range=s'       => \$debug_range,      
	     'no-mirror'           => \$no_mirror,
	     'proxy=s'             => \$PROXY,
	     'MAN'                 => \$man, 
	     'help|x'              => \$help, 
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


POI::DBFuncs::create_db()
    if ( $do_create_db );

# Get and Unpack Census Data        http://www.census.gov/geo/cob/bdy/
POI::census::import_Data()
    if ( $do_census );

# Get and Unpack POCKETGPS_DIR      http://www.pocketgpspoi.com
POI::PocketGpsPoi::import_Data()
    if ( $do_cameras );

# Convert MapSource Waypoints to gpsdrive POI
POI::mapsource::import_Data()
    if ( $do_mapsource_points );


# Get and Unpack earth-info.nga.mil     http://earth-info.nga.mil/gns/html/cntyfile/gm.zip
POI::NGA::import_Data($do_earthinfo_nga_mil)
    if ( $do_earthinfo_nga_mil );


# Get and Unpack opengeodb  http://www.opengeodb.de/download/
POI::OpenGeoDB::import_Data() 
    if ( $do_opengeodb );

# Get and Unpack wdb  http://www.evl.uic.edu/pape/data/WDB/WDB-text.tar.gz
POI::WDB::import_Data() 
    if ( $do_wdb );

# extract street Data from all tracks
POI::GpsDrive::import_Data() 
    if ( $do_gpsdrive_tracks );

# extract street Data from all tracks
POI::Kismet::import_Data($do_kismet_tracks) 
    if ( $do_kismet_tracks );

# extract WLAN Points from JiGLE Data
POI::JiGLE::import_Data($do_jigle) 
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

=item B<--mapsource_points='Filename'>

******** NO function yet *********

=item B<--cameras>

******** NO function yet *********

=item B<--create-db>

Try creating the tables inside the geoinfo database. 
This also fills the database with some predefined types.
and imports wour way*.txt Files.
This also creates and modified the old waypoints table.

=item B<--all>


Triggers all of the above


=item B<--gpsdrive-tracks>

Read all gpsdrive Tracks 
and insert into  streets DB


=item B<--kismet-tracks=Directory>

Read all Kismet .gps Files in directory, extract the Tracks
and insert them into streets DB


=item B<--jigle=Directory>

Read all jigle Files in directory, extract the Wavelan points
and insert them into POI DB

See http://www.wigle.net/


=item B<--lat_min --lat_max --lon_min --lon_max>

For debug reasons for some inserts limit to the given rectangle




=item B<--db-user>

username to connect to mySQL database

=item B<--db-password>

password for user to connect to mySQL database

=item B<--no-mirror>

do not try mirroring the files from the original Server

=item B<--proxy>

use proxy for download
