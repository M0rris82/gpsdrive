#!/usr/bin/perl
# Handling of POI Data:
# For now:
# Retrieve POI Data from Different Sources
# And import them into mySQL for use with gpsdrive
#
# $Log$
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

my $VERSION ="poi.pl (c) Jörg Ostertag
Initial Version (Jan,2005) by Jörg Ostertag <joerg.ostertag\@rechengilde.de>
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
use POI::Utils;
use POI::NGA;
use POI::census;
use POI::PocketGpsPoi;

my ($man,$help);

our $CONFIG_DIR    = "$ENV{'HOME'}/.gpsdrive"; # Should we allow config of this?
our $CONFIG_FILE   = "$CONFIG_DIR/gpsdriverc";

my $do_census            = 0;
my $do_earthinfo_nga_mil = 0;
my $do_opengeodb         = 0;
my $do_mapsource_points  = 0; 
my $do_cameras           = 0;
my $do_all               = 0;
my $do_create_db         = 0;
our $db_user             = 'gast';
our $db_password         = 'gast';

# Set defaults and get options from command line
Getopt::Long::Configure('no_ignore_case');
GetOptions ( 
	     'census'              => \$do_census,
	     'earthinfo_nga_mil=s' => \$do_earthinfo_nga_mil,
	     'opengeodb'           => \$do_opengeodb,
	     'mapsource_points=s'  => \$do_mapsource_points,
	     'cameras'             => \$do_cameras,
	     'create-db'           => \$do_create_db,
	     'all'                 => \$do_all,
	     'debug'               => \$debug,      
	     'u=s'                 => \$db_user,
	     'p=s'                 => \$db_password,
	     'db-user=s'           => \$db_user,
	     'db-password=s'       => \$db_password,
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
    $do_create_db =
	$do_census = $do_earthinfo_nga_mil = $do_opengeodb 
	= $do_cameras   = 1;
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
POI::OpenGEODB::import_Data() 
    if ( $do_opengeodb ) 


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

=head1 SYNOPSIS

B<Common usages:>

poi.pl [-d] [-v] [-h] [-earthinfo_nga_mil>]

=head1 OPTIONS

=over 8

=item B<-earthinfo_nga_mil=xx[,yy][,zz]...>

Still experimental, but works partly.

Download Country File from
 http://earth-info.nga.mil/gns/html/

where xx is one or more countries. 
 poi.pl -earthinfo_nga_mil=??
Gives you a complete list of allowed county tupels.

For more info on Countries have a look at
 http://earth-info.nga.mil/gns/html/cntry_files.html

=item B<-opengeodb>

Experimental

=item B<-mapsource_points='Filename'>

Experimental

=item B<-cameras>

Experimental

=item B<-all>

Experimental

=item B<--create-db>

Try creating the tables inside the geodata database

=item B<--db-user>

username to connect to mySQL database

=item B<--db-password>

password for user to connect to mySQL database

=item B<--no-mirror>

do not try mirroring the files from the original Server

=item B<--proxy>

use proxy for download
