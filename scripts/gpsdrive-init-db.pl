#!/usr/bin/perl
# Handling of POI/Streets Vector Data:
# For now:
# Retrieve POI Data from Different Sources
# And import them into mySQL for use with gpsdrive

# Get version number from version-control system, as integer 
   
my $Version = '$Revision: 1612 $'; 
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

    # For DSL
    unshift(@INC,"/opt/gpsdrive/share/perl5");
    unshift(@INC,"/opt/gpsdrive"); # For DSL
};

use strict;
use warnings;

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

my ($man,$help);

our $CONFIG_DIR    = "$ENV{'HOME'}/.gpsdrive"; # Should we allow config of this?
our $CONFIG_FILE   = "$CONFIG_DIR/gpsdriverc";
our $GPSDRIVE_DB_NAME = "geoinfo";

our $development_version = (`id` =~ m/tweety/);

our $db_user             = $ENV{DBUSER} || '';
our $db_password         = $ENV{DBPASS} || '';

Geo::Gpsdrive::DBFuncs::db_read_mysql_sys_pwd();

$db_user           ||= 'gast';
$db_password       ||= 'gast';

our $db_host             = $ENV{DBHOST} || 'localhost';
#$db_host = 'host=localhost;mysql_socket=/home/tweety/.gpsdrive/mysql/mysqld.socket';
my $do_show_version=0;

# Set defaults and get options from command line
Getopt::Long::Configure('no_ignore_case');
GetOptions ( 
	     'u=s'                 => \$db_user,
	     'p=s'                 => \$db_password,
	     'db-name=s'           => \$GPSDRIVE_DB_NAME,
	     'db-user=s'           => \$db_user,
	     'db-password=s'       => \$db_password,
	     'db-host=s'           => \$db_host,
	     'd+'                  => \$DEBUG,
	     'debug+'              => \$DEBUG,      
	     'verbose'            => \$VERBOSE,
	     'v+'                  => \$VERBOSE,
	     'debug_range=s'       => \$debug_range,      
	     'MAN'                 => \$man, 
	     'man'                 => \$man, 
	     'h|help|x'            => \$help, 
	     'version'             => \$do_show_version, 
	     )
    or pod2usage(1);


if ( $do_show_version ) {
    print "$VERSION\n";
};

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


Geo::Gpsdrive::DBFuncs::create_db();
Geo::Gpsdrive::DB_Defaults::fill_defaults();


__END__

=head1 NAME

B<geoinfo.pl> Version 0.9

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

gpsdrive-init-db [-d] [-v] [-h]

=head1 OPTIONS

=over 2

=item B<--man> Complete documentation

Complete documentation

Try creating the tables inside the geoinfo database. 
This also fills the database with some predefined types.
and imports wour way*.txt Files.
This also creates and modified the old waypoints table.

Fill the Databases with usefull defaults. This option is 
needed before you can import any of the other importers.

=item B<--db-name>

Name of Database to use; default is geoinfo

=item B<--db-user>

username to connect to mySQL database. Default is gast


=item B<--db-password>

password for user to connect to mySQL database. Default is gast

=item B<--db-host>

hostname for  connecting to your mySQL database. Default is localhost

=back
