# Einlesen der GpsDrive Track Daten und schreiben in die geoinfo Datenbank von 
# gpsdrive

package POI::GpsDrive;

use strict;
use warnings;

use IO::File;
use File::Basename;
use File::Path;
use Data::Dumper;

use POI::DBFuncs;
use POI::Utils;
use POI::Gps;

use Date::Manip;
use Time::Local;

$|=1;

##########################################################################

sub import_GpsDrive_track_file($$){
    my  $full_filename = shift;
    my $source = shift;
    print "Reading $full_filename                   \n";

    my $fh = IO::File->new("<$full_filename");

    my ($lat1,$lon1,$alt1,$time1) = (0,0,0,0);
    my ($lat2,$lon2,$alt2,$time2) = (0,0,0,0);

    my $source_id = POI::DBFuncs::source_name2id($source);

    unless ( $source_id ) {
	my $source_hash = {
	    'source.url'     => "",
	    'source.name'    => $source ,
	    'source.comment' => 'My own Tracks' ,
	    'source.licence' => "It's up to myself"
	    };
	POI::DBFuncs::insert_hash("source", $source_hash);
	$source_id = POI::DBFuncs::source_name2id($source);
    }
    

    my $street_nr=0;
    my $segments_in_street=0;
    while ( my $line = $fh->getline() ) {
	my $valid=0;
	my $streets_type_id = 2;

	chomp $line;
#	print "line: $line\n";
	#48.175667  11.754383        561 Tue May 18 18:28:04 2004

	( $lat1,$lon1,$alt1,$time1 ) = ( $lat2,$lon2,$alt2,$time2 );

	( $lat2,$lon2,$alt2,$time2 ) = (0,0,0,0);

        if ($line =~ m/^\s*(\d{1,3}\.\d+)\s+(\d{1,3}\.\d+)\s+([\d\.]+)\s+(\S+\s+\S+\s+\d+\s+[\d\:]+\s+\d+)/ ) {
            $lat2  = $1;
            $lon2  = $2;
            $alt2  = $3;
            my $date = ParseDate($4);
	    #       Wed Dec 10 09:38:24 2003
	    $time2 = UnixDate($date,"%s");
	    $valid=1;
	} elsif ( $line =~ m/^\s*$/)  {
        } elsif ( $line =~ m/^\s*nan\s*nan\s*/)  {
        } elsif ( $line =~ m/^\s*1001.000000 1001.000000\s*/)  {           
        } else {
            print "Unparsed Line '$line'";
        }

	next unless $valid;


	my $dist = POI::Gps::earth_distance($lat1,$lon1,$lat2,$lon2);;
	my $time_delta = $time2 - $time1;
	my $speed      = $valid&&$time_delta ? $dist / $time_delta * 3.600 : -1;
	#printf "Dist: %.4f/%.2f =>  %.2f\n",$dist,$time_delta,$speed;
	$streets_type_id = 1 if ( $speed >0 );
	$streets_type_id = 2 if ( $speed >30 );
	$streets_type_id = 3 if ( $speed >60 );
	$streets_type_id = 4 if ( $speed >100 );

	if ( $alt2 == 0 ) { # Otherwise I assume it was POS Mode
	    print "Altitude = 0  ---> type=10\n";
	    print "Line: $line\n";
	    $streets_type_id = 10;
	}

	if ( $time_delta >300 ) {
	    print "Time diff = $time_delta\n";
	    $valid = 0;
	}

	if ( $speed >400 ) {
	    print "Speed = $speed\n";
	    print "But ignoring, because time accuracy =1 sec ==> speed not accurate\n";
	    $valid = 0;
	}

	if ( $dist > 200  ) {
	    printf "earth_distance($lat1,$lon1,$lat2,$lon2) => %.2f\n",$dist;
	    $valid = 0;
	}

	if ( $lat2 > 1000 ||
	     $lon2 > 1000 
	     ) {
	    print "lat/lon >1000\n";
	    $valid = 0;
	}
	
	if ( $valid ) {
	    POI::DBFuncs::streets_add(
				      { lat1 => $lat1, lon1 => $lon1, alt1 => $alt1,
					lat2 => $lat2, lon2 => $lon2, alt2 => $alt2,
					level_min => 0, level_max => 99,
					streets_type_id => $streets_type_id, 
					name => "$dist $full_filename",
					source_id => $source_id
					}
				      );
	      $segments_in_street++;
	  } else {
	      if ( $segments_in_street ) {
		  $street_nr ++;
		  print "Streets: $street_nr ($segments_in_street Segments)\n";
	      }
	      $segments_in_street=0;
	  }
    }
}




# *****************************************************************************
sub import_Data(){

    my $gpsdrive_dir = "$main::CONFIG_DIR/";
    my $source = "Gpsdrive Tracks";
    delete_all_from_source($source);
    
    debug("$gpsdrive_dir/{tracks}/*.sav");
    foreach  my $full_filename ( glob("$gpsdrive_dir/*.sav"),
				 glob("$gpsdrive_dir/tracks*.sav") ) {
	import_GpsDrive_track_file($full_filename,$source);
    }
}

1;
