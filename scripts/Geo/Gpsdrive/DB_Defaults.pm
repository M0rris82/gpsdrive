# Database Defaults for poi/streets Table for poi.pl

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
    '1' 			   	=> 'ppl1',
    '2' 			   	=> 'ppl2',
    '3' 			   	=> 'ppl3',
    '4' 			   	=> 'ppl4',
    '5' 			   	=> 'ppl5',
    'accomodation' 	           	=> "hotel",
    'accomodation.campground'		=> 'camping',
    'area'				=> 'area',
    'area.area'				=> 'area',
    'area.area.lake'			=> 'area-blue',
    'area.area.forest'			=> 'trees',
    'area.area.hill' 			=> 'summit',
    'area.area.hill.large'              => "summit-3",
    'area.area.hill.medium'             => "summit-2",
    'area.area.hill.mountain'         	=> "summit-4",
    'area.area.hill.small'              => "summit-1",
    'area.bridge'                     	=> 'bridge',
    'area.bridge.car'                 	=> 'bridge_car',
    'area.bridge.pedestrian'          	=> 'bridge_pedestrian',
    'area.bridge.train'               	=> 'bridge_train',
    'area.city' 			=> 'area_city',
    'area.forest'                     	=> 'trees',
    'area.pedestrian zone'            	=> "pedestrian",
    'area.play_street'                	=> "play_street",
    'bank'                            	=> "Dollar",
    'bank.ATM.EC'                     	=> "ec",
    'bank.agency'	            	=> 'bank-agency',
    'bank.agency.Postbank'            	=> 'post',
    'bank.agency.Reifeisenbank'       	=> "VR",
    'camping.water'                   	=> 'tap',
    'church'                          	=> "church",
    'church.cemetary'                 	=> 'cemetary',
    'circle' 			      	=> 'ppl',
    'custom.friends.home' 	        => "home",
    'custom.friends.work' 	        => "work",
    'custom.my.home'  	          	=> "home",
    'custom.my.work' 	          	=> "work",
    'education.school.primary school'   => 'ABC',
    'education.school.university'       => 'emc2',
    'food' 		                => "food",
    'food.beergarden'                   => 'beergarden',
    'food.cafe'                         => "cafe",
    'food.cafe.internet'                => "cafe",
    'food.drinking water'               => 'tap_drinking',
    'food.ice cream parlor'		=> "ice_cream",
    'food.restaurant.asia'            	=> 'japanese',
    'food.restaurant.bavarian'        	=> 'beergarden',
    'food.restaurant.chinese'         	=> 'japanese',
    'food.restaurant.fastfood.burger king'=> "burgerking",
    'food.restaurant.fastfood.mc donalds' => "mcdonalds",
    'food.restaurant.fastfood.pizza_hut'=> 'pizza',
    'food.restaurant.ice cream parlor'	=> 'ice_cream',
    'food.restaurant.japanese'        	=> 'japanese',
    'food.restaurant.pizza'           	=> 'pizza',
    'general.caution'            	=> 'warning',
    'general.marker-1'		        => 'ppl1',
    'general.marker-2'		        => 'ppl2',
    'general.marker-3'		        => 'ppl3',
    'general.marker-4'		        => 'ppl4',
    'general.marker-5'		        => 'ppl5',
    'health'                          	=> "red_cross",
    'health.doctor'                   	=> 'aesculab',
    'health.emergency'                	=> 'emergency',
    'health.hospital'                 	=> 'hospital',
    'health.pharmacy'                 	=> "pharmacy",
    'information'                     	=> "info",
    'mine' 			        => 'mine',
    'open' 			        => 'open',
    'postal'                          	=> 'post',
    'postal.letter-box'               	=> 'letter',
    'postal.office'                   	=> 'letter1',
    'recreation'                      	=> 'balloon',
    'recreation.cinema'               	=> 'movies',
    'recreation.geocache' 	        => "geocache",
    'recreation.landmark' 	        => "monument",
    'recreation.music'                	=> 'music',
    'recreation.music.opera'          	=> 'masks',
    'recreation.music.theater'        	=> 'masks',
    'recreation.night club' 	  	=> "girls",
    'recreation.playground'           	=> 'swing',
    'recreation.pub'                  	=> 'beer',
    'recreation.pub.dart'             	=> "dart",
    'recreation.sports.bicycling' 	=> 'bicycling',
    'recreation.sports.boat'           	=> "boat",
    'recreation.sports.diving'        	=> 'alpha_flag',
    'recreation.sports.golf place'     	=> "golf",
    'recreation.sports.horse.racing'    => 'horse',
    'recreation.sports.horse.riding'    => 'horse',
    'recreation.sports.motorbike'      	=> "motorbike",
    'recreation.sports.skiing' 	  	=> 'skiing',
    'shopping'                        	=> "shop",
    'shopping.computer'			=> 'shop-computer',
    'shopping.food.bakery'            	=> 'prezel',
    'shopping.supermarket.lidl'       	=> "lidl_logo",
    'summit'                          	=> 'summit',
    'telephone'                       	=> 'phone',
    'transport.airport'               	=> "airport",
    'transport.car'                     => "car",
    'transport.car.garage'            	=> "car_repair",
    'transport.car.gas station'       	=> "fuel",
    'transport.car.gas station.agip'  	=> "fuel.agip",
    'transport.car.parking'		=> "car-parking",
    'transport.car.parking.handicapped'	=> "parking-handicapped",
    'transport.car.parking.park_and_ride'=> "P+R",
    'transport.car.parkinglot'        	=> "car-parking",
    'transport.car.phototrap traffic light' => "speed_trap",
    'transport.car.rental'            	=> 'car_rental',
    'transport.car.rental.sixt'       	=> 'sixt',
    'transport.car.rest area' 	  	=> 'rest_area',
    'transport.car.wrecking service'  	=> 'towing',
    'transport.nautical.alpha flag'   	=> 'alpha_flag',
    'transport.nautical.dive flag'    	=> 'dive_flag',
    'transport.park and ride'         	=> 'P+R',
    'transport.public.airport'        	=> 'airport',
    'transport.public.bus station'   	=> 'bus_stop',
    'transport.public.ferry'          	=> 'ferry',
    'transport.public.ferry.car'       	=> 'ferry-car',
    'transport.public.railway'        	=> 'train',
    'transport.public.station.bus'     => 'bus_stop',
    'transport.public.station.railroad' => 'train',
    'transport.public.station.subway city'    => 'U-Bahn',
    'transport.public.station.subway regional'=> 'S-Bahn',
    'transport.public.station.tram'     => 'bus_stop',
    'transport.ship'	              	=> 'boat',
    'transport.ship.anker'              => 'anker',
    'transport.traffic.construction'  	=> "warning",
    'transport.traffic.signs.dead end' 	=> 'dead_end',
    'transport.traffic.signs.play street'=> 'play_street',
    'transport.traffic.signs.right of way' => 'right_of_way',
    'transport.traffic.signs.stop'	=> 'stop',
    'transport.traffic.signs.trafficlight' => "traffic-light",
    'transport.traffic.signs.speedlimit.end' => '30-end',
    'transport.traffic.speedtrap'     	=> "speed_trap",
    'transport.traffic.traffic jam'   	=> "warning",
    'transport.truck'                 	=> 'truck',
    'w-lan.open' 	                => "wlan",
    'w-lan.public' 	                => "tower",		
    'w-lan.wep' 	                => "wlan-wep",
    'entrance'				=> 'door',
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
	  area.area.country
	  area.area.farm
	  area.area.forest
	  area.area.hill
	  area.area.hill.large
	  area.area.hill.medium
	  area.area.hill.small
	  area.area.industial-area
	  area.area.lake
	  area.area.hill.mountain
	  area.area.play_street
	  area.area.river
	  area.bridge
	  area.bridge.car
	  area.bridge.pedestrian
	  area.bridge.train
	  area.building
	  area.building.barn
	  area.building.farmhouse
	  area.city
	  area.city.capital:-3000000000
	  area.city.hamlet
	  area.city.large:-3000000000
	  area.city.mayor:-300000000
	  area.city.medium:-30000000
	  area.city.small
	  area.pedestrian_zone
	  area.state
	  area.tunnel
	  area.railway-crossing
	  bank
	  bank.ATM
	  bank.ATM.EC
	  bank.agency
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
	  education.kindergarden
	  education.nursery
	  education.school
	  education.school.highschool
	  education.school.primary_school
	  education.school.secondary_school
	  education.school.university
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
	  general
	  general.caution
	  general.marker-1
	  general.marker-2
	  general.marker-3
	  general.marker-4
	  general.marker-5
	  general.viewpoint
	  general.waypoint
	  general.point-of-interest
	  general.memorial
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
	  shopping.computer.games
	  shopping.computer.hardware
	  shopping.computer.software
	  shopping.dry_cleaner
	  shopping.electronics
	  shopping.flea_market
	  shopping.food.bakery
	  shopping.food.butcher
	  shopping.food.fruit
	  shopping.food.market
	  shopping.food.tea
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
	  transport.public.station.lift
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
	  transport.traffic.signs
	  transport.traffic.signs.dead_end
	  transport.traffic.signs.oneway
	  transport.traffic.signs.play_street
	  transport.traffic.signs.right_of_way
	  transport.traffic.signs.speedlimit
	  transport.traffic.signs.speedlimit.100_kmh
	  transport.traffic.signs.speedlimit.120_kmh
	  transport.traffic.signs.speedlimit.30_kmh
	  transport.traffic.signs.speedlimit.50_kmh
	  transport.traffic.signs.speedlimit.60_kmh
	  transport.traffic.signs.speedlimit.80_kmh
	  transport.traffic.signs.speedlimit.end
	  transport.traffic.signs.stop
	  transport.traffic.signs.trafficlight
	  transport.traffic.speedtrap
	  transport.traffic.traffic_jam
	  transport.travel-agency
	  transport.truck
	  w-lan.open
	  w-lan.public
	  w-lan.wep
	  );

my $translate_de = { 
    '100_kmh'               => "100_kmh",
    '120_kmh'               => "120_kmh",
    '30_kmh'                => "30_kmh",
    '50_kmh'                => "50_kmh",
    '60_kmh'                => "60_kmh",
    '80_kmh'                => "80_kmh",
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
    'barn'                  => "Heuschober",
    'bavarian'              => "Bayrisch",
    'beergarden'            => 'Biergarten',
    'beverages'             => "Getraenke",
    'bicycling'             => "Fahrrad",
    'boat'                  => "Boot",
    'bob'                   => "Bob",
    'books'                 => 'Buecher',
    'bowling'               => "Kegeln",
    'bridge'                => "Bruecke",
    'building'              => "Gebaeude",
    'burger_king'           => 'Burger_King',
    'bus'                   => "Bus",
    'butcher'               => "Metzger",
    'cafe'                  => 'cafe',
    'campground'            => 'Campingplatz',
    'camping'               => "Camping",
    'capital'               => "Hauptstadt",
    'car'                   => 'Auto',
    'car_loading_terminal'  => 'Auto_Verlade_Bahnhof',
    'car_registration'      => 'Zulassungsstelle',
    'catholic'              => 'Katholisch',
    'caution'               => "Achtung",
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
    'dead_end'              => "Sackgasse",
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
    'end'                   => "Ende",
    'entrance'              => 'Eingang',
    'esso'                  => "Esso",
    'event_location'        => 'Veranstaltungshalle',
    'exhibition'            => 'Messe',
    'exhibitions'           => 'Ausstellungen',
    'exibition'             => "Ausstellung",
    'fairground'            => 'Rummelplatz',
    'farm'                  => "Felder",
    'farmhouse'             => "Bauernhof",
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
    'general'               => "Allgemein",
    'geocache'              => 'Geocache',
    'gift_shop'             => 'Geschenke_Laden',
    'give_way'              => "Vorfahrt Achten",
    'glass'                 => 'Glas',
    'glider'                => "Selgelflugzeug",
    'golf_place'            => 'Golfplatz',
    'greek'                 => "Griechisch",
    'groceries'             => 'Lebensmittel',
    'hall'                  => "Halle",
    'hamlet'                => "kleineres Dorf",
    'handicapped'           => "Behinderte",
    'harbor'                => "Hafen",
    'hardware'              => 'Hardware',
    'hardware_store'        => 'Baumarkt',
    'health'                => 'Gesundheit',
    'helicopter'            => "Hubschrauber",
    'herz'                  => "Herz",
    'highschool'            => 'Gymnasium',
    'hill'                  => 'Berg',
    'home'                  => 'Daheim',
    'horse'                 => "Pferde",
    'hospital'              => 'Krankenhaus',
    'hotel'                 => 'Hotel',
    'hunt'                  => "Jagd",
    'ice_cream_parlor'      => 'Eisdiele',
    'ice_scating'           => "Eislaufen",
    'import_way'            => 'import_way',
    'indian'                => "Indisch",
    'industial-area'        => "Industrie_Gebiet",
    'information'           => 'Information',
    'internet'              => 'internet',
    'interurban_train_station' => 'S-Bahn-Haltestelle',
    'italian'               => "Italienisch",
    'japanese'              => "japanisch",
    'jet'                   => "Jet",
    'kindergarden'          => 'Kindergarten',
    'lake'                  => "See",
    'landmark'              => 'Sehenswuerdigkeit',
    'large'                 => "Gross",
    'letter-box'            => 'Briefkasten',
    'lidl'                  => "Lidl",
    'lift'                  => "Lift",
    'literature'            => 'Literatur',
    'marker-1'              => "Marke_1",
    'marker-2'              => "Marke_2",
    'marker-3'              => "Marke_3",
    'marker-4'              => "Marke_4",
    'marker-5'              => "Marke 5",
    'market'                => "Markt",
    'mayor'                 => "haupt",
    'mc_donalds'            => 'MC_Donalds',
    'medium'                => "mittel",
    'memorial'              => "Denkmal",
    'mine'                  => "Miene",
    'minigolf_place'        => 'Minigolfplatz',
    'model_aircrafts'       => 'Modellflugplatz',
    'money_exchange'        => 'Geldwechsel',
    'monument'              => "Denkmal",
    'motel'                 => 'Motel',
    'motorbike'             => "Motorrad",
    'mountain'              => "Gebirge",
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
    'oneway'                => "Einbahn",
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
    'point-of-interest'     => "Interesanter Punkt",
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
    'railway-crossing'      => "Bahnuebergang",
    'recreation'            => 'Freizeit',
    'recycling'             => 'Recycling',
    'recycling_centre'      => 'Wertstoffhof',
    'rental'                => "Vermietung",
    'rest_area'             => 'Raststaette',
    'restaurant'            => 'Gaststaette',
    'restaurant'            => 'Restaurant',
    'riding'                => 'Reiten',
    'right_of_way'          => "Vorfahrt Achten",
    'river'                 => "Fluss",
    'rock_climbing'         => "Fels klettern",
    'rocks'                 => "Fels",
    'rollerblades'          => "Rollerblades",
    'rollerscates'          => "Rollschuhe",
    'scale'                 => "Waage",
    'school'                => "Schule",
    'secondary_school'      => 'HauptSchule',
    'shell'                 => "Shell",
    'ship'                  => "Boot",
    'shopping'              => 'Einkaufen',
    'shopping_center'       => 'Einkaufszentrum',
    'shower'                => "Dusche",
    'signs'                 => "Zeichen",
    'sixt'                  => "Sixt",
    'ski'                   => "Ski",
    'skiing'                => 'Skifahren',
    'small'                 => "klein",
    'snakeboard'            => "Snakeboard",
    'soccer_field'          => 'Fussballplatz',
    'soccer_stadion'        => 'Fussball_Stadion',
    'software'              => 'Software',
    'speedlimit'            => "Geschwindigkeitbeschraenkung",
    'speedtrap'             => 'Radarfalle',
    'sport'                 => "Sport",
    'sports'                => 'Sport',
    'state'                 => "Staat",
    'station'               => "Station",
    'stop'                  => "Stop",
    'subway_city'           => "U-Bahn",
    'subway_regional'       => "S-Bahn",
    'supermarket'           => 'Supermarkt',
    'swimming'              => "schwimmen",
    'swimming_area'         => 'Schwimmbad',
    'synagoge'              => 'Synagoge',
    'taxi'                  => "Taxi",
    'taxi_stand'            => 'Taxi-Stand',
    'tea'                   => "Tee",
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
    'trafficlight'          => "VerkehrsAmpel",
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
    'viewpoint'             => "Aussichtspunkt",
    'w-lan'                 => 'W-LAN',
    'water'                 => "Wasser",
    'water_ski'             => "Wasser Ski",
    'waypoint'              => "Wegpunkt",
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
    if ( -s "../data/icons/$icon.png" ) {
	$icon .= ".png";
    } elsif ( -s "../data/icons/$icon.gif" ) {
	$icon .= ".gif";
    } else {
	print "$icon not found\n";
    };

    $used_icons->{"../data/icons/$icon"} += $counter;
    
    #debug($icon);
    return $icon;
}



# -----------------------------------------------------------------------------
# Fill poi_type database
sub fill_default_poi_types() {
    my $i=1;
    $used_icons ={};
    
    # for debug purpose
    Geo::Gpsdrive::DBFuncs::db_exec("TRUNCATE TABLE `poi_type`;");

    my $unused_icon ={};
    my $existing_icon ={};
    my $icon_directory='../data/icons';
    $icon_directory = '../share/gpsdrive/icons'  unless -d $icon_directory;
    $icon_directory = '/usr/local/share/gpsdrive'  unless -d $icon_directory;
    $icon_directory = '/usr/share/gpsdrive/icons'  unless -d $icon_directory;
    $icon_directory = '/opt/gpsdrive/icons'  unless -d $icon_directory;
    die "no Icon Directory found" unless -d $icon_directory;
    for my $icon ( `find $icon_directory -name "*.png"` ) {
	chomp $icon;
	next if $icon =~ m,icons/classic/,;
	next unless -s $icon;
	$icon =~s,([^/]+/){4},,;
	$unused_icon->{$icon}++;
	$existing_icon->{$icon}++;
	print "icon: $icon\n" if $debug;
    }

    my $icon_file='../data/icons.txt';
    $icon_file = '../share/gpsdrive/icons.txt'    unless -s $icon_file;
    $icon_file = '/usr/local/share/gpsdrive'      unless -s $icon_file;
    $icon_file = '/usr/share/gpsdrive/icons.txt'  unless -s $icon_file;
    $icon_file = '/opt/gpsdrive/icons.txt'        unless -s $icon_file;
    die "no Icon File found" unless -s $icon_file;
    my $fh = IO::File->new("<$icon_file");
    my $poi_type_id=0;
    while ( my $line = $fh->getline() ) {
	chomp $line;
	next if $line =~  m/^\#/;
	my ($name,$scale_min,$scale_max) = split(/\s+/,$line);
	#print "($name,$scale_min,$scale_max)\n";
	my $icon = $name;
	$icon =~ s,\.,\/,g;
	$icon = "$icon.png";
	warn "Icon $icon missing\n"
	    unless $existing_icon->{$icon};
	delete $unused_icon->{$icon};

	$poi_type_id++;

	# Insert to Database
	Geo::Gpsdrive::DBFuncs::db_exec("DELETE FROM `poi_type` WHERE poi_type_id = $poi_type_id ;");
	Geo::Gpsdrive::DBFuncs::db_exec("INSERT INTO `poi_type` ".
					"              (poi_type_id,  name,   scale_min,   scale_max ) \n".
					"	VALUES ($poi_type_id,'$name','$scale_min','$scale_max');") 
	    or die;
	$i++;
    }
    warn "unused Icons:\n\t".join("\n\t",keys %{$unused_icon})."\n"
	if keys %{$unused_icon} ;

    $fh->close();
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
sub fill_default_street_types() {   # Fill streets_type database
    my $i=1;
    my $streets_type_id=0;

    # ------------ Entries for WDB
    for my $area ( qw(africa asia europe namer samer ) ) {
	for my $kind ( qw(bdy cil riv pby) ) {
	    for my $rank ( 1 .. 15 ) {
		my $name;
		my $color;
		my $color_bg  = "#000000";
		my $width     = 2;
		my $width_bg  = 0;
		my $scale_min =                             1;
		my $scale_max =                        100000;
		if ( $rank == 1 ) { $scale_max    = 100000000; $width=2 };
		if ( $rank == 2 ) { $scale_max    =   1000000; $width=2};
		if ( $rank == 3	) { $scale_max    =    100000; $width=3};
		if ( $rank == 4 ) { $scale_max    =    100000; $width=2};
		if ( $rank == 5 ) { $scale_max    =    100000; $width=1};
		$color = "#000000";
		if ( $kind eq "riv" ) { $color = "#0000FF"; $width=4; }; # riv  rivers
		if ( $kind eq "cil" ) { $color = "#0044FF";           }; # cil  coastlines, islands, and lakes
		if ( $kind eq "bdy" ) {	$color = "#001010";           }; # bdy  national boundaries
		if ( $kind eq "pby" ) {	$color = "#001010";           }; # bdy  national boundaries
		$name = "WDB $area $kind rank ${rank}";
		my $linetype='';
		$streets_type_id++;
		Geo::Gpsdrive::DBFuncs::db_exec("DELETE FROM `streets_type` WHERE streets_type_id = '$streets_type_id' ;");
		Geo::Gpsdrive::DBFuncs::db_exec
		    ("INSERT INTO `streets_type` ".
		     "        (streets_type_id, name,  color ,   color_bg,   width ,  width_bg ,  linetype ,  scale_min ,  scale_max )".
		     " VALUES ($i,            '$name','$color','$color_bg','$width','$width_bg','$linetype','$scale_min','$scale_max');");
		
		$i++;
	    }
	}
    }

    # ------------ Street Types
    # TODO: make english primary language
    my @streets_types = qw( xFFFFFF_x000000_w1_4_z00100000_unbekannt
			    x555555_x000000_w2_4_z00100000_Strassen.Allgemein
			    x557777_x000000_w2_4_z00100000_Strassen.primary
			    x557766_x000000_w2_4_z00100000_Strassen.mayor
			    x557766_x555555_w2_4_z00100000_Strassen.secondary
			    x558866_x000000_w2_4_z00100000_Strassen.residential
			    x555555_x000000_w1_2_z00100000_Strassen.minor

			    x00FFFF_xAA0000_w2_4_z10000000_Strassen.Highway
			    xFFFF00_xAA0000_w2_4_z10000000_Strassen.Autobahn
			    xFFFF00_x550000_w2_4_z00100000_Strassen.Bundesstrasse
			    xFFFF00_x550000_w2_4_z00100000_Strassen.Landstrasse
			    x00FFFF_x000000_w2_4_z00100000_Strassen.Innerorts
			    x00FFFF_x555555_w1_2_z00100000_Strassen.30_Zohne

			    x222222_x555555_w2_4_z00100000_Strassen.Wanderweg
			    x222222_x555555_w2_4_z00100000_Strassen.Fahrrad
			    x222222_x555555_w2_4_z00100000_Strassen.Fussweg
			    x222222_x557755_w2_4_z00100000_Strassen.Reitweg
			    x222222_x555555_w2_4_z00100000_Strassen.Trampelpfad

			    x444444_x000000_w2_4_z01000000_Schiene.ICE_Trasse
			    x444444_x000000_w2_4_z00100000_Schiene.Zug_Trasse
			    x002222_x000000_w2_4_z00100000_Schiene.S_Bahn_Trasse
			    x000022_x000000_w2_4_z00100000_Schiene.U_Bahn_Trasse
			    x000000_x000000_w2_4_z00100000_Schiene.Bus_Trasse

			    x444444_x000000_w2_4_z10000000_Grenzen.Landesgrenze
			    x444444_x000000_w2_4_z01000000_Grenzen.Bundesland_Grenze
			    x444444_x000000_w2_4_z01000000_Grenzen.Stadt_Grenze

			    x0000FF_x000000_w2_4_z01000000_Wasser.Fluss
			    x000FFF_x000000_w2_4_z01000000_Wasser.Kueste
			    );

    for my $entry  ( @streets_types ) {    
	warn "could not split $entry\n" 
	    
unless $entry =~ m/^x(......)_x(......)_w(\d+)_(\d+)_z(\d+)_(.*)$/;
	my $color     = "#$1";
	my $color_bg  = "#$2";
	my $width     = $3;
	my $width_bg  = $4;
	my $scale_max = $5;
	my $name      = $6;
	my $scale_min =  1;
	unless ( $color && $color_bg && $width && $width_bg && $scale_max && $name ) {
	    print "Error spliting $entry \n";
	}	    
	$name =~ s/_/ /g;
	my $linetype='';

	if( 0 && $debug ){	# For testing and displaying all streets in every scale
	    $scale_max = 1000000000000;
	    if ( $name =~ m/strasse/i ) {
		$color     = "#FFFF00";
		$color_bg  = "#AA0000";
		$color_bg  = "#000000";
		$width     = 1;
		$width_bg  = 1;
	    }
	}
		
	$streets_type_id++;
	Geo::Gpsdrive::DBFuncs::db_exec("DELETE FROM `streets_type` WHERE streets_type_id = '$streets_type_id';");
	Geo::Gpsdrive::DBFuncs::db_exec
	    ("INSERT INTO `streets_type` ".
	     "        (streets_type_id,  name,   color ,  color_bg  , width ,  width_bg  , linetype  , scale_min , scale_max )".
	     " VALUES ($i             ,'$name','$color','$color_bg','$width','$width_bg','$linetype','$scale_min','$scale_max');");
	
	$i++;
    }

    # Reserve Type 200 so users place there id's behind it
    $streets_type_id =500;
    Geo::Gpsdrive::DBFuncs::db_exec("DELETE FROM `streets_type` WHERE streets_type_id = '$streets_type_id';");
    Geo::Gpsdrive::DBFuncs::db_exec("INSERT INTO `streets_type` ".
				    "        (streets_type_id,  name     ,  color  ,  color_bg , linetype )".
				    " VALUES ($streets_type_id,'Reserved','#000000','#000000'  , '');");
    
} # of fill_default_street_types()

# -----------------------------------------------------------------------------

sub fill_defaults(){
    print "Create Defaults ...\n";
    fill_default_poi_types();
    fill_default_sources();
    fill_default_street_types();
    print "Create Defaults completed\n";
}

1;
