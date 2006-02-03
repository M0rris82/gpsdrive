# Database Defaults for poi/streets Table for poi.pl
#
# $Log$
# Revision 1.3  2006/02/03 18:31:58  tweety
# Update ASCII Form of Street/POI Defaults. Some minor Bugs fixing.
# More Street Examples
#
# Revision 1.2  2006/02/03 12:03:31  tweety
# move the Examples to Text Files for better Code visibility
# and easier editing the Examples Data by others
#
# Revision 1.1  2005/10/11 08:28:35  tweety
# gpsdrive:
# - add Tracks(MySql) displaying
# - reindent files modified
# - Fix setting of Color for Grid
# - poi Text is different in size depending on Number of POIs shown on
#   screen
#
# geoinfo:
#  - get Proxy settings from Environment
#  - create tracks Table in Database and fill it
#    this separates Street Data from Track Data
#  - make geoinfo.pl download also Opengeodb Version 2
#  - add some poi-types
#  - Split off Filling DB with example Data
#  - extract some more Funtionality to Procedures
#  - Add some Example POI for Kirchheim(Munich) Area
#  - Adjust some Output for what is done at the moment
#  - Add more delayed index generations 'disable/enable key'
#  - If LANG=*de_DE* then only impert europe with --all option
#  - WDB will import more than one country if you wish
#  - add more things to be done with the --all option
#
# Revision 1.1  2005/08/15 13:54:22  tweety
# move scripts/POI --> scripts/Geo/Gpsdrive to reflect final Structure and make debugging easier
#
# Revision 1.4  2005/08/09 01:08:30  tweety
# Twist and bend in the Makefiles to install the DataDirectory more apropriate
# move the perl Functions to Geo::Gpsdrive in /usr/share/perl5/Geo/Gpsdrive/POI
# adapt icons.txt loading according to these directories
#
# Revision 1.3  2005/05/24 08:35:25  tweety
# move track splitting to its own function +sub track_add($)
# a little bit more error handling
# earth_distance somtimes had complex inumbers as result
# implemented streets_check_if_moved_reset which is called when you toggle the draw streets button
# this way i can re-read all currently displayed streets from the DB
# fix minor array iindex counting bugs
# add some content to the comment column
#
# Revision 1.2  2005/05/14 21:21:23  tweety
# Update Index createion
# Update default Streets
# Eliminate some undefined Value
#
# Revision 1.1  2005/05/09 19:35:12  tweety
# Split Default Values into seperate File
# Add new Icon
#

package Geo::Gpsdrive::DB_Examples;

use strict;
use warnings;

use POSIX qw(strftime);
use Time::Local;
use DBI;
use Data::Dumper;
use IO::File;

use Geo::Gpsdrive::DBFuncs;
use Geo::Gpsdrive::Utils;

my $example_source_name = "Examples";
my $example_source_id=0;

$|= 1;                          # Autoflush

BEGIN {
    use Exporter   ();
    our ($VERSION, @ISA, @EXPORT, @EXPORT_OK, %EXPORT_TAGS);

    # set the version for version checking
    $VERSION     = 1.00;
    # if using RCS/CVS, this may be preferred
    $VERSION = sprintf "%d.%03d", q$Revision$ =~ /(\d+)/g;

    @ISA         = qw(Exporter);
    @EXPORT = qw( );
    %EXPORT_TAGS = ( );
    @EXPORT_OK   = qw();

}
#our @EXPORT_OK;

# -----------------------------------------------------------------------------
# Open Data File in predefined Directories
sub data_open($){
    my $file_name = shift;
    for my $path ( qw(/usr/local/share/gpsdrive/
		      /usr/share/gpsdrive/
		      ../data/
		      data/
		      ) ) {
	my $file_with_path=$path.$file_name;
	if ( -s $file_with_path ) {
	    debug("Opening $file_with_path");
	    my $fh = IO::File->new("<$file_with_path")
		or die("cannot open $file_with_path: $!");;
	    return $fh;
	}
    }
    die "cannot Find $file_name";
}
	
# -----------------------------------------------------------------------------
# Insert Example Waypoints
sub fill_example_waypoints($) {
    my $file_name = shift;
    my $fh = data_open($file_name);

    die "No Example Source ID defined\n" unless $example_source_id;

    my $error=0;
    while ( my $line = $fh->getline() ) {
	chomp $line;
	my ( $name, $lat, $lon, $type_name ) = split(/\s+/,$line);
	
	debug("\n\nInsert Waypoint '$name' '$lat','$lon' '$type_name'");

	die "Fehler in lat,lon: '$lat','$lon' for $name,'$type_name'\n"
	    unless ( $lat =~ m/^\s*\d+\.\d+\s*$/  &&
		     $lon =~ m/^\s*\d+\.\d+\s*$/ );

	$type_name =~ s/_/ /g;
	my $type_id = poi_type_name2id($type_name);
	unless ( $type_id ){
	    $error++;
	    die "--------------- Type for '$type_name' not found \n";
	    next;
	}
	
	#for  my $t ( qw(waypoints poi)) {
	{
	    my $t = 'poi';
	    my $loc  = { "$t.name" => $name, "$t.lat" => $lat, "$t.lon" => $lon};
	    my $wp_defaults = { "$t.wep"         => 0 ,
				"$t.nettype"     => '',
				"$t.scale_min"   => 1,
				"$t.scale_max"   => 1000000000,
				"$t.source_id"   => $example_source_id,
				"$t.poi_type_id" => $type_id,
				"$t.last_modified" => time(),
			    };
	    #print "Sample WP:$t	'$name'\n";
	    Geo::Gpsdrive::DBFuncs::insert_hash($t, $wp_defaults, $loc );
	}
    }
}

# ------------------------------------------------------------------
sub fill_example_cities($){
    my $file_name = shift;
    my $fh = data_open($file_name);

    # Insert Example Cities
    my $type_name = "area.city";
    my $type_id = poi_type_name2id($type_name);
    die "Type for '$type_name' not found \n" unless $type_id;

    die "No Example Source ID defined\n" unless $example_source_id;

#    for  my $t ( qw(waypoints poi)) {
    {
	my $t = 'poi';
	my $wp_defaults = { "$t.wep"         => 0 ,
			    "$t.nettype"     => '',
			    "$t.scale_min"   => 1,
			    "$t.scale_max"   => 1000000000,
			    "$t.type"        => 'City',
			    "$t.source_id"   => $example_source_id,
			    "$t.poi_type_id" => $type_id,
			    "$t.last_modified" => time(),
			};
	my $type_query = $t  eq "poi" 
	    ? " poi_type_id = '$type_id'" 
	    : " type = '". $wp_defaults->{"$t.type"} ."'";
	
	while ( my $line = $fh->getline() ) {
	    chomp $line;
	    my ( $lat, $lon, $name ) = split(/\s+/,$line);
	    my $loc;
	    $loc->{"$t.name"} = $loc->{name} = $name;
	    $loc->{"$t.lat"}  = $loc->{lat}  = $lat;
	    $loc->{"$t.lon"}  = $loc->{lon}  = $lon;

	    #my $delete_query=sprintf("DELETE FROM $t ".
	    #"WHERE name = '%s' AND $type_query",
	    #$loc->{"$t.name"});
	    #Geo::Gpsdrive::DBFuncs::db_exec( $delete_query);
	    Geo::Gpsdrive::DBFuncs::insert_hash($t, $wp_defaults, $loc );
	}
    }
} # of fill_example_cities()



# -----------------------------------------------------------------------------
sub fill_example_streets($) { # Insert Street Sample
    my $file_name = shift;
    my $fh = data_open($file_name);
    my $street_name='';
    my $multi_segment={};
    while ( 1 ) {
	my $line = $fh->getline();
	chomp $line;
	if ( $line =~ m/^\S+/ || $fh->eof() ) {
	    if ( $street_name ) { # Es ist eine Strasse gespeichert
		print Dumper($multi_segment);
		debug("Importing Street: $street_name");
		# street_segments_add_from_segment_array($multi_segment);
		street_segments_add($multi_segment);

	    }

	    last 
		if $fh->eof();
	    
	    # Neue Strasse anfangen
	    $street_name = $line;
	    $multi_segment={};
	    $multi_segment->{'source_id'} = $example_source_id;
	    $multi_segment->{'streets_type_id'} = '';
	    if ( $street_name =~ m/^\#/ ) { # Komments
		next;
	    } elsif ( $street_name =~ m/^A/ ) {
		$multi_segment->{'streets_type_id'} = streets_type_name2id('Strassen.Autobahn');
	    } elsif ( $street_name =~ m/^ST/ ) {
		$multi_segment->{'streets_type_id'} = streets_type_name2id('Strassen.Bundesstrasse');
	    } elsif ( $street_name =~ m/^B/ ) {
		$multi_segment->{'streets_type_id'} = streets_type_name2id('Strassen.Bundesstrasse');
	    }   else {
		$multi_segment->{'streets_type_id'} = streets_type_name2id('Strassen.Allgemein');
	    }
	    
	    $multi_segment->{'name'}            = $street_name;
	    $multi_segment->{'scale_min'}       = 1;
	    $multi_segment->{'scale_max'}       = 50000000;
	    
	} elsif ( $line =~ m/^\s*$/ ) { # Empty Line
	    next;
	} else {
#	} elsif ( $line =~ m/^[\t\s]+[\d\+\-\.\,]+[\t\s]+[\d\+\-\.\,]+[\t\s]+$/) {
	    my ($indent,$lat,$lon) = split(/\s+/,$line);
	    push(@{$multi_segment->{'segments'}},[$lat,$lon]);
#	} else {
#	    warn "Error in File: $file_name Line: '$line'";
	}
    }
}; # of fill_example_streets()

# ------------------------------------------------------------------
sub fill_examples(){

    $example_source_id = source_name2id($example_source_name);
    die "Unknown Source ID: $example_source_name\n" unless $example_source_id;

    delete_all_from_source($example_source_name);

    print "\nCreate Examples ...(source_id=$example_source_id)\n";

    fill_example_cities("poi/cities.txt");
    fill_example_waypoints("poi/germany.txt");
    fill_example_streets("streets/Autobahnen.txt");
    fill_example_streets("streets/Streets.txt");
#    fill_example_streets("streets/Streets1.txt");
    print "Create Examples completed\n";
}


1;
