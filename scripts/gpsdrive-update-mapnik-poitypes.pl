#!/usr/bin/perl
# match gpsdrive poi_types with osm types and update the
# poi column inside the mapnik postgis db

# Get version number from version-control system, as integer 
my $version = '$Revision: 1824 $';
$Version =~ s/\$Revision:\s*(\d+)\s*\$/$1/;
 
my $VERSION ="gpsdrive-update-mapnik-poitypes.pl (c) Guenther Meyer
Version 0.1-$Version";


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

use DBI;
use Getopt::Long;
use Utils::Debug;
use Pod::Usage;

my ($help);
my $do_show_version=0;
my $do_not_add_column=0;
my $dbh;
my $sth;

my $db_name = 'gis';
my $db_table = 'planet_osm_point';

my %poi_table =
(
 'aeroway' =>
 {
   'airport'		=>	'transport.airport',
   'helipad'		=>	'transport.airport.helipad',
   'terminal'		=>	'transport.airport.terminal',
 },
 'amenity' =>
 {
   'arts_centre'	=>	'public.arts_centre',
   'atm'		=>	'money.atm',
   'bicycle_parking'	=>	'vehicle.parking.bike',
   'bank'		=>	'money.bank',
   'bank;atm'		=>	'money.bank',
   'bus_stop'		=>	'transport.bus',
   'bus_station'	=>	'transport.bus',
   'courthouse'		=>	'public.administration.court_of_law',
   'church'		=>	'religion.church',
   'cafe'		=>	'food.cafe',
   'cemetery'		=>	'religion.cemetery',
   'cinema'		=>	'recreation.cinema',
   'clinic'		=>	'health.hospital',
   'community_centre'	=>	'public',
   'college'		=>	'education.college',
   'doctor'		=>	'health.doctor',
   'doctors'		=>	'health.doctor',
   'embassy'		=>	'public.administration.embassy',
   'fast_food'		=>	'food.fastfood',
   'fire_station'	=>	'public.firebrigade',
   'fuel'		=>	'vehicle.fuel_station',
   'garage'		=>	'vehicle.repair_shop',
   'grave_yard'		=>	'religion.cemetery',
   'hospital'		=>	'health.hospital',
   'kindergarden'	=>	'education.kindergarden',
   'library'		=>	'shopping.rental.library',
   'museum'		=>	'sightseeing.museum',
   'park'		=>	'recreation.park',
   'parking'		=>	'vehicle.parking',
   'pharmacy'		=>	'health.pharmacy',
   'phone_box'		=>	'public.telephone',
   'place_of_worship'	=>	'religion.church',
   'playground'		=>	'recreation.playground',
   'police'		=>	'public.police',
   'post_box'		=>	'public.post_box',
   'post_office'	=>	'public.post_office',
   'postbox'		=>	'public.post_box',
   'prison'		=>	'public.administration.prison',
   'pub'		=>	'food.pub',
   'public_building'	=>	'public',
   'recycling'		=>	'public.recycling',
   'restaurant'		=>	'food.restaurant',
   'school'		=>	'education.school',
   'shop'		=>	'shopping',
   'shops'		=>	'shopping',
   'supermarket'	=>	'shopping.supermarket',
   'telephone'		=>	'public.telephone',
   'taxi'		=>	'transport.taxi',
   'theatre'		=>	'recreation.theater',
   'toilets'		=>	'public.toilets',
   'townhall'		=>	'public.administration.townhall',
   'university'		=>	'education.university',
   'village_hall'	=>	'public.administration.townhall',
 },
 'highway' =>
 {
   'bus_stop'	=>	'transport.bus',
   'toll_booth'	=>	'vehicle.toll_station',
 },
 'landuse' =>
 {
   'cave'	=>	'misc.landmark.cave',
   'peak'	=>	'misc.landmark.peak',
   'spring'	=>	'misc.landmark.spring',
 },
 'leisure' =>
 {
   'common'		=>	'recreation',
   'garden'		=>	'recreation.park',
   'golf_course'	=>	'sports.golf',
   'marina'		=>	'nautical.marina',
   'nature_reserve'	=>	'recreation.nature_reserve',
   'park'		=>	'recreation.park',
   'pitch'		=>	'sports.pitch',
   'playground'		=>	'recreation.playground',
   'playing_fields'	=>	'sports.pitch',
   'sports_centre'	=>	'sports.sports_centre',
   'sport'		=>	'sports',
   'stadium'		=>	'sports.stadium',
   'swimming_pool'	=>	'sports.swimming',
   'water_park'		=>	'recreation.water_park',
   'zoo'		=>	'recreation.zoo',
 },
 'man_made' =>
 {
   'barn'		=>	'misc.landmark.barn',
   'cairn'		=>	'misc.landmark.cairn',
   'gasometer'		=>	'misc.landmark.gasometer',
   'lighthouse'		=>	'misc.landmark.lighthouse',
   'power_fossil'	=>	'misc.landmark.power.fossil',
   'power_hydro'	=>	'misc.landmark.power.hydro',
   'power_wind'		=>	'misc.landmark.power.wind',
   'wastewater_plant'	=>	'misc.landmark.wastewater_plant',
   'water_tower'	=>	'misc.landmark.water_tower',
   'windmill'		=>	'misc.landmark.windmill',
 },
 'place' =>
 {
   'city'	=>	'places.settlement.city',
   'hamlet'	=>	'places.settlement.hamlet',
   'suburb'	=>	'places.settlement.suburb',
   'town'	=>	'places.settlement.town',
   'village'	=>	'places.settlement.village',
 },
 'railway' =>
 {
   'halt'	=>	'transport.railway_small',
   'station'	=>	'transport.railway',
   'tram_stop'	=>	'transport.tram',
 },
 'sport' =>
 {
   'basketball'	=>	'sports.basketball',
   'baseball'	=>	'sports.baseball',
   'bowling'	=>	'sports.bowling',
   'bowls'	=>	'sports.bowling',
   'climbing'	=>	'sports.climbing',
   'cricket'	=>	'sports.cricket',
   'cycling'	=>	'sports.cycling',
   'equestrian'	=>	'sports.riding',
   'football'	=>	'sports.football',
   'golf'	=>	'sports.golf',
   'golf_course'	=>	'sports.golf',
   'gymnastics'	=>	'sports.gymnastics',
   'hockey'	=>	'sports.hockey',
   'motor'	=>	'sports.motor',
   'multi'	=>	'sports.centre',
   'pitch'	=>	'sports.pitch',
   'polo'	=>	'sports.polo',
   'rugby'	=>	'sports.rugby',
   'sailing'	=>	'sports.sailing',
   'skateboard'	=>	'sports.skate_park',
   'skating'	=>	'sports.skating',
   'skiing'	=>	'sports.skiing',
   'soccer'	=>	'sports.soccer',
   'sports_centre'	=>	'sports.centre',
   'squash'	=>	'sports.squash',
   'swimming'	=>	'sports.swimming',
   'table_tennis'	=>	'sports.table_tennis',
   'tennis'	=>	'sports.tennis',
   'track'	=>	'sports.track',
   'volleyball'	=>	'sports.volleyball',
 },
 'tourism' =>
 {
   'attraction'		=>	'sightseeing',
   'camp_site'		=>	'accommodation.camping',
   'caravan_site'	=>	'accommodation.camping.caravan',
   'guest_house'	=>	'accommodation.guest_house',
   'hostel'		=>	'accommodation.hostel',
   'hotel'		=>	'accommodation.hotel',
   'information'	=>	'misc.information',
   'museum'		=>	'sightseeing.museum',
   'motel'		=>	'accommodation.motel',
   'picnic_site'	=>	'recreation.picnic',
   'theme_park'		=>	'recreation.theme_park',
   'viewpoint'		=>	'sightseeing.viewpoint',
   'zoo'		=>	'recreation.zoo',
 },
# 'name' =>
# {
#   'foo' => 'bar',
# }
);


# Set defaults and get options from command line
Getopt::Long::Configure('no_ignore_case');
GetOptions
(
  'verbose'	=> \$VERBOSE,
  'v+'		=> \$VERBOSE,
  'h|help'	=> \$help,
  'version'	=> \$do_show_version,
  'n'		=> \$do_not_add_column,
)
  or pod2usage(1);


if ( $do_show_version )
{
  print "$VERSION\n";
};

pod2usage(1) if $help;


#############################################################################
# Connect to database and add column 'poi' if not yet available
#
sub update_poi_column()
{

  my $key;
  my $value;

  foreach $key (keys %poi_table)
  {
    my $i = 0;
    my $j = 0;
    print "$key:";

    $sth = $dbh->prepare(qq{UPDATE planet_osm_point SET poi = ? WHERE $key = ?});

    for $value (keys %{$poi_table{$key}})
    {
      $j = $sth->execute($poi_table{$key}{$value}, $value);
      $i += $j;
      print "\n    ($j) $value   =   $poi_table{$key}{$value}" if ($VERBOSE && $j > 0);
    }
    print " ($i)" unless ($VERBOSE);
    print "\n";
  }

}


#############################################################################
#
#                     Main
#
#############################################################################

# connect to database
$dbh = DBI->connect("dbi:Pg:dbname=$db_name",'','',{ RaiseError => 1, AutoCommit => 1 })
  or die $dbh->errstr;

# add poi column to database
unless ($do_not_add_column)
{
  print "Adding column 'poi' to database...\n" if ($VERBOSE);
  $sth = $dbh->prepare("ALTER TABLE $db_table ADD COLUMN poi text;");
  $sth->execute()
    or warn $sth->errstr;
}

# fill poi column
update_poi_column();

# disconnect from database
$dbh->disconnect
  or warn $dbh->errstr;


__END__

=head1 NAME

B<gpsdrive-update-mapnik-poitypes.pl> Version 0.1

=head1 DESCRIPTION

B<gpsdrive-update-mapnik-poitypes.pl> is a program that looks for entries
indicating "Points of Interest" inside the mapnik database, and adds the
matching gpsdrive poi_types to a separate column called "poi".

WARNING: 
    This programm replaces some/all poi entries.
    So any changes made to the database may be overwritten!!!


=head1 SYNOPSIS

B<Common usages:>

gpsdrive-update-mapnik-poitypes.pl [-v] [-h] [-n]

=back
