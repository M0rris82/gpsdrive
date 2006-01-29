# Database Defaults for poi/streets Table for poi.pl
#
# $Log$
# Revision 1.8  2006/01/29 21:58:43  tweety
# small fixes
#
# Revision 1.7  1994/06/08 13:02:31  tweety
# adjust debug levels
#
# Revision 1.6  2005/11/26 18:16:45  tweety
# insert horse icon
#
# Revision 1.5  2005/11/21 07:35:14  tweety
# *** empty log message ***
#
# Revision 1.3  2005/11/21 06:57:18  tweety
# Update icons.txt and DB_Defaults to reflect new icons
#
# Revision 1.2  2005/10/11 08:28:35  tweety
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

package Geo::Gpsdrive::DB_Defaults;

use strict;
use warnings;

use POSIX qw(strftime);
use Time::Local;
use DBI;
use Geo::Gpsdrive::Utils;
use Data::Dumper;
use IO::File;
use Geo::Gpsdrive::DBFuncs;

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

# Icon
my $icons = {		  
    '1' 			          => 'ppl1',
    '2' 			          => 'ppl2',
    '3' 			          => 'ppl3',
    '4' 			          => 'ppl4',
    '5' 			          => 'ppl5',
    'accomodation' 	                  => "hotel",
    'area.bridge'                     => 'bridge',
    'area.bridge.car'                 => 'bridge_car',
    'area.bridge.pedestrian'          => 'bridge_pedestrian',
    'area.bridge.train'               => 'bridge_train',
    'area.forest'                     => 'trees',
    'area.mountain'                   => "summit",
    'area.pedestrian zone'            => "pedestrian",
    'area.play_street'                => "play_street",
    'bank'                            => "Dollar",
    'bank.ATM.EC'                     => "ec",
    'bank.agency.Postbank'            => 'post',
    'bank.agency.Reifeisenbank'       => "VR",
    'camping.water'                   => 'tap',
    'church'                          => "church",
    'church.cemetary'                 => 'cemetary',
    'circle' 			      => 'ppl',
    'custom.friends.home' 	          => "home",
    'custom.friends.work' 	          => "work",
    'custom.my.home'  	          	=> "home",
    'custom.my.work' 	          	=> "work",
    'education.primary school'        	=> 'ABC',
    'education.university'            	=> 'emc2',
    'food' 		                => "food",
    'food.beergarden'                   => 'beergarden',
    'food.cafe'                         => "cafe",
    'food.cafe.internet'                => "cafe",
    'food.drinking water'               => 'tap_drinking',
    'food.ice cream parlor'		=> "ice_cream",
    'food.restaurant.asia'            => 'japanese',
    'food.restaurant.bavarian'        => 'beergarden',
    'food.restaurant.chinese'         => 'japanese',
    'food.restaurant.fastfood.burger king'       => "burgerking",
    'food.restaurant.fastfood.mc donalds' 	  => "mcdonalds",
    'food.restaurant.fastfood.pizza_hut' => 'pizza',
    'food.restaurant.ice cream parlor'=> 'ice_cream',
    'food.restaurant.japanese'        => 'japanese',
    'food.restaurant.pizza'           => 'pizza',
    'health'                          => "red_cross",
    'health.doctor'                   => 'aesculab',
    'health.emergency'                => 'emergency',
    'health.hospital'                 => 'hospital',
    'health.pharmacy'                 => "pharmacy",
    'information'                     => "info",
    'mine' 			          => 'mine',
    'open' 			          => 'open',
    'postal'                          => 'post',
    'postal.letter-box'               => 'letter',
    'postal.office'                   => 'letter1',
    'recreation'                      => 'balloon',
    'recreation.cinema'               => 'movies',
    'recreation.geocache' 	          => "geocache",
    'recreation.landmark' 	          => "monument",
    'recreation.music'                => 'music',
    'recreation.music.opera'          => 'masks',
    'recreation.music.theater'        => 'masks',
    'recreation.night club' 	  => "girls",
    'recreation.playground'           => 'swing',
    'recreation.pub'                  => 'beer',
    'recreation.pub.dart'             => "dart",
    'recreation.sports.bicycling' => 'bicycling',
    'recreation.sports.boat'           => "boat",
    'recreation.sports.diving'        => 'alpha_flag',
    'recreation.sports.golf place'     => "golf",
    'recreation.sports.horse.racing'    => 'horse',
    'recreation.sports.horse.riding'    => 'horse',
    'recreation.sports.motorbike'      => "motorbike",
    'recreation.sports.skiing' 	  => 'skiing',
    'shopping'                        => "shop",
    'shopping.food.bakery'            => 'prezel',
    'shopping.supermarket.lidl'       => "lidl_logo",
    'summit'                          => 'summit',
    'telephone'                       => 'phone',
    'transport.airport'               => "airport",
    'transport.car'                   => "car",
    'transport.car.garage'            => "car_repair",
    'transport.car.gas station'       => "fuel",
    'transport.car.gas station.agip'  => "fuel.agip",
    'transport.car.parking.handicapped'=> "handicapped",
    'transport.car.parking.park_and_ride'=> "P+R",
    'transport.car.parkinglot'        => "parking",
    'transport.car.phototrap traffic light' => "speed_trap",
    'transport.car.rental'            => 'car_rental',
    'transport.car.rental.sixt'       => 'sixt',
    'transport.car.rest area' 	  => 'rest_area',
    'transport.car.wrecking service'  => 'towing',
    'transport.nautical.alpha flag'   => 'alpha_flag',
    'transport.nautical.dive flag'    => 'dive_flag',
    'transport.park and ride'         => 'P+R',
    'transport.public.airport'        => 'airport',
    'transport.public.bus station'   => 'bus_stop',
    'transport.public.ferry'          => 'ferry',
    'transport.public.railway'        => 'train',
    'transport.public.station.railroad' => 'train',
    'transport.public.station.subway city'    => 'U-Bahn',
    'transport.public.station.subway regional'=> 'S-Bahn',
    'transport.public.station.tram'          => 'bus_stop',
    'transport.traffic.construction'  => "warning",
    'transport.traffic.speedtrap'     => "speed_trap",
    'transport.traffic.traffic jam'   => "warning",
    'transport.truck'                 => 'truck',
    'w-lan.open' 	                  => "wlan",
    'w-lan.public' 	                  => "tower",		
    'w-lan.wep' 	                  => "wlan-wep",

};
# Some suggestions for pictures
#  education.nursery ==> Schnuller
#  Entrance          ==> Door
#  police            ==> Sherrif Stern
#  park_and_ride     ==> blaues P + Auto + xxx
#  recreation.fairground    ==> Riessenrad
#  gift_shop         ==> Geschenk Paeckchen
#  model.airplanes   ==> Airplane + Fernsteuerung
#  recreation.playground   => Schaukel
#  Bei Sportarten evtl. die Olympischen Symbole verwenden
#  Zoo               ==> Giraffe+ Voegel
#  WC                ==> WC oder 00
#  Recycling         ==> R oder Gruene Punkt

my @poi_type_names
    = qw( unknown
	  WC
	  accomodation
	  accomodation.campground
	  accomodation.campground.campground
	  accomodation.campground.car
	  accomodation.campground.shower
	  accomodation.campground.tent
	  accomodation.campground.trailer
	  accomodation.hotel
	  accomodation.motel
	  accomodation.nursing_home
	  accomodation.youth_hostel
	  administration
	  administration.car_registration
	  area.area
	  area.bridge
	  area.bridge.car
	  area.bridge.pedestrian
	  area.bridge.train
	  area.city
	  area.country
	  area.forest
	  area.lake
	  area.mountain
	  area.pedestrian_zone
	  area.play_street
	  area.state
	  area.tunnel
	  bank
	  bank.ATM
	  bank.ATM.EC
	  bank.agency.HypoVereinsbank
	  bank.agency.Postbank
	  bank.agency.Reifeisenbank
	  bank.agency.Sparda
	  bank.agency.Sparkasse
	  bank.money_exchange
	  camping.dumpstation
	  camping.water
	  church
	  church.catholic
	  church.cemetary
	  church.protestant
	  church.synagoge
	  custom
	  custom.friends
	  custom.friends.home
	  custom.friends.work
	  custom.my.home
	  custom.my.work
	  developer
	  education
	  education.adult_education
	  education.highschool
	  education.kindergarden
	  education.nursery
	  education.primary_school
	  education.secondary_school
	  education.university
	  entrance
	  food
	  food.beergarden
	  food.cafe
	  food.cafe.internet
	  food.delivery_service
	  food.drinking_water
	  food.restaurant
	  food.restaurant.asia
	  food.restaurant.bavarian
	  food.restaurant.chinese
	  food.restaurant.fastfood
	  food.restaurant.fastfood.burger_king
	  food.restaurant.fastfood.mc_donalds
	  food.restaurant.fastfood.pizza_hut
	  food.restaurant.greek
	  food.restaurant.ice_cream_parlor
	  food.restaurant.indian
	  food.restaurant.italian
	  food.restaurant.japanese
	  food.restaurant.pizza
	  health
	  health.ambulance
	  health.doctor
	  health.emergency
	  health.hospital
	  health.pharmacy
	  import_way.txt
	  information
	  mine
	  office
	  office.fire_department
	  office.police
	  office.town_hall
	  postal.letter-box
	  postal.office
	  recreation
	  recreation.cinema
	  recreation.event_location
	  recreation.exhibitions
	  recreation.exhibitions.exibition
	  recreation.exhibitions.museum
	  recreation.exhibitions.trade_show
	  recreation.fairground
	  recreation.geocache
	  recreation.gift_shop
	  recreation.landmark
	  recreation.landmark.monument
	  recreation.model_aircrafts
	  recreation.music
	  recreation.music.concert_hall
	  recreation.music.dance_floor
	  recreation.music.disco
	  recreation.music.opera
	  recreation.music.theater
	  recreation.national_park
	  recreation.night_club
	  recreation.park
	  recreation.park.amusement
	  recreation.park.aquarium
	  recreation.park.zoo
	  recreation.picnic_area
	  recreation.playground
	  recreation.pub
	  recreation.pub.dart
	  recreation.sport
	  recreation.sports.bicycling
	  recreation.sports.boat
	  recreation.sports.bob
	  recreation.sports.bowling
	  recreation.sports.climbing
	  recreation.sports.climbing.area
	  recreation.sports.climbing.hall
	  recreation.sports.climbing.rocks
	  recreation.sports.coast
	  recreation.sports.diving
	  recreation.sports.fishing
	  recreation.sports.fittness
	  recreation.sports.golf_place
	  recreation.sports.horse.racing
	  recreation.sports.horse.riding
	  recreation.sports.hunt
	  recreation.sports.ice_scating
	  recreation.sports.minigolf_place
	  recreation.sports.motorbike
	  recreation.sports.parachute
	  recreation.sports.rock_climbing
	  recreation.sports.rollerblades
	  recreation.sports.rollerscates
	  recreation.sports.skiing
	  recreation.sports.skiing.cross-country
	  recreation.sports.skiing.down-hill
	  recreation.sports.snakeboard
	  recreation.sports.soccer_field
	  recreation.sports.soccer_stadion
	  recreation.sports.swimming
	  recreation.sports.swimming_area
	  recreation.sports.tennis_place
	  recreation.sports.water_ski
	  recreation.zoo
	  recycling
	  recycling.glass
	  recycling.paper
	  recycling.recycling_centre
	  shopping
	  shopping.beverages
	  shopping.books
	  shopping.computer
	  shopping.computer.software
	  shopping.computer.games
	  shopping.computer.hardware
	  shopping.dry_cleaner
	  shopping.electronics
	  shopping.flea_market
	  shopping.food.bakery
	  shopping.food.butcher
	  shopping.food.fruit
	  shopping.food.market
	  shopping.food.vegetables
	  shopping.furnishing_house
	  shopping.furniture
	  shopping.hardware_store
	  shopping.literature
	  shopping.shopping_center
	  shopping.sport
	  shopping.sport.diving
	  shopping.sport.ski
	  shopping.sport.tennis
	  shopping.supermarket
	  shopping.supermarket.aldi
	  shopping.supermarket.groceries
	  shopping.supermarket.lidl
	  shopping.supermarket.penny
	  shopping.supermarket.plus
	  shopping.supermarket.tengelmann
	  telephone
	  town_hall
	  toys
	  traffic.emergency_call
	  transport
	  transport.car
	  transport.car.garage
	  transport.car.gas_station
	  transport.car.gas_station.agip
	  transport.car.gas_station.aral
	  transport.car.gas_station.dea
	  transport.car.gas_station.esso
	  transport.car.gas_station.jet
	  transport.car.gas_station.omv
	  transport.car.gas_station.shell
	  transport.car.gas_station.texaco
	  transport.car.parking
	  transport.car.parking.handicapped
	  transport.car.parking.park_and_ride
	  transport.car.parkinglot
	  transport.car.phototrap_traffic_light
	  transport.car.rental
	  transport.car.rental.avis
	  transport.car.rental.herz
	  transport.car.rental.sixt
	  transport.car.rest_area
	  transport.car.toll_station
	  transport.car.wrecking_service
	  transport.car_loading_terminal
	  transport.coach
	  transport.container_loading_terminal
	  transport.driver-guide_service
	  transport.nautical.alpha_flag
	  transport.nautical.dive_flag
	  transport.park_and_ride
	  transport.public.airport
	  transport.public.airport.aeroplane
	  transport.public.airport.glider
	  transport.public.airport.helicopter
	  transport.public.ferry
	  transport.public.ferry.car
	  transport.public.interurban_train_station
	  transport.public.railway
	  transport.public.station.bus
	  transport.public.station.railroad
	  transport.public.station.subway_city
	  transport.public.station.subway_regional
	  transport.public.station.taxi
	  transport.public.station.tram
	  transport.public.station.underground
	  transport.scale
	  transport.ship
	  transport.ship.anker
	  transport.ship.dam
	  transport.ship.harbor
	  transport.traffic.construction
	  transport.traffic.speedtrap
	  transport.traffic.traffic_jam
	  transport.travel-agency
	  transport.truck
	  w-lan.open
	  w-lan.public
	  w-lan.wep
	  );

my $translate_de = { 
    'ATM'                   => 'Geldautomat',
    'EC'                    => 'EC-Automat',
    'HypoVereinsbank' 	=> 'HypoVereinsbank',
    'Postbank'              => 'Postbank',
    'Reifeisenbank'         => 'Reifeisenbank',
    'Sparda'                => 'Sparda',
    'Sparkasse'             => "Sparkasse",
    'WC'                    => 'WC',
    'accomodation'          => 'Uebernachtung',
    'administration'        => 'Verwaltung',
    'adult_education'       => 'VHS',
    'aeroplane'             => "Motor Flugzeug",
    'agency'                => "Filiale",
    'agip'                  => "Agip",
    'airport'               => 'Flugplatz',
    'aldi'                  => "Aldi",
    'alpha_flag'            => 'Taucher',
    'ambulance'             => 'Ambulanz',
    'amusement'             => "Freizeit",
    'anker'                 => "Anker",
    'aquarium'              => "Aquarium",
    'aral'                  => "Aral",
    'area'                  => "Gebiet",
    'area'                  => "Gebiet",
    'asia'                  => "Asiatisch",
    'avis'                  => "Avis",
    'bakery'                => "Bakery",
    'bank'                  => 'Bank',
    'bavarian'              => "Bayrisch",
    'beergarden'            => 'Biergarten',
    'beverages'             => "Getraenke",
    'bicycling'             => "Fahrrad",
    'boat'                  => "Boot",
    'bob'                   => "Bob",
    'books'                 => 'Buecher',
    'bowling'               => "Kegeln",
    'bridge'                => "Bruecke",
    'burger_king'           => 'Burger_King',
    'bus'                   => "Bus",
    'butcher'               => "Metzger",
    'cafe'                  => 'cafe',
    'campground'            => 'Campingplatz',
    'camping'               => "Camping",
    'car'                   => 'Auto',
    'car_loading_terminal'  => 'Auto_Verlade_Bahnhof',
    'car_registration'      => 'Zulassungsstelle',
    'catholic'              => 'Katholisch',
    'cemetary'              => 'Friedhof',
    'cemetery'              => "Friedhof",
    'chinese'               => "Chinesisch",
    'church'                => 'Kirche',
    'cinema'                => 'Kino',
    'city'                  => 'Stadt',
    'climbing'              => "Klettern",
    'coach'                 => 'Kutsche',
    'coast'                 => 'rodeln',
    'computer'              => 'Computer',
    'concert_hall'          => "Konzert_Halle",
    'construction'          => 'Baustelle',
    'container_loading_terminal' => 'Gueter_Verlade_Bahnhof',
    'country'               => "Land",
    'cross-country'         => "langlauf",
    'custom'                => 'Benutzer_Eigene',
    'dam'                   => "Wehr",
    'dance_floor'           => "Tanzen",
    'dart'                  => 'Dart',
    'dea'                   => "Dea",
    'delivery_service'      => 'Lieferservice',
    'developer'             => 'Entwickler',
    'disco'                 => 'disco',
    'dive_flag'             => 'Taucher',
    'diving'                => "Tauchen",
    'doctor'                => 'Arzt',
    'down-hill'             => "Abfahrt",
    'drinking_water'        => "Trinkwasser",
    'driver-guide_service'  => 'Lotsendienst',
    'dry_cleaner'           => "Reinigung",
    'dumpstation'           => "Dump Platz",
    'education'             => 'Ausbildung',
    'electronics'           => "Electronik",
    'emergency'             => 'Notaufnahme',
    'emergency_call'        => 'Notruf_saeule',
    'entrance'              => 'Eingang',
    'esso'                  => "Esso",
    'event_location'        => 'Veranstaltungshalle',
    'exhibition'            => 'Messe',
    'exhibitions'           => 'Ausstellungen',
    'exibition'             => "Ausstellung",
    'fairground'            => 'Rummelplatz',
    'fastfood'              => 'Schnellrestaurant',
    'ferry'                 => 'Faehre',
    'fire_department'       => 'Feuerwehr',
    'fishing'               => "angeln",
    'fittness'              => "Fittness",
    'flea_market'           => 'Flohmarkt',
    'food'                  => 'Essen',
    'forest'                => "Wald",
    'friends'               => 'Freunde',
    'fruit'                 => 'Obst',
    'furnishing_house'      => 'Einrichtungs_Haus',
    'furniture'             => "Moebel",
    'games'                 => "Spiele",
    'garage'                => 'Werkstatt',
    'gas_station'           => 'Tankstelle',
    'geocache'              => 'Geocache',
    'gift_shop'             => 'Geschenke_Laden',
    'glass'                 => 'Glas',
    'glider'                => "Selgelflugzeug",
    'golf_place'            => 'Golfplatz',
    'greek'                 => "Griechisch",
    'groceries'             => 'Lebensmittel',
    'hall'                  => "Halle",
    'handicapped'           => "Behinderte",
    'harbor'                => "Hafen",
    'hardware'              => 'Hardware',
    'hardware_store'        => 'Baumarkt',
    'health'                => 'Gesundheit',
    'helicopter'            => "Hubschrauber",
    'herz'                  => "Herz",
    'highschool'            => 'Gymnasium',
    'home'                  => 'Daheim',
    'horse'                 => "Pferde",
    'hospital'              => 'Krankenhaus',
    'hotel'                 => 'Hotel',
    'hunt'                  => "Jagd",
    'ice_cream_parlor'      => 'Eisdiele',
    'ice_scating'           => "Eislaufen",
    'import_way'            => 'import_way',
    'indian'                => "Indisch",
    'information'           => 'Information',
    'internet'              => 'internet',
    'interurban_train_station' => 'S-Bahn-Haltestelle',
    'italian'               => "Italienisch",
    'japanese'              => "japanisch",
    'jet'                   => "Jet",
    'kindergarden'          => 'Kindergarten',
    'lake'                  => "See",
    'landmark'              => 'Sehenswuerdigkeit',
    'letter-box'            => 'Briefkasten',
    'lidl'                  => "Lidl",
    'literature'            => 'Literatur',
    'market'                => "Markt",
    'mc_donalds'            => 'MC_Donalds',
    'mine'                  => "Miene",
    'minigolf_place'        => 'Minigolfplatz',
    'model_aircrafts'       => 'Modellflugplatz',
    'money_exchange'        => 'Geldwechsel',
    'monument'              => "Denkmal",
    'motel'                 => 'Motel',
    'motorbike'             => "Motorrad",
    'mountain'              => "Berg",
    'museum'                => 'Museum',
    'music'                 => "Music",
    'my'                    => "Meine",
    'national_park'         => 'Nationalpark',
    'nautical'              => 'Schiffahrt',
    'night_club'            => 'Nachtclub',
    'nursery'               => 'Kinderkrippe',
    'nursing_home'          => 'Altenheim',
    'office'                => 'Amt',
    'omv'                   => "Omv",
    'open'                  => 'Offen',
    'opera'                 => 'Oper',
    'paper'                 => 'Papier',
    'parachute'             => "Fallschirmspringen",
    'park'                  => "Park",
    'park_and_ride'         => 'Park_and_Ride',
    'parking'               => "Parken",
    'parkinglot'            => 'Parkplatz',
    'pedestrian'            => "Fussgaenger",
    'pedestrian_zone'       => 'Fussgaenger_Zone',
    'penny'                 => "Penny",
    'pharmacy'              => 'Apotheke',
    'phototrap_traffic_light' => 'Blitzampel',
    'picnic_area'           => "Rastplatz",
    'pizza'                 => "Pizza",
    'pizza_hut'             => "Pizza_Hut",
    'play_street'           => "Spiel Strasse",
    'playground'            => 'Spielplatz',
    'plus'                  => "Plus",
    'plus'                  => "Plus",
    'police'                => 'Polizei',
    'postal'                => 'Post',
    'postoffice'            => 'Post',
    'primary_school'         => 'GrundSchule',
    'protestant'            => "evangelisch",
    'pub'                   => 'Kneipe',
    'public'                => 'oeffentlich',
    'racing'                => "Rennen",
    'railroad'              => "Eisenbahn",
    'railway'               => "Eisenbahn",
    'recreation'            => 'Freizeit',
    'recycling'             => 'Recycling',
    'recycling_centre'      => 'Wertstoffhof',
    'rental'                => "Vermietung",
    'rest_area'             => 'Raststaette',
    'restaurant'            => 'Gaststaette',
    'restaurant'            => 'Restaurant',
    'riding'                => 'Reiten',
    'rock_climbing'         => "Fels klettern",
    'rocks'                 => "Fels",
    'rollerblades'          => "Rollerblades",
    'rollerscates'          => "Rollschuhe",
    'scale'                 => "Waage",
    'secondary_school'      => 'HauptSchule',
    'shell'                 => "Shell",
    'ship'                  => "Boot",
    'shopping'              => 'Einkaufen',
    'shopping_center'       => 'Einkaufszentrum',
    'shower'                => "Dusche",
    'sixt'                  => "Sixt",
    'ski'                   => "Ski",
    'skiing'                => 'Skifahren',
    'snakeboard'            => "Snakeboard",
    'soccer_field'          => 'Fussballplatz',
    'soccer_stadion'        => 'Fussball_Stadion',
    'software'              => 'Software',
    'speedtrap'             => 'Radarfalle',
    'sport'                 => "Sport",
    'sports'                => 'Sport',
    'state'                 => "Staat",
    'station'               => "Station",
    'subway_city'           => "U-Bahn",
    'subway_regional'       => "S-Bahn",
    'supermarket'           => 'Supermarkt',
    'swimming'              => "schwimmen",
    'swimming_area'         => 'Schwimmbad',
    'synagoge'              => 'Synagoge',
    'taxi'                  => "Taxi",
    'taxi_stand'            => 'Taxi-Stand',
    'telephone'             => 'Telefon-Zelle',
    'tengelmann'            => "Tengelmann",
    'tennis'                => "Tennis",
    'tennis_place'          => 'Tennisplatz',
    'tent'                  => "Zelt",
    'texaco'                => "Texaco",
    'theater'               => "Theater",
    'theatre'               => 'Theater',
    'toll_station'          => 'Mautstation',
    'town_hall'             => 'Stadthalle',
    'toys'                  => 'Spielwaren',
    'trade_show'            => "Handels_Messe",
    'traffic'               => 'Verkehr',
    'traffic_jam'           => 'Stau',
    'trailer'               => "Wohnwagen",
    'train'                 => 'Zug',
    'tram'                  => "Strassenbahn",
    'tram_station'          => 'Strassen-Bahn_Haltestelle',
    'transport'             => 'Transport',
    'travel-agency'         => "Reisebuero",
    'truck'                 => "Lastwagen",
    'tunnel'                => "Tunnel",
    'txt'                   => 'txt',
    'underground'           => "U-Bahn",
    'underground_station'   => 'U-Bahn_Haltestelle',
    'university'            => 'UNI',
    'unknown'               => 'unknown',
    'vegetables'            => 'Gemuese',
    'w-lan'                 => 'W-LAN',
    'water'                 => "Wasser",
    'water_ski'             => "Wasser Ski",
    'wep'                   => 'WEP',
    'work'                  => 'Arbeit',
    'wrecking_service'      => "Abschleppdienst",
    'youth_hostel'          => 'Jugend_Herberge',
    'zoo'                   => 'Zoo',
};

# ------------------------------------------------------------------
# try to do a simple translation of an icon name
my $unknown_translations ='';
sub translate_icon($){
    my $name = shift;
    # Translate the entries
    my $name_de ='';
    for my $part ( split(/\./,$name)){
	if ( ! defined( $translate_de->{$part}) ) {
	    $unknown_translations .= 
		sprintf("	%-23s => \"".lc($part)."\",\n","'".$part."'");
	} else {
	    $name_de .= ".$translate_de->{$part}";
	}
    }
    $name_de    =~ s/^\.//;
    $name_de    =~ s/_/ /g;
    $name =~ s/_/ /g;
    return $name_de;	
}

# ------------------------------------------------------------------
# Find Icon
my $used_icons ={};
sub find_icon($$){
    my $name = shift;
    my $counter = shift;

    my $icon;
    my $icon_name = "$name.png";
    while ( $icon_name =~ s/\.[^\.]+$// ){ # cut off last part
	if ( $icons->{$icon_name} ) {
	    $icon = $icons->{$icon_name};
	    last;
	}
    }
    $icon =  "unknown" unless defined $icons->{$icon_name};

    $icon =~ s/ /_/g;
    if ( -s "../data/icons/$icon.png" ) { $icon .= ".png" };
    if ( -s "../data/icons/$icon.gif" ) { $icon .= ".gif" };

    $used_icons->{"../data/icons/$icon"} += $counter;
    
    #debug($icon);
    return $icon;
}

# -----------------------------------------------------------------------------
# Fill generate_poi_type_html_page
sub generate_poi_type_html_page() {
    $used_icons ={};
    
    my $generate_poi_type_html_page= (-d '../data/icons/');

    if ( ! $generate_poi_type_html_page ) {
	die "Icon directory was not found\n";
    }

    my $poi_type_html_page = IO::File->new(">../data/icons/index.html")
	or warn "Cannot open html Page for writing: $!";;
    my $poi_type_txt_file = IO::File->new(">../data/icons.txt")
	or warn "Cannot open txt file for writing: $!";;
    print $poi_type_html_page "<html>\n";
    print $poi_type_html_page "<body bgcolor=\"#AAAAFF\">\n";
    print $poi_type_html_page "<table border>\n";
    print $poi_type_html_page "<th>icon</th>	<th>filename</th>\n";
    print $poi_type_html_page "<th>name</th>	<th>description</th>\n";
#   print $poi_type_html_page "<th>name_de</th>	";
    print $poi_type_html_page "<th>description_de</th>\n";

    for my $icon ( glob("../data/icons/*.{png,gif}" ) ) {
	next unless -f $icon;
	$used_icons->{$icon}=0;
    }

    my $poi_type_id=0;
    for my $name_raw  ( sort @poi_type_names ) {
	$poi_type_id++;
	
	my $name =$name_raw;

	# Translate the entries
	my $name_de =translate_icon($name);

	$name =~ s/_/ /g;
	
	# Find Icon
	my $icon = find_icon($name,1);

	# create description
	my $description     = $name;    $description    =~ s/\./ /g;
	my $description_de  = $name_de; $description_de =~ s/\./ /g;

	# create html page for icons
	print $poi_type_html_page "<tr> ";
	if ( $icon =~ m/unknown/ ) {
	    print $poi_type_html_page "<td align=RIGHT ><img src=\"$icon\"></td>";
	} else {
	    print $poi_type_html_page "<td valign=TOP ><img src=\"$icon\"></td>";
	}
	print $poi_type_html_page "<td valign=TOP >$icon</td>";
	
	print $poi_type_html_page "<td valign=TOP >$name</td>";
	    print $poi_type_html_page "<td valign=TOP >$description</td>";
#	    print $poi_type_html_page "<td>$name_de</td>";
	print $poi_type_html_page "<td valign=TOP >$description_de</td>";
	print $poi_type_html_page "</tr> \n";
	
	print $poi_type_txt_file "$name_raw $icon\n";
	debug(sprintf("%3d %-20s %-45s %-45s %-45s %-45s ",
		      $poi_type_id,$icon,$name,$name_de,$description,$description_de));
    } # of for @poi_type_names
    
    print $poi_type_html_page "</table>\n";
    for my $type ( qw(used system unused ) ){
	print $poi_type_html_page "<H2>$type</h2>\n";
	my $col_count=0;
	print $poi_type_html_page "<table border=1>\n";
	for my $icon ( sort keys %{$used_icons} ) {
	    my $system_icon =0;
	    $system_icon = 1 if $icon =~ m/\/(battery|compass|friendsicon|gauge|gpsicon|gpsiconbt|kismet|powercharges|powercord|talogo|)\.(png|gif)$/;
	    next if $icon =~ m/gpsdrivesplash|gpsdrivemini/;
		next if $type eq "used"   && ! $used_icons->{$icon};
	    next if $type eq "system" && ! $system_icon;
	    next if $type eq "unused" &&  ($used_icons->{$icon} || $system_icon);
	    print $poi_type_html_page "<tr>" 
		if $col_count ==0;
	    if ( $used_icons->{$icon} || $icon =~ m,/unknown\....,) {
		printf $poi_type_html_page "<td>%d *</td>",$used_icons->{$icon};
	    }
	    my $icon_name = $icon ;
	    $icon_name =~ s,.*/,,;
	    print $poi_type_html_page "<td><img src=\"$icon_name\"></td>";
	    print $poi_type_html_page "<td>$icon_name</td>";
	    
	    $col_count++;
		if ( $col_count>4 ) {
		    $col_count=0;
		    print $poi_type_html_page "</tr>";
		}
	}
	print $poi_type_html_page "</table>\n";
    }

    print $poi_type_html_page "</html> ";
    $poi_type_html_page->close();
    $poi_type_txt_file->close();    

    if ( $unknown_translations ) {
	print "# Missing poi_type Translations:\n";
	print "$unknown_translations\n";
	print "\n";
    }
}


# -----------------------------------------------------------------------------
# Fill poi_type database
sub fill_default_poi_types() {
    my $i=1;
    $used_icons ={};
    
    # for debug purpose
    Geo::Gpsdrive::DBFuncs::db_exec("TRUNCATE TABLE `poi_type`;");

    my $used_icons ={};
    for my $icon ( glob("../data/icons/*.{png,gif}" ) ) {
	next unless -f $icon;
	$used_icons->{$icon}=0;
    }

    my $poi_type_id=0;
    for my $name  ( @poi_type_names ) {
	$poi_type_id++;

	# Translate the entries
	my $name_de = translate_icon($name);

	$name =~ s/_/ /g;
	
	# Find Icon
	my $icon = find_icon($name,0);

	# create description
	my $description     = $name;    $description    =~ s/\./ /g;
	my $description_de  = $name_de; $description_de =~ s/\./ /g;

	# Insert to Database
	Geo::Gpsdrive::DBFuncs::db_exec("DELETE FROM `poi_type` WHERE poi_type_id = $poi_type_id ;");
	Geo::Gpsdrive::DBFuncs::db_exec("INSERT INTO `poi_type` ".
					"       (poi_type_id, name,name_de, symbol, description,description_de ) \n".
					"	VALUES ($poi_type_id,'$name','$name_de','$icon','$description','$description_de');") 
	    or die;
	$i++;
    } # of for @poi_type_names
    
    if ( $unknown_translations ) {
	print "# Missing poi_type Translations:\n";
	print "$unknown_translations\n";
	print "\n";
    }
}

# -----------------------------------------------------------------------------
sub fill_default_sources() {   # Just some Default Sources

    # -------------------------------------------- NGA
    my $coutry2name;
    for my $k ( keys %{$Geo::Gpsdrive::NGA::name2country} ) {
	$coutry2name->{$Geo::Gpsdrive::NGA::name2country->{$k}} =$k;
    }

    for my $country  ( @Geo::Gpsdrive::NGA::countries ) {    
	my $name ="earth-info.nga.mil $country";

	Geo::Gpsdrive::DBFuncs::db_exec("DELETE FROM `source` WHERE source.name = '$name';");
	my $source_hash = {
	    'source.url'     => "http://www.evl.uic.edu/pape/data/WDB/WDB-text.tar.gz",
	    'source.name'    => $name,
	    'source.comment' => "GeoData for $coutry2name->{$country}($country)",
	    'source.licence' => ""
	    };
	Geo::Gpsdrive::DBFuncs::insert_hash("source", $source_hash);
    }

    # -------------------------------------------- import_way.txt, Default, Example
    for my $source  ( qw( import_way.txt
			  Defaults
			  Examples
			  ) ) {    
	my $name ="$source";
	$name =~ s/_/ /g;

	Geo::Gpsdrive::DBFuncs::db_exec("DELETE FROM `source` WHERE source.name = '$name';");
	my $source_hash = {
	    'source.url'     => "",
	    'source.name'    => $name,
	    'source.comment' => "$name",
	    'source.licence' => ""
	    };
	Geo::Gpsdrive::DBFuncs::insert_hash("source", $source_hash);
    }
}

# -----------------------------------------------------------------------------
sub fill_example_street_types() {   # Fill streets_type database
    my $i=1;

    my $lang;
    my $name;
    my $color;

    my $streets_type_id=0;

    # Entries for WDB
    for my $kind ( qw(bdy cil riv) ) {
	for my $rank ( 1.. 20 ) {
	    $color = "000000";
	    $color = "0000FF" if $kind eq"riv"; # Water
	    $color = "001010" if $kind eq"bdy"; # 
	    $color = "004400" if $kind eq"cil"; # 
	    $lang = "de";
	    $name = "WDB $kind rank ${rank}";
	    my $linetype='';
	    $streets_type_id++;
	    Geo::Gpsdrive::DBFuncs::db_exec("DELETE FROM `streets_type` WHERE streets_type_id = '$streets_type_id' ;");
	    Geo::Gpsdrive::DBFuncs::db_exec("INSERT INTO `streets_type` ".
					    "        (streets_type_id, name, description , description_de , color , linetype )".
					    " VALUES ($i,'$name','$name','$name','$color','$linetype');");
	    
	    $i++;
	}
    }

    # Just some Default types
    # TODO: make english primary language
    my @streets_types = qw( de.unbekannt_xFFFFFF
			    de.Strassen.Allgemein_x111111
			    de.Strassen.Wanderweg_x111111
			    de.Strassen.Fussweg_x111111
			    de.Strassen.30_Zohne_x00FFFF
			    de.Strassen.Autobahn_xFF0000
			    de.Strassen.Bundesstrasse_xFFFF00
			    de.Strassen.Innerorts_x00FFFF
			    de.Strassen.Landstrasse_xFFFF00
			    de.Strassen.Highway_x00FFFF
			    de.Schiene.ICE_Trasse_x000000
			    de.Schiene.Zug_Trasse_x000000
			    de.Schiene.S_Bahn_Trasse_x000000
			    de.Schiene.U_Bahn_Trasse_x000000
			    de.Schiene.Bus_Trasse_x000000
			    de.Grenzen.Landesgrenze_x000000
			    de.Grenzen.Bundesland_Grenze_x000000
			    de.Grenzen.Stand_Grenze_x000000
			    de.Wasser.Fluss_x0000FF
			    de.Wasser.Kueste_x000FFF
			    );

    for my $entry  ( @streets_types ) {    
	$entry =~ m/^(..)\.(.*)_x(......)$/;
	$lang  = $1;
	$name  = $2;
	$color = $3;
	if ( ! $lang ) {
	    die "Error in street type entry '$entry'\n";
	}
	$lang ||= 'en';
	$name =~ s/_/ /g;
	my $linetype='';

	$streets_type_id++;
	Geo::Gpsdrive::DBFuncs::db_exec("DELETE FROM `streets_type` WHERE streets_type_id = '$streets_type_id';");
	Geo::Gpsdrive::DBFuncs::db_exec("INSERT INTO `streets_type` ".
					"        (streets_type_id,  name, description , color , linetype )".
					" VALUES ($i,'$name','$name','$color','$linetype');");
	
	$i++;
    }

    # Reserve Type 1000 so users pace there id's behind it
    $streets_type_id =1000;
    Geo::Gpsdrive::DBFuncs::db_exec("DELETE FROM `streets_type` WHERE streets_type_id = '$streets_type_id';");
    Geo::Gpsdrive::DBFuncs::db_exec("INSERT INTO `streets_type` ".
				    "        (streets_type_id,  name, description , color , linetype )".
				    " VALUES ($streets_type_id,'Reserved','Reserved','0x000000','');");
    
} # of fill_example_street_types()

# -----------------------------------------------------------------------------

sub fill_defaults(){
    print "Create Defaults ...\n";
    fill_default_poi_types();
    fill_default_sources();
    fill_example_street_types();
    print "Create Defaults completed\n";
}

1;
