# Einlesen der Kismet Daten und schreiben in die geoinfo Datenbank von 
# gpsdrive
#
# $Log$
# Revision 1.4  2005/04/13 19:58:30  tweety
# renew indentation to 4 spaces + tabstop=8
#
# Revision 1.3  2005/04/10 00:15:58  tweety
# changed primary language for poi-type generation to english
# added translation for POI-types
# added some icons classifications to poi-types
# added LOG: Entry for CVS to some *.pm Files
#

package POI::Kismet;

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
sub bad_location($$){
    my $lat = shift;
    my $lon = shift;
    return 1 
	if $lat>48.17552 && $lon >11.75440 
	&& $lat<48.17579 && $lon <11.75494;
    return 0;
}

##########################################################################

sub import_Kismet_track_file($$){
    my  $full_filename = shift;
    my $source = shift;
    print "Reading $full_filename                   \n";

    my $fh = IO::File->new("<$full_filename");

    my ($lat1,$lon1,$alt1,$time1) = (1001,1001,0,0);
    my ($lat2,$lon2,$alt2,$time2) = (1001,1001,0,0);

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
    


    my $distance_street=0;
    my $distance_streets=0;
    my $street_nr=0;
    my $segments_in_street=0;
    my $line1='';
    while ( my $line = $fh->getline() ) {
	my $valid=0;
	my $streets_type_id = 2;
	my $dist;
	my $time_delta;
	my $speed;

	chomp $line;
#	print "line: $line\n";
	#48.175667  11.754383        561 Tue May 18 18:28:04 2004

        next unless ( $line =~ s/.*gps-point\s+bssid="GP:SD:TR:AC:KL:OG"\s*// );
#       print $line;

	( $lat1,$lon1,$alt1,$time1 ) = ( $lat2,$lon2,$alt2,$time2 );
	( $lat2,$lon2,$alt2,$time2 ) = (1001,1001,0,0);

        my %elem = split(/[\s=]+/,$line);
        for my $k ( keys %elem ) {
            $elem{$k} =~ s/^"(.*)"$/$1/;
        }
        
	$lat2  = $elem{lat};
	$lon2  = $elem{lon};
	$alt2  = $elem{alt};
	#       Wed Dec 10 09:38:24 2003
	$time2 = $elem{'time-sec'}+( $elem{'time-usec'}/1000000);

	$valid=1;
#	print Dumper(\%elem);

	
        # print Dumper(\%elem);
	my $time2= $elem{'time-sec'}+( $elem{'time-usec'}/1000000);

	if ( $lat1 > 1000 ||
	     $lon1 > 1000 ||
	     $lat2 > 1000 ||
	     $lon2 > 1000 
	     ) {
	    #print "lat/lon >1000\n";
	    #print "\t($lat1,$lon1) - ($lat2,$lon2)\n";
	    $valid = 0;
	}
	
	if ( $valid ) {
	    $dist = POI::Gps::earth_distance($lat1,$lon1,$lat2,$lon2);;
	    $time_delta = $time2 - $time1;
	    $speed      = $valid&&$time_delta ? $dist / $time_delta * 3.600 : -1;
	    #printf "Dist: %.4f/%.2f =>  %.2f\n",$dist,$time_delta,$speed;
	    $streets_type_id = 1 if ( $speed >0 );
	    $streets_type_id = 2 if ( $speed >30 );
	    $streets_type_id = 3 if ( $speed >60 );
	    $streets_type_id = 4 if ( $speed >100 );

	    if ( $alt2 == 0 ) { # Otherwise I assume it was POS Mode
		print "Altitude = 0  ---> type=10\n";
		$streets_type_id = 10;
	    }

	    if ( $time_delta >300 ) {
		print "Time diff = $time_delta\n";
		$valid = 0;
	    }

	    if ( $speed >400 ) {
		print "Speed = $speed\n";
		$valid = 0;
	    }

	    if ( $dist > 200  ) {
		printf "earth_distance($lat1,$lon1,$lat2,$lon2) => %.2f\n",$dist;
		$valid = 0;
	    }

	}

	if ( $valid ) {
	    #debug(sprintf "\t(%.3f,%.3f) - (%.3f,%.3f) %.0f m (%.1f Km/h) (Type %d)",$lat1,$lon1,$lat2,$lon2,$dist,$speed,$streets_type_id);
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
	    $distance_street +=$dist;
	    $distance_streets +=$dist;
	} else {
	    if ( $segments_in_street ) {
		$street_nr ++;
		printf "Streets: $street_nr ($segments_in_street Segments)    %.2f Km \n"
		    ,$distance_streets/1000;
	    }
	    $segments_in_street=0;
	}
	$line1=$line;
    }
    if ($segments_in_street ) {
	printf "Streets: $street_nr ($segments_in_street Segments)    %.2f Km \n"
	    ,$distance_streets/1000;
    }

}




# *****************************************************************************
sub import_Data($){
    my $dir = shift;
    my $kismet_dir = $dir || "$main::CONFIG_DIR/kismet";
    my $source = "Kismet Tracks";
    delete_all_from_source($source);
    
    debug("$kismet_dir/{tracks}/*.gps");
    foreach  my $full_filename ( glob("$kismet_dir/*.gps") ) {
	import_Kismet_track_file($full_filename,$source);
    }
}

1;
