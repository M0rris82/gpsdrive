# Some Gps related Functions
#
# $Log$
# Revision 1.3  2005/04/10 00:15:58  tweety
# changed primary language for poi-type generation to english
# added translation for POI-types
# added some icons classifications to poi-types
# added LOG: Entry for CVS to some *.pm Files
#

use strict;
use warnings;

package POI::Gps;

use IO::File;
use File::Basename;
use Data::Dumper;
use Math::Trig;
#use Date::Manip qw(ParseDate DateCalc UnixDate);

require Exporter;
use vars qw(@ISA @EXPORT @EXPORT_OK %EXPORT_TAGS $VERSION);

@ISA = qw( Exporter );

@EXPORT = qw( earth_distance miles2km hash2str is_in_country is_europe is_germany is_bayern is_muenchen);

my $M_PI = 3.14159;

my $last_point={};
my $last_points=[];

#############################################################################
sub pow($$){
    my $x = shift;
    my $y = shift;
    return ($x ** $y);
}
#############################################################################
sub xx_rad2deg($){
    my $x = shift;
    return $x * $M_PI/180.0;
}

#############################################################################
# Liegen die Punkte im Groben innerhalb Europas/Deutschland?
sub is_in_country($$){
    my $point=shift;
    my $country=shift;
    if ($country =~ m/europ/ ) {
	return is_europe($point);
    } elsif ($country =~ m/bayern/ ) {
	return is_bayern($point);
    } elsif ($country =~ m/muenchen/ ) {
	return 0 if $point->{lat}  < 45;
	return 0 if $point->{lat}  >49;
	return 0 if $point->{lon}  <9.5;
	return 0 if $point->{lon}  >12;
    } else {
	return is_germany($point);
    }
	return 1;
}

#############################################################################
# Liegen die Punkte im Groben innerhalb Europas?
sub is_europe($){
    my $point = shift;
    return 0 if $point->{lat}  < 5;
    return 0 if $point->{lat}  > 60;
    return 0 if $point->{lon}  < -10;
    return 0 if $point->{lon}  > 20;
#    return 0 if $point->{lat}  < 10;
#    return 0 if $point->{lat}  > 56;
#    return 0 if $point->{lon}  < 0;
#    return 0 if $point->{lon}  > 16;
    return 1;
}

#############################################################################
# Liegen die Punkte im Groben innerhalb Deutschlands?
sub is_germany($){
    my $point = shift;
    return 0 if $point->{lat}  <47;
    return 0 if $point->{lat}  >55;
    return 0 if $point->{lon}  <5;
    return 0 if $point->{lon}  >15;
    return 1;
}

#############################################################################
# Liegen die Punkte im Groben innerhalb Bayern?
sub is_bayern($){
    my $point = shift;
    return 0 if $point->{lat}  <47;
    return 0 if $point->{lat}  >51;
    return 0 if $point->{lon}  <5;
    return 0 if $point->{lon}  >15;
    return 1;
}

    
#############################################################################

sub earth_distance($$$$){
    my $lat1 = shift || 0;
    my $lon1 = shift || 0;
    my $lat2 = shift || 0;
    my $lon2 = shift || 0;

    # /*
    # my $calcedR1 = calcR(lat1);
    # my $calcedR2 = calcR(lat2);

    # my $sinradi1 = sin(rad2deg(90-lat1));
    # my $sinradi2 = sin(rad2deg(90-lat2));
    
    # my $x1 = calcedR1 * cos(rad2deg(lon1)) * sinradi1;
    # my $x2 = calcedR2 * cos(rad2deg(lon2)) * sinradi2;
    # my $y1 = calcedR1 * sin(rad2deg(lon1)) * sinradi1;
    # my $y2 = calcedR2 * sin(rad2deg(lon2)) * sinradi2;
    # my $z1 = calcedR1 * cos(rad2deg(90-lat1));
    # my $z2 = calcedR2 * cos(rad2deg(90-lat2));
    
    # my $calcedR = calcR((double)(lat1+lat2)) / 2;
    # my $a = acos((x1*x2 + y1*y2 + z1*z2)/square(calcedR));
    # */
	
    my $x1 = calcR( $lat1 ) * cos(rad2deg( $lon1 )) * sin(rad2deg( 90-$lat1 ));
    my $x2 = calcR( $lat2 ) * cos(rad2deg( $lon2 )) * sin(rad2deg( 90-$lat2 ));
    my $y1 = calcR( $lat1 ) * sin(rad2deg( $lon1 )) * sin(rad2deg( 90-$lat1 ));     
    my $y2 = calcR( $lat2 ) * sin(rad2deg( $lon2 )) * sin(rad2deg( 90-$lat2 ));
    my $z1 = calcR( $lat1 ) * cos(rad2deg( 90 - $lat1 ));
    my $z2 = calcR( $lat2 ) * cos(rad2deg( 90 - $lat2 ));   

    my $a = acos( ($x1*$x2 + $y1*$y2 + $z1*$z2) / pow(calcR( ($lat1+$lat2)/2),2) );

    if(0){
	if ( $a =~ m/i/ ) {
	    my $a1 =      ($x1*$x2 + $y1*$y2 + $z1*$z2);
	    my $a2 =                                          calcR( ($lat1+$lat2)/2) ;
	    my $a3 =    ( ($x1*$x2 + $y1*$y2 + $z1*$z2) / pow(calcR( ($lat1+$lat2)/2),2) ) ;
	    my $b = acos( $a3 );
	    my $t1 = 40590335705258.5;
	    my $t2 = 40590335705258.5;
	    my $t3 = $t1/pow($t2,2);
	    print "acos(1)= ".acos(1)."\n";
	    print "acos($a3)= ".acos($a3)."\n";
	    print "acos($t3)= ".acos($t3)."\n";
	    print "earth_distance($lat1,$lon1,$lat2,$lon2) 	$b	1: $a1	2:$a2	3:$a3   ($x1*$x2 + $y1*$y2 + $z1*$z2)\n";
	}
    }
    my $erg = calcR( ($lat1+$lat2) / 2) * $a;
    $erg = $erg / 3340;
    $erg =~ s/.*e-0\di\s*$/0.0/; # damit werte von  '2.98023223876953e-08i' eliminiert werden
    return $erg;
}

# Lifted from gpsdrive 1.7
# CalcR gets the radius of the earth at a particular latitude
# calcxy finds the x and y positions on a 1280x1024 image of a certian scale
#  centered on a given lat/lon.

# This pulls the "real radius" of a lat, instead of a global guesstimate
sub calcR($){
    my $lat = shift;

    my $a = 6378.137;
    my ( $r, $sc, $x, $y, $z);
    my $e2 = 0.081082 * 0.081082;
    # the radius of curvature of an ellipsoidal Earth in the plane of the
    # meridian is given by

    #R' = a * (1 - e^2) / (1 - e^2 * (sin(lat))^2)^(3/2)

    #where a is the equatorial radius,
    # b is the polar radius, and
    # e is the eccentricity of the ellipsoid = sqrt(1 - b^2/a^2)

    # a = 6378 km (3963 mi) Equatorial radius (surface to center distance)
    # b = 6356.752 km (3950 mi) Polar radius (surface to center distance)
    # e = 0.081082 Eccentricity
    # */

    $lat = $lat * $M_PI / 180.0;
    $sc = sin($lat);
    $x = $a * (1.0 - $e2);
    $z = 1.0 - $e2 * $sc * $sc;
    $y = pow($z, 1.5);
    $r = $x / $y;

    $r = $r * 1000.0;
    return $r;
}

my $map_width = 1280;
my $map_height = 1024;
my $draw_x_offset=0;
my $draw_y_offset=0;


sub miles2km($){
    my $m = shift;
    return $m * 1.6;
}

sub hash2str($){
    my $h = shift;
    my $erg = '';

    foreach my $k ( sort keys %$h){
	next if $k eq 'line';
	$erg .= ", " if $erg;
	$erg .= "$k=$h->{$k}";
    }
    $erg .= "	line: $h->{line} ";
    return  $erg;
}
1;
