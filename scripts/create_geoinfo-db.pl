#!/usr/bin/perl
# Create SQLite Database file used by GpsDrive
# Fill it with:
#   - POI Sources predefined in this script
#   - POI Types as defined in icons.xml
#   TODO: - Field Types for additional POI Information (poi_extra)

########################################################################################

my $default_lang  = 'en';
my $default_licence = 'Creative Commons Attribution-ShareAlike 2.0';

########################################################################################

# Get version number from version-control system, as integer 
my $version = '$Revision: 1824 $';
$Version =~ s/\$Revision:\s*(\d+)\s*\$/$1/;
 
my $VERSION ="create_geoinfo-db.pl (c) Guenther Meyer
Version 0.2-$Version";

# History:
#
# v0.2 Added Table for OpenStreetMap-Matching
# v0.1 Initial Release

use strict;
use warnings;

use DBI;
use File::Copy;
use XML::Twig;
use Getopt::Long;

my $lang;
my $db_file;
my $dbh;


########################################################################################
# Execute SQL statement
#
sub db_exec($){
    my $statement = shift;

    my $sth = $dbh->prepare($statement);
    unless ( $sth->execute() ) {
        warn "Error in query '$statement'\n";
        $sth->errstr;
        return 0;
    }
    return 1;
}


########################################################################################
# Creata db file and tables
#
sub create_dbfile(){
    my $create_statement;
    my $sth;

    move("$db_file","$db_file.old");

    $create_statement="CREATE DATABASE geoinfo;";
    $dbh = DBI->connect("dbi:SQLite:dbname=$db_file",'','');
    $dbh->{unicode} = 1;

    # ------- POI_TYPE
    db_exec('CREATE TABLE poi_type (
		poi_type       VARCHAR(160)  PRIMARY KEY,
		scale_min      INTEGER       NOT NULL default \'1\',
		scale_max      INTEGER       NOT NULL default \'50000\',
		title          VARCHAR(160)  NULL default \'\',
		description    VARCHAR(160)  NULL default \'\');') or die;

     # ------- SOURCE
    db_exec('CREATE TABLE source (
		source_id      INTEGER      PRIMARY KEY,
		name           VARCHAR(80)  NOT NULL default \'\',
		comment        VARCHAR(160) NOT NULL default \'\',
		last_update    DATE         NOT NULL default \'0000-00-00\',
		url            VARCHAR(160) NOT NULL default \'\',
		licence        VARCHAR(160) NOT NULL default \'\');') or die;

     # ------- OPENSTREETMAP
    db_exec('CREATE TABLE osm (
		key            VARCHAR(80)  NOT NULL default \'\',
		value          VARCHAR(160) NOT NULL default \'\',
		poi_type       VARCHAR(160) NOT NULL default \'unknown\');') or die;
}


########################################################################################
# Fill poi_type table
#
sub fill_default_poi_types {
    my $i=1;
    my $used_icons ={};

    my $unused_icon ={};
    my $existing_icon ={};

    my $icon_file='../data/map-icons/icons.xml';
    $icon_file = '../share/map-icons/icons.xml'         unless -s $icon_file;
    $icon_file = '/usr/local/share/icons/map-icons/icons.xml' unless -s $icon_file;
    $icon_file = '/usr/local/share/map-icons/icons.xml' unless -s $icon_file;
    $icon_file = '/usr/share/icons/map-icons/icons.xml' unless -s $icon_file;
    $icon_file = '/usr/share/map-icons/icons.xml'       unless -s $icon_file;
    $icon_file = '/opt/gpsdrive/icons.xml'              unless -s $icon_file;
    die "no Icon File found" unless -s $icon_file;

    our $title = ''; our $title_en = '';
    our $description = ''; our $description_en = '';

    # parse icon file
    #
    my $twig= new XML::Twig
    (
       TwigHandlers => { rule        => \&sub_poi,
                         title       => \&sub_title,
                         description => \&sub_desc }
    );
    $twig->parsefile( "$icon_file");
    my $rules= $twig->root;

    $twig->purge;

    sub sub_poi
    {
      my ($twig, $poi_elm) = @_;
      if ($poi_elm->first_child('condition')->att('k') eq 'poi')
      {
        my $name = $poi_elm->first_child('condition')->att('v');
        my $scale_min = $poi_elm->first_child('scale_min')->text;
        my $scale_max = $poi_elm->first_child('scale_max')->text;
        $title = $title_en unless ($title);
	$description = $description_en unless ($description);

	# replace ' by something else, because otherwise the sql statement wil fail
	#$description =~ s/'/&#0039;/g;
	$description =~ s/'/&apos;/g;

	print "Adding POI: $name\n";
	print "            $title - $description\n";

	db_exec(
	  "INSERT INTO `poi_type` ".
          "(poi_type, scale_min, scale_max, title, description) ".
	  "VALUES ('$name','$scale_min','$scale_max','$title','$description');") 
	    or die;
      }
      $title = ''; $title_en = '';
      $description = ''; $description_en = '';
    }

    sub sub_title
    {
      my ($twig, $title_elm) = @_;
      if ($title_elm->att('lang') eq 'en')
        { $title_en = $title_elm->text; }
      elsif ($title_elm->att('lang') eq $lang)
        { $title = $title_elm->text; }
    }

    sub sub_desc
    {
      my ($twig, $desc_elm) = @_;
      if ($desc_elm->att('lang') eq 'en')
        { $description_en = $desc_elm->text; }
      elsif ($desc_elm->att('lang') eq $lang)
        { $description = $desc_elm->text; }
    }
}


########################################################################################
# Fill source table
#
sub fill_default_sources() {   # Just some Default Sources

    my @sources = (
      { source_id   => '1',
        name        => 'unknown',
        comment     => 'Unknown source or source not defined', 
        last_update => '2008-03-01',
        url         => 'http://www.gpsdrive.de/',
        licence     => 'unknown'
      },
      { source_id   => '2',
        name        => 'way.txt',
        comment     => 'Data imported from way.txt', 
        last_update => '2008-03-01',
        url         => 'http://www.gpsdrive.de/',
        licence     => 'unknown'
      },
      { source_id   => '3',
        name        => 'user',
	comment     => 'Data entered by the GpsDrive-User',
	last_update => '2008-03-01',
	url         => 'http://www.gpsdrive.cc/',
	licence     => $default_licence
      },
      { source_id   => '4',
        name        => 'OpenStreetMap.org',
        comment     => 'General Data imported from the OpenStreetMap Project', 
        last_update => '2007-01-03',
        url         => 'http://www.openstreetmap.org/',
        licence     => 'Creative Commons Attribution-ShareAlike 2.0'
      },
      { source_id   => '5',
        name        => 'groundspeak',
        comment     => 'Geocache data from Groundspeak', 
        last_update => '2007-01-30',
        url         => 'http://www.groundspeak.com/',
        licence     => 'unknown'
      },
      { source_id   => '6',
        name        => 'opencaching',
        comment     => 'Geocache data from Opencaching', 
        last_update => '2007-09-30',
        url         => 'http://www.opencaching.de/',
        licence     => 'unknown'
      },
      
      { source_id   => '7',
        name        => 'friendsd',
        comment     => 'Position received from friendsd server', 
        last_update => '2007-09-30',
        url         => 'http://friendsd.gpsdrive.de/',
        licence     => 'none'
      },
      { source_id   => '8',
        name        => 'fon',
        comment     => 'Access point data from FON', 
        last_update => '2007-09-30',
        url         => 'http://www.fon.com/',
        licence     => 'unknown'
      },
      { source_id   => '9',
        name        => 'kismet',
        comment     => 'Access point data found by Kismet', 
        last_update => '2008-03-01',
        url         => 'http://www.kismetwireless.net/',
        licence     => 'unknown'
      },
      { source_id   => '10',
        name        => 'postgis',
        comment     => 'Data read from a local mapnik/postgis database', 
        last_update => '2008-03-11',
        url         => 'http://www.openstreetmap.org/',
        licence     => 'Creative Commons Attribution-ShareAlike 2.0'
      },
    );

    foreach (@sources) {
      print "Adding Source: $$_{'name'} - $$_{'url'}\n";
      db_exec(
        "INSERT INTO `source` ".
          "(source_id, name, comment, last_update, url, licence) ".
	  "VALUES ($$_{'source_id'},'$$_{'name'}','$$_{'comment'}',".
	  "'$$_{'last_update'}','$$_{'url'}','$$_{'licence'}');") or die;
    }

}


########################################################################################
# Fill openstreetmap table
#
sub fill_osm_table() {

my %osm_table =
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

  my $key;
  my $value;

  foreach $key (keys %osm_table)
  {
    for $value (keys %{$osm_table{$key}})
    {
        db_exec(
        "INSERT INTO `osm` (key,value,poi_type) VALUES ('$key','$value','$osm_table{$key}{$value}');")
          or die;
    }
  }

}


########################################################################################
#
#                     Main
#
########################################################################################

# Set defaults and get options from command line
Getopt::Long::Configure('no_ignore_case');
GetOptions (
	     'lang=s'              => \$lang,
	   );

$lang = $lang || $default_lang;

if ($lang eq 'en')
  { $db_file = "./geoinfo.db"; }
else
  { $db_file = "./geoinfo.$lang.db"; }

print "$VERSION\n";

create_dbfile();
fill_default_sources();
fill_default_poi_types();
fill_osm_table();


__END__

=head1 NAME

B<create_geoinfo-db.pl>

=head1 DESCRIPTION

B<create_geoinfo-db.pl> is creating the geoinfo.db from the icons and icons.xml File

Create SQLite Database file used by GpsDrive
Fill it with:
   - POI Sources predefined in this script
   - POI Types as defined in icons.xml

=head1 SYNOPSIS

B<Common usages:>

  create_geoinfo-db.pl --lang=en

=head1 OPTIONS

=over 8

=item B<-lang>

select the language to use

=back


=head1 AUTHOR

Written by Guenther Meyer <d.s.e@sordidmusic.com>

=head1 COPYRIGHT

This is free software.  You may redistribute copies of it under the terms of the GNU General Pub-
lic  License <http://www.gnu.org/licenses/gpl.html>.  There is NO WARRANTY, to the extent permit-
ted by law.

=head1 SEE ALSO

gpsdrive(1)


=cut

