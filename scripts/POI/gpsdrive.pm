package POI::mapsource;

use strict;
use warnings;

use IO::File;
use POI::DBFuncs;
use POI::Utils;


#############################################################################
# Write the Waypoints in gpsdrive Format
#############################################################################
sub write_gpsdrive_waypoints($$){
    my $waypoints = shift;
    my $full_filename= shift;
    print "Writing gpsdrive waypoints to '$full_filename'\n";
    my $fo = IO::File->new(">$full_filename");
    $fo or die ("write_gpsdrive_waypoints: Cannot open $full_filename:$!\n");

    for my $wp_name ( sort keys  %{$waypoints} ) {
	my $values = $waypoints->{$wp_name};

	unless ( defined($values->{'poi.lat'}) ) {
	    print "Error undefined poi.lat: ".Dumper(\$values);
	}
	unless ( defined($values->{'poi.lon'}) ) {
	    print "Error undefined poi.lon: ".Dumper(\$values);
	}

	$wp_name =~ s/ÃÂ€/ae/g;
	$wp_name =~ s/ÃÂ¶/oe/g;
	$wp_name =~ s/ÃÂŒ/ue/g;
	$wp_name =~ s/ÃÂ€/Ae/g;
	$wp_name =~ s/Ã/Oe/g;
	$wp_name =~ s/Ã/Ue/g;
	$wp_name =~ s/Ã/sss/g;

	correct_lat_lon($values);

	for my $t (qw(Wlan Action Sqlnr Proximity) ) {
	    unless ( defined ( $values->{$t})) {
		$values->{$t} = 0;
	    }
	}

	$values->{Symbol} ||= "-";

	print $fo $wp_name;
	print $fo "\t".$values->{'poi.lat'};
	print $fo "\t".$values->{'poi.lon'};
	print $fo "\t".($values->{'poi.Symbol'}||'');
	print $fo "\t".($values->{'poi.Wlan'}||'');
	print $fo "\t".$values->{'Action'};
	print $fo "\t".$values->{'Sqlnr'};
	my $proximity = $values->{'Proximity'};
	$proximity =~ s/\s*m$//;
	print $fo "\t".$proximity;
	print $fo "\n";
    }
    $fo->close()
	or die ("write_gpsdrive_waypoints: Cannot close $full_filename:$!\n");
}


1;
