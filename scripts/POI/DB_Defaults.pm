# Database Defaults for poi/streets Table for poi.pl
#
# $Log$
# Revision 1.1  2005/05/09 19:35:12  tweety
# Split Default Values into seperate File
# Add new Icon
#

package POI::DB_Defaults;

use strict;
use warnings;

use POSIX qw(strftime);
use Time::Local;
use DBI;
use POI::Utils;
use Data::Dumper;
use IO::File;
use POI::DBFuncs;

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
# Fill poi_type database
sub defaults(){
{
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
	      accomodation.youth_hostel
	      administration
	      administration.car_registration
	      area.area
	      area.bridge
	      area.bridge.car
	      area.bridge.pedestrian
	      area.bridge.train
	      area.city
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
	      bank.money_exchange
	      camping.dumpstation
	      camping.water
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
	      fire_department
	      food.beergarden
	      food.cafe
	      food.delivery_service
	      food.drinking_water
	      food.ice_cream_parlor
	      food.restaurant
	      food.restaurant.bavarian
	      food.restaurant.chinese
	      food.restaurant.fastfood
	      food.restaurant.fastfood.burger_king
	      food.restaurant.fastfood.mc_donalds
	      food.restaurant.fastfood.pizza_hut
	      food.restaurant.greek
	      food.restaurant.indian
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
	      police
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
	      recreation.park
	      recreation.park.aquarium
	      recreation.park.zoo
	      recreation.park.amusement
	      recreation.model_aircrafts
	      recreation.music
	      recreation.music.concert_hall
	      recreation.music.dance_floor
	      recreation.music.disco
	      recreation.music.opera
	      recreation.music.theater
	      recreation.national_park
	      recreation.night_club
	      recreation.picnic_area
	      recreation.playground
	      recreation.pub
	      recreation.pub.dart
	      recreation.sport
	      recreation.sports.bicycling
	      recreation.sports.boat
	      recreation.sports.bob
	      recreation.sports.rollerscates
	      recreation.sports.rollerblades
	      recreation.sports.bowling
	      recreation.sports.fishing
	      recreation.sports.fittness
	      recreation.sports.golf_place
	      recreation.sports.horse.riding
	      recreation.sports.horse.racing
	      recreation.sports.water_ski
	      recreation.sports.hunt
	      recreation.sports.ice_scating
	      recreation.sports.minigolf_place
	      recreation.sports.motorbike
	      recreation.sports.parachute
	      recreation.sports.rock_climbing
	      recreation.sports.skiing
	      recreation.sports.snakeboard
	      recreation.sports.soccer_field
	      recreation.sports.soccer_stadion
	      recreation.sports.swimming
	      recreation.sports.swimming_area
	      recreation.sports.tennis_place
	      recreation.sports.climbing
	      recreation.sports.climbing.hall
	      recreation.sports.climbing.area
	      recreation.sports.skiing.
	      recreation.sports.skiing.cross-country
	      recreation.sports.skiing.down-hill
	      recreation.sports.climbing.rocks
	      recreation.sports.coast
	      recreation.zoo
	      recycling
	      recycling.glass
	      recycling.paper
	      recycling.recycling_centre
	      shopping
	      shopping.books
	      shopping.computer
	      shopping.computer.Software
	      shopping.computer.hardware
	      shopping.flea_market
	      shopping.food.bakery
	      shopping.food.butcher
	      shopping.food.fruit
	      shopping.food.vegetables
	      shopping.furnishing_house
	      shopping.furnishing_house
	      shopping.hardware_store
	      shopping.literature
	      shopping.shopping_center
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
	      transport.truck
	      transport.car.garage
	      transport.car.gas_station
	      transport.car.gas_station.shell
	      transport.car.gas_station.aral
	      transport.car.gas_station.omv
	      transport.car.gas_station.texaco
	      transport.car.gas_station.dea
	      transport.car.gas_station.jet
	      transport.car.gas_station.esso
	      transport.car.gas_station.agip
	      transport.car.parking
	      transport.car.parking.handicapped
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
	'Software'              => 'Software',
	'Sparda'                => 'Sparda',
	'WC'                    => 'WC',
	'accomodation'          => 'Uebernachtung',
	'administration'        => 'Verwaltung',
	'adult_education'       => 'VHS',
	'agency'                => "Filiale",
	'airport'               => 'Flugplatz',
	'aldi'                  => "Aldi",
	'ambulance'             => 'Ambulanz',
	'area'                  => "Gebiet",
	'area'                  => "Gebiet",
	'bank'                  => 'Bank',
	'beergarden'            => 'Biergarten',
	'bicycling'             => "Fahrrad",
	'boat'                  => "Boot",
	'books'                 => 'Buecher',
	'burger_king'           => 'Burger_King',
	'bus'                   => "Bus",
	'cafe'                  => 'cafe',
	'campground'            => 'Campingplatz',
	'car'                   => 'Auto',
	'car_loading_terminal'  => 'Auto_Verlade_Bahnhof',
	'car_registration'      => 'Zulassungsstelle',
	'catholic'              => 'Katholisch',
	'cemetary'              => 'Friedhof',
	'church'                => 'Kirche',
	'cinema'                => 'Kino',
	'city'                  => 'Stadt',
	'coach'                 => 'Kutsche',
	'coast'                 => 'rodeln',
	'computer'              => 'Computer',
	'construction'          => 'Baustelle',
	'container_loading_terminal' => 'Gueter_Verlade_Bahnhof',
	'custom'                => 'Benutzer_Eigene',
	'dart'                  => 'Dart',
	'delivery_service'      => 'Lieferservice',
	'developer'             => 'Entwickler',
	'disco'                 => 'disco',
	'doctor'                => 'Arzt',
	'drinking_water'        => "Trinkwasser",
	'driver-guide_service'  => 'Lotsendienst',
	'education'             => 'Ausbildung',
	'emergency'             => 'Notaufnahme',
	'emergency_call'        => 'Notruf_saeule',
	'entrance'              => 'Eingang',
	'event_location'        => 'Veranstaltungshalle',
	'exhibition'            => 'Messe',
	'exhibitions'           => 'Ausstellungen',
	'fairground'            => 'Rummelplatz',
	'fastfood'              => 'Schnellrestaurant',
	'ferry'                 => 'Faehre',
	'fire_department'       => 'Feuerwehr',
	'flea_market'           => 'Flohmarkt',
	'food'                  => 'Essen',
	'forest'                => "Wald",
	'friends'               => 'Freunde',
	'fruit'                 => 'Obst',
	'furnishing_house'      => 'Einrichtungs_Haus',
	'garage'                => 'Werkstatt',
	'gas_station'           => 'Tankstelle',
	'geocache'              => 'Geocache',
	'gift_shop'             => 'Geschenke_Laden',
	'glass'                 => 'Glas',
	'golf_place'            => 'Golfplatz',
	'groceries'             => 'Lebensmittel',
	'handicapped'           => "Behinderte",
	'hardware'              => 'Hardware',
	'hardware_store'        => 'Baumarkt',
	'health'                => 'Gesundheit',
	'highschool'            => 'Gymnasium',
	'home'                  => 'Daheim',
	'hospital'              => 'Krankenhaus',
	'hotel'                 => 'Hotel',
	'ice_cream_parlor'      => 'Eisdiele',
	'ice_scating'           => "Eislaufen",
	'import_way'            => 'import_way',
	'information'           => 'Information',
	'interurban_train_station' => 'S-Bahn-Haltestelle',
	'kindergarden'          => 'Kindergarten',
	'lake'                  => "See",
	'landmark'              => 'Sehenswuerdigkeit',
	'letter-box'            => 'Briefkasten',
	'lidl'                  => "Lidl",
	'literature'            => 'Literatur',
	'mc_donalds'            => 'MC_Donalds',
	'minigolf_place'        => 'Minigolfplatz',
	'model_aircrafts'       => 'Modellflugplatz',
	'money_exchange'        => 'Geldwechsel',
	'monument'              => "Denkmal",
	'motel'                 => 'Motel',
	'motorbike'             => "Motorrad",
	'mountain'              => "Berg",
	'museum'                => 'Museum',
	'national_park'         => 'Nationalpark',
	'night_club'            => 'Nachtclub',
	'nursery'               => 'Kinderkrippe',
	'office'                => 'Amt',
	'open'                  => 'Offen',
	'opera'                 => 'Oper',
	'paper'                 => 'Papier',
	'park'                  => "Park",
	'park_and_ride'         => 'Park_and_Ride',
	'parking'               => "Parken",
	'parkinglot'            => 'Parkplatz',
	'pedestrian_zone'       => 'Fussgaenger_Zone',
	'penny'                 => "Penny",
	'pharmacy'              => 'Apotheke',
	'phototrap_traffic_light' => 'Blitzampel',
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
	'railroad'              => "Eisenbahn",
	'recreation'            => 'Freizeit',
	'recycling'             => 'Recycling',
	'recycling_centre'      => 'Wertstoffhof',
	'rest_area'             => 'Raststaette',
	'restaurant'            => 'Gaststaette',
	'restaurant'            => 'Restaurant',
	'riding'                => 'Reiten',
	'rock_climbing'         => "Fels klettern",
	'secondary_school'      => 'HauptSchule',
	'shopping'              => 'Einkaufen',
	'shopping_center'       => 'Einkaufszentrum',
	'skiing'                => 'Skifahren',
	'soccer_field'          => 'Fussballplatz',
	'soccer_stadion'        => 'Fussball_Stadion',
	'speedtrap'             => 'Radarfalle',
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
	'tennis_place'          => 'Tennisplatz',
	'theatre'               => 'Theater',
	'toll_station'          => 'Mautstation',
	'town_hall'             => 'Stadthalle',
	'toys'                  => 'Spielwaren',
	'traffic'               => 'Verkehr',
	'traffic_jam'           => 'Stau',
	'train'                 => 'Zug',
	'tram_station'          => 'Strassen-Bahn_Haltestelle',
	'transport'             => 'Transport',
	'txt'                   => 'txt',
	'underground'           => "U-Bahn",
	'underground_station'   => 'U-Bahn_Haltestelle',
	'university'            => 'UNI',
	'unknown'               => 'unknown',
	'vegetables'            => 'Gemuese',
	'w-lan'                 => 'W-LAN',
	'wep'                   => 'WEP',
	'work'                  => 'Arbeit',
	'youth_hostel'          => 'Jugend_Herberge',
	'zoo'                   => 'Zoo',
        'aeroplane'             => "Motor Flugzeug",
        'agip'                  => "Agip",
        'amusement'             => "Freizeit",
        'anker'                 => "Anker",
        'aquarium'              => "Aquarium",
        'aral'                  => "Aral",
        'avis'                  => "Avis",
        'bakery'                => "Bakery",
        'bavarian'              => "Bayrisch",
        'bob'                   => "Bob",
        'bowling'               => "Kegeln",
        'bridge'                => "Bruecke",
        'butcher'               => "Metzger",
        'camping'               => "Camping",
        'cemetery'              => "Friedhof",
        'chinese'               => "Chinesisch",
        'climbing'              => "Klettern",
        'concert_hall'          => "Konzert_Halle",
        'cross-country'         => "langlauf",
        'dam'                   => "Wehr",
        'dance_floor'           => "Tanzen",
        'dea'                   => "Dea",
        'down-hill'             => "Abfahrt",
        'dumpstation'           => "Dump Platz",
        'esso'                  => "Esso",
        'exibition'             => "Ausstellung",
        'fishing'               => "angeln",
        'fittness'              => "Fittness",
        'glider'                => "Selgelflugzeug",
        'greek'                 => "Griechisch",
        'hall'                  => "Halle",
        'harbor'                => "Hafen",
        'helicopter'            => "Hubschrauber",
        'herz'                  => "Herz",
        'horse'                 => "Pferde",
        'hunt'                  => "Jagd",
        'indian'                => "Indisch",
        'japanese'              => "japanisch",
        'jet'                   => "Jet",
        'mine'                  => "Miene",
        'music'                 => "Music",
        'my'                    => "Meine",
        'omv'                   => "Omv",
        'parachute'             => "Fallschirmspringen",
        'pedestrian'            => "Fussgaenger",
        'picnic_area'           => "Rastplatz",
        'pizza'                 => "Pizza",
        'pizza_hut'             => "Pizza_Hut",
        'play_street'           => "Spiel Strasse",
        'racing'                => "Rennen",
        'railway'               => "Eisenbahn",
        'rental'                => "Vermietung",
        'rocks'                 => "Fels",
        'rollerblades'          => "Rollerblades",
        'rollerscates'          => "Rollschuhe",
        'scale'                 => "Waage",
        'shell'                 => "Shell",
        'ship'                  => "Boot",
        'shower'                => "Dusche",
        'sixt'                  => "Sixt",
        'snakeboard'            => "Snakeboard",
        'sport'                 => "Sport",
        'tent'                  => "Zelt",
        'texaco'                => "Texaco",
        'theater'               => "Theater",
        'trade_show'            => "Handels_Messe",
        'trailer'               => "Wohnwagen",
        'tram'                  => "Strassenbahn",
        'truck'                 => "Lastwagen",
        'tunnel'                => "Tunnel",
        'water'                 => "Wasser",
        'water_ski'             => "Wasser Ski",
        'wrecking_service'      => "Abschleppdienst",
    };

    my $i=1;
    my $unknown_translations;
    
    # for debug purpose
    POI::DBFuncs::db_exec("TRUNCATE TABLE `poi_type`;");


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
	'camping.water' => 'tap',
	'church'                          => "church",
	'church.cemetary'                 => 'cemetary',
	'circle' 			  => 'ppl',
	'custom.friends.home' 	          => "home",
	'custom.friends.work' 	          => "work",
	'custom.my.home' 	          => "home",
	'custom.my.work' 	          => "work",
	'education.primary school'        => 'ABC',
	'education.university'            => 'emc2',
	'food' 		                  => "food",
	'food.beergarden'                 => 'beergarden',
	'food.cafe'            => "cafe",
	'food.drinking water'             => 'tap_drinking',
	'food.ice cream parlor' => "ice_cream",
	'food.restaurant.bavarian'        => 'beergarden',
	'food.restaurant.fastfood.burger king'       => "burgerking",
	'food.restaurant.fastfood.mc donalds' 	  => "mcdonalds",
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
	'recreation.sports.golf place'     => "golf",
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
	'transport.car.parking.handicapped'=> "handicapped",
	'transport.car.parkinglot'        => "parking",
	'transport.car.parkinglot'        => "parking",
	'transport.car.phototrap traffic light' => "speed_trap",
	'transport.car.rental'            => 'car_rental',
	'transport.car.rest area' 	  => 'rest_area',
	'transport.car.wrecking service'  => 'towing',
	'transport.park_and_ride'         => 'P+R',
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
    # education.nursery ==> Schnuller
    # Entrance          ==> Door
    # police ==> Sherrif Stern
    # park_and_ride ==> blaues P + Auto + xxx
    # recreation.fairground    ==> Riessenrad
    # gift_shop ==> Geschenk Paeckchen
    # model.airplanes ==> Airplane + Fernsteuerung
    # recreation.playground   => Schaukel
    # Bei Sportarten evtl. die Olympischen Symbole verwenden
    # Zoo => Giraffe+ V�gel
    # WC ==> WC oder 00
    # Recycling ==> R oder Gruene Punkt

    my $generate_poi_type_html_page=1;
    my $poi_type_html_page;
    if ($generate_poi_type_html_page) {
	$poi_type_html_page = IO::File->new(">../data/icons/index.html")
	    or warn "Cannot open html Page for writing: $!";;
	print $poi_type_html_page "<html>\n";
	print $poi_type_html_page "<body bgcolor=\"#AAAAFF\">\n";
	print $poi_type_html_page "<table border>\n";
	print $poi_type_html_page "<th>icon</th>	<th>filename</th>\n";
	print $poi_type_html_page "<th>name</th>	<th>description</th>\n";
#	print $poi_type_html_page "<th>name_de</th>	";
	print $poi_type_html_page "<th>description_de</th>\n";
    }

    my $used_icons ={};
    for my $icon ( glob("../data/icons/*.{png,gif}" ) ) {
	next unless -f $icon;
	$used_icons->{$icon}=0;
    }

    my $poi_type_id=0;
    for my $name  ( @poi_type_names ) {
	$poi_type_id++;
	# Translate the entries
	my $name_de ='';
	for my $part ( split(/\./,$name)){
	    if ( ! defined( $translate_de->{$part}) ) {
		$unknown_translations .= 
		    sprintf("	%-23s => \"".lc($part)."\",\n","'".$part."'");
	    }
	    $name_de .= ".$translate_de->{$part}";
	}
	$name_de    =~ s/^\.//;
	$name_de    =~ s/_/ /g;
	$name =~ s/_/ /g;
	
	
	# Find Icon
	my $icon;
	my $icon_name = "$name.png";
	while ( $icon_name =~ s/\.[^\.]+$// ){
	    if (  defined ( $icons->{$icon_name} )) {
		$icon = $icons->{$icon_name};
		last;
	    }
	}
	$icon ||=  "unknown";
	$icon =~ s/ /_/g;
	if ( -s "../data/icons/$icon.png" ) { $icon .= ".png" };
	if ( -s "../data/icons/$icon.gif" ) { $icon .= ".gif" };

	$used_icons->{"../data/icons/$icon"}++;

	# create description
	my $description     = $name;    $description    =~ s/\./ /g;
	my $description_de  = $name_de; $description_de =~ s/\./ /g;

	# create html page for icons
	if ( $poi_type_html_page ) {
	    print $poi_type_html_page "<tr> ";
	    if ( $icon =~ m/unknown/ ) {
		print $poi_type_html_page "<td>&nbsp;</td>";
		print $poi_type_html_page "<td>&nbsp;</td>";
	    } else {
		print $poi_type_html_page "<td valign=TOP ><img src=\"$icon\"></td>";
		print $poi_type_html_page "<td valign=TOP >$icon</td>";
	    }
	    print $poi_type_html_page "<td valign=TOP >$name</td>";
	    print $poi_type_html_page "<td valign=TOP >$description</td>";
#	    print $poi_type_html_page "<td>$name_de</td>";
	    print $poi_type_html_page "<td valign=TOP >$description_de</td>";
	    print $poi_type_html_page "</tr> \n";
	}
	
	# Insert to Database
	POI::DBFuncs::db_exec("DELETE FROM `poi_type` WHERE poi_type_id = $poi_type_id ;");
	POI::DBFuncs::db_exec("INSERT INTO `poi_type` ".
		"       (poi_type_id, name,name_de, symbol, description,description_de ) ".
		"VALUES ($poi_type_id,'$name','$name_de','$icon','$description','$description_de');") 
	    or die;
	$i++;
    } # of for @poi_type_names
    
    print $poi_type_html_page "</table>\n";
    if ( $poi_type_html_page ) { # create html page for icons
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
    }

    if ( $unknown_translations ) {
	print "# Missing poi_type Translations:\n";
	print "$unknown_translations\n";
	print "\n";
    }


}

# -----------------------------------------------------------------------------
{   # Just some Default Sources
    my $coutry2name;
    for my $k ( keys %{$POI::NGA::name2country} ) {
	$coutry2name->{$POI::NGA::name2country->{$k}} =$k;
    }

    for my $country  ( @POI::NGA::countries ) {    
	my $name ="earth-info.nga.mil $country";

	POI::DBFuncs::db_exec("DELETE FROM `source` WHERE source.name = '$name';");
	my $source_hash = {
	    'source.url'     => "http://www.evl.uic.edu/pape/data/WDB/WDB-text.tar.gz",
	    'source.name'    => $name,
	    'source.comment' => "GeoData for $coutry2name->{$country}($country)",
	    'source.licence' => ""
	    };
	POI::DBFuncs::insert_hash("source", $source_hash);
    }

    for my $source  ( qw( import_way.txt
			  defaults
			  ) ) {    
	my $name ="$source";
	$name =~ s/_/ /g;

	POI::DBFuncs::db_exec("DELETE FROM `source` WHERE source.name = '$name';");
	my $source_hash = {
	    'source.url'     => "",
	    'source.name'    => $name,
	    'source.comment' => "$name",
	    'source.licence' => ""
	    };
	POI::DBFuncs::insert_hash("source", $source_hash);
    }

}

{
    # Insert Example Waypoints
    my $source_name = "Defaults";
    my $source_id = source_name2id($source_name);
    die "Unknown Source ID: $source_name\n" unless $source_id;

    my $type_name = "Developer";
    my $type_id = poi_type_name2id($type_name);
    die "Type for '$type_name' not found \n" unless $type_id;


    for  my $t ( qw(waypoints poi)) {
	my $wp_defaults = { "$t.wep"         => 0 ,
			    "$t.nettype"     => '',
			    "$t.scale_min"     => 1,
			    "$t.scale_max"     => 1000000000,
			    "$t.last_modified" => time(),
			    "$t.type"        => 'Developer',
			    "$t.poi_type_id" => $type_id,
			    "$t.source_id" => $source_id,
			};
	my $type_query = $t  eq "poi" 
	    ? "poi_type_id = '$type_id'" 
	    : "type = '". $wp_defaults->{"$t.type"} ."'";
	
	for my $loc (
		 { "$t.name" => "Joerg", "$t.lat" => '48.17', "$t.lon" => '11.75'} 
		     ) {
	    print "Sample WP:$t\n";
	    my $delete_query=sprintf("DELETE FROM $t ".
				     "WHERE name = '%s' AND $type_query",
				     $loc->{"$t.name"});
	    POI::DBFuncs::db_exec( $delete_query);
	    POI::DBFuncs::insert_hash($t, $wp_defaults, $loc );
	}
    }
}

{
    # Insert Example Cities
    my $type_name = "area.city";
    my $type_id = poi_type_name2id($type_name);
    die "Type for '$type_name' not found \n" unless $type_id;

    my $source_name = "Defaults";
    my $source_id = source_name2id($source_name);
    die "Unknown Source ID: $source_name\n" unless $source_id;

    for  my $t ( qw(waypoints poi)) {
	my $wp_defaults = { "$t.wep"         => 0 ,
			    "$t.nettype"     => '',
			    "$t.scale_min"   => 1,
			    "$t.scale_max"   => 1000000000,
			    "$t.type"        => 'City',
			    "$t.source_id"   => $source_id,
			    "$t.poi_type_id" => $type_id,
			    "$t.last_modified" => time(),
			};
	my $type_query = $t  eq "poi" 
	    ? " poi_type_id = '$type_id'" 
	    : " type = '". $wp_defaults->{"$t.type"} ."'";
	
	for my $loc (
		 { name => 'Mechtat A������ne el Msad',lat => 36.5033333,lon => 6.0758333}, 
		 { name => 'Halte A������n Lehma',lat => 36.1166667,lon => 6.4833333}, 
		 { name => 'Halte d A������n-Lehms',lat => 36.1166667,lon => 6.4833333}, 
		 { name => 'Berteau A������n Lehma',lat => 36.1166667,lon => 6.4833333}, 
		 { name => 'Berteaux-A������n Lehma',lat => 36.1166667,lon => 6.4833333}, 
		 { name => 'Bordjel El Hassane',lat => 36.6027778,lon => 7.6916667}, 
		 { name => 'Borj Scander',lat => 36.8347222,lon => 7.7013889}, 
		 { name => 'Bouchegout',lat => 36.4722222,lon => 7.7333333}, 
		 { name => 'Dar Lahleme',lat => 36.8347222,lon => 7.575}, 
		 { name => 'Djenane El Bey',lat => 36.8305556,lon => 7.1180556}, 
		 { name => 'Henchir Mokhtar Abdullah el Medjid',lat => 36.6388889,lon => 7.7083333}, 
		 { name => 'Mechetet Bellchari',lat => 36.6083333,lon => 7.5805556}, 
		 { name => 'Mechta A������n el Msad',lat => 36.5033333,lon => 6.0758333}, 
		 { name => 'Mechta Beni Hameza',lat => 36.6694444,lon => 7.4347222}, 
		 { name => 'Mechta Fedj el Ouathia',lat => 36.2583333,lon => 7.9861111}, 
		 { name => 'Mechta Fetmiat',lat => 36.7680556,lon => 7.3166667}, 
		 { name => 'Mechta Guebel A������n',lat => 36.0013889,lon => 7.7819444}, 
		 { name => 'Mechta GuebelA������n',lat => 36.0027778,lon => 7.7805556}, 
		 { name => 'Mechta Jorf El Ahmar',lat => 36.4513889,lon => 7.5805556}, 
		 { name => 'Mechta Messous',lat => 36.4847222,lon => 7.2472222}, 
		 { name => 'Mechta Ouled Bou Dene',lat => 36.0294444,lon => 6.5097222}, 
		 { name => 'Mechta Ouled Bou Denn',lat => 36.0294444,lon => 6.5097222}, 
		 { name => 'Mechtat Ahmed Bel Ajmi',lat => 36.3347222,lon => 7.5083333}, 
		 { name => 'Mechtat Bou Rougaa',lat => 36.6194444,lon => 7.875}, 
		 { name => 'Mechtat es Stah',lat => 36.0916667,lon => 7.7847222}, 
		 { name => 'Mechtat Ouled Bou Denn',lat => 36.0294444,lon => 6.5097222}, 
		 { name => 'Mechtet Ain Maflouf',lat => 36.525,lon => 7.5347222}, 
		 { name => 'Mzara Sidi Goulea',lat => 36.6638889,lon => 7.5583333}, 
		 { name => 'Elbasan',lat => 41.1125,lon => 20.0822222}, 
		 { name => 'Elbasani',lat => 41.1125,lon => 20.0822222}, 
		 { name => 'Elbassan',lat => 41.1125,lon => 20.0822222}, 
		 { name => 'Bosna-Sarai',lat => 43.85,lon => 18.3833333}, 
		 { name => 'Sarajevo',lat => 43.85,lon => 18.3833333}, 
		 { name => 'Vrh Bosna',lat => 43.85,lon => 18.3833333}, 
		 { name => 'Chuquisaca',lat =>  -19.0430556,lon =>  -65.2591667}, 
		 { name => 'Ciudad Potosi',lat =>  -19.5836111,lon =>  -65.7530556}, 
		 { name => 'Ciudad Sucre',lat =>  -19.0430556,lon =>  -65.2591667}, 
		 { name => 'Potos������',lat =>  -19.5836111,lon =>  -65.7530556}, 
		 { name => 'Sucre',lat =>  -19.0430556,lon =>  -65.2591667}, 
		 { name => 'Bazargic',lat => 43.5666667,lon => 27.8333333}, 
		 { name => 'Bourgas',lat => 42.5,lon => 27.4666667}, 
		 { name => 'Burgas',lat => 42.5,lon => 27.4666667}, 
		 { name => 'Burgaz',lat => 42.5,lon => 27.4666667}, 
		 { name => 'Burghaz',lat => 42.5,lon => 27.4666667}, 
		 { name => 'Choum������n',lat => 43.2766667,lon => 26.9291667}, 
		 { name => 'Dobri������',lat => 43.5666667,lon => 27.8333333}, 
		 { name => 'Dobrich',lat => 43.5666667,lon => 27.8333333}, 
		 { name => 'Dobritch',lat => 43.5666667,lon => 27.8333333}, 
		 { name => 'Dobritsch',lat => 43.5666667,lon => 27.8333333}, 
		 { name => 'Eski Zagra',lat => 42.4327778,lon => 25.6419444}, 
		 { name => 'Eumolpias',lat => 42.15,lon => 24.75}, 
		 { name => 'Filiba',lat => 42.15,lon => 24.75}, 
		 { name => 'Filipopol',lat => 42.15,lon => 24.75}, 
		 { name => 'Filippopol',lat => 42.15,lon => 24.75}, 
		 { name => 'Flavia',lat => 42.15,lon => 24.75}, 
		 { name => 'Had������i-Oghlu-Pazard������ik',lat => 43.5666667,lon => 27.8333333}, 
		 { name => 'Islimje',lat => 42.6858333,lon => 26.3291667}, 
		 { name => 'Julia',lat => 42.15,lon => 24.75}, 
		 { name => 'Khadzhioglu Bazardzhik',lat => 43.5666667,lon => 27.8333333}, 
		 { name => 'Khadzhioglu Pazardzhik',lat => 43.5666667,lon => 27.8333333}, 
		 { name => 'Kolarovgrad',lat => 43.2766667,lon => 26.9291667}, 
		 { name => 'Odessos',lat => 43.2166667,lon => 27.9166667}, 
		 { name => 'Odessus',lat => 43.2166667,lon => 27.9166667}, 
		 { name => 'Philippopel',lat => 42.15,lon => 24.75}, 
		 { name => 'Philippopoli',lat => 42.15,lon => 24.75}, 
		 { name => 'Philippopolis',lat => 42.15,lon => 24.75}, 
		 { name => 'Pinople',lat => 42.15,lon => 24.75}, 
		 { name => 'Pleven',lat => 43.4166667,lon => 24.6166667}, 
		 { name => 'Plevna',lat => 43.4166667,lon => 24.6166667}, 
		 { name => 'Plewen',lat => 43.4166667,lon => 24.6166667}, 
		 { name => 'Plodin',lat => 42.15,lon => 24.75}, 
		 { name => 'Ploudin',lat => 42.15,lon => 24.75}, 
		 { name => 'Plovdin',lat => 42.15,lon => 24.75}, 
		 { name => 'Plovdiv',lat => 42.15,lon => 24.75}, 
		 { name => 'Plyeven',lat => 43.4166667,lon => 24.6166667}, 
		 { name => 'Poulpoudeva',lat => 42.15,lon => 24.75}, 
		 { name => 'Rouss������',lat => 43.8563889,lon => 25.9708333}, 
		 { name => 'Roustchouk',lat => 43.8563889,lon => 25.9708333}, 
		 { name => 'Ruschuk',lat => 43.8563889,lon => 25.9708333}, 
		 { name => 'Ruschuq',lat => 43.8563889,lon => 25.9708333}, 
		 { name => 'Rusciuk',lat => 43.8563889,lon => 25.9708333}, 
		 { name => 'Rusclink',lat => 43.8563889,lon => 25.9708333}, 
		 { name => 'Ru������������uk',lat => 43.8563889,lon => 25.9708333}, 
		 { name => 'Ruse',lat => 43.8563889,lon => 25.9708333}, 
		 { name => 'Rushchuk',lat => 43.8563889,lon => 25.9708333}, 
		 { name => 'Rushtuk',lat => 43.8563889,lon => 25.9708333}, 
		 { name => 'Russe',lat => 43.8563889,lon => 25.9708333}, 
		 { name => 'Rustschuk',lat => 43.8563889,lon => 25.9708333}, 
		 { name => 'Saray-Makhle',lat => 43.8563889,lon => 25.9708333}, 
		 { name => 'Schumen',lat => 43.2766667,lon => 26.9291667}, 
		 { name => 'Serdica',lat => 42.6833333,lon => 23.3166667}, 
		 { name => 'Shumen',lat => 43.2766667,lon => 26.9291667}, 
		 { name => 'Shumla',lat => 43.2766667,lon => 26.9291667}, 
		 { name => 'Sinople',lat => 42.15,lon => 24.75}, 
		 { name => 'Sliven',lat => 42.6858333,lon => 26.3291667}, 
		 { name => 'Slivno',lat => 42.6858333,lon => 26.3291667}, 
		 { name => 'Sliwen',lat => 42.6858333,lon => 26.3291667}, 
		 { name => 'Sofia',lat => 42.6833333,lon => 23.3166667}, 
		 { name => 'Sofija',lat => 42.6833333,lon => 23.3166667}, 
		 { name => 'Sofiya',lat => 42.6833333,lon => 23.3166667}, 
		 { name => 'Sredets',lat => 42.6833333,lon => 23.3166667}, 
		 { name => 'Stalin',lat => 43.2166667,lon => 27.9166667}, 
		 { name => 'Stara Sagora',lat => 42.4327778,lon => 25.6419444}, 
		 { name => 'Stara Zagora',lat => 42.4327778,lon => 25.6419444}, 
		 { name => '������umen',lat => 43.2766667,lon => 26.9291667}, 
		 { name => '������umla',lat => 43.2766667,lon => 26.9291667}, 
		 { name => 'Tolbuhin',lat => 43.5666667,lon => 27.8333333}, 
		 { name => 'Tolbukhin',lat => 43.5666667,lon => 27.8333333}, 
		 { name => 'Trimontium',lat => 42.15,lon => 24.75}, 
		 { name => 'Ulpia',lat => 42.15,lon => 24.75}, 
		 { name => 'Ulpia-Serdica',lat => 42.6833333,lon => 23.3166667}, 
		 { name => 'Varna',lat => 43.2166667,lon => 27.9166667}, 
		 { name => 'Vinipoppolis',lat => 42.15,lon => 24.75}, 
		 { name => 'Warna',lat => 43.2166667,lon => 27.9166667}, 
		 { name => 'Bubanza',lat => -3.0833333,lon => 29.3944444}, 
		 { name => 'Buheranyi',lat => -3.095,lon => 30.3072222}, 
		 { name => 'Bujumbura',lat => -3.3761111,lon => 29.36}, 
		 { name => 'Bururi',lat => -3.9511111,lon => 29.6166667}, 
		 { name => 'Cankuzo',lat => -3.2194444,lon => 30.5527778}, 
		 { name => 'Cibitoke',lat => -2.8861111,lon => 29.1236111}, 
		 { name => 'Ciyubake',lat => -3.4277778,lon => 29.9677778}, 
		 { name => 'Dutwaro',lat => -3.3205556,lon => 29.8611111}, 
		 { name => 'Gacaca',lat => -3.2869444,lon => 29.9322222}, 
		 { name => 'Gasyangiri',lat => -3.3458333,lon => 30.42}, 
		 { name => 'Gitega',lat => -3.425,lon => 29.9333333}, 
		 { name => 'Karuzi',lat => -3.1013889,lon => 30.1647222}, 
		 { name => 'Kayanza',lat => -2.9222222,lon => 29.6222222}, 
		 { name => 'Kibenga',lat => -3.7991667,lon => 29.3858333}, 
		 { name => 'Kirambi',lat => -3.4352778,lon => 30.1155556}, 
		 { name => 'Kirundo',lat => -2.5847222,lon => 30.0972222}, 
		 { name => 'Kiryama',lat => -2.6588889,lon => 30.0502778}, 
		 { name => 'Kitega',lat => -3.425,lon => 29.9333333}, 
		 { name => 'Makamba',lat => -4.1347222,lon => 29.805}, 
		 { name => 'Muramvya',lat => -3.2680556,lon => 29.6166667}, 
		 { name => 'Mururinzi',lat => -3.1361111,lon => 30.4508333}, 
		 { name => 'Muyinga',lat => -2.8494444,lon => 30.3361111}, 
		 { name => 'Ngozi',lat => -2.9083333,lon => 29.8277778}, 
		 { name => 'Nyaguhaga',lat => -3.5266667,lon => 30.1477778}, 
		 { name => 'Nyamuhanga',lat => -3.8283333,lon => 29.5947222}, 
		 { name => 'Rusunwe',lat => -3.0511111,lon => 29.9180556}, 
		 { name => 'Rutana',lat => -3.9191667,lon => 29.9936111}, 
		 { name => 'Ruvumu',lat => -3.4144444,lon => 30.1094444}, 
		 { name => 'Ruyigi',lat => -3.4763889,lon => 30.2486111}, 
		 { name => 'Rwimbogo',lat => -3.2808333,lon => 30.4641667}, 
		 { name => 'Beijing',lat => 39.9288889,lon =>  116.3883333}, 
		 { name => 'Beijing Shi',lat => 39.9288889,lon =>  116.3883333}, 
		 { name => 'Kombo Lot������',lat => 3.4830556,lon => 9.7280556}, 
		 { name => 'Bangui',lat => 4.3666667,lon => 18.5833333}, 
		 { name => 'Aalborg',lat => 57.05,lon => 9.9333333}, 
		 { name => 'Aarhus',lat => 56.15,lon => 10.2166667}, 
		 { name => '������lborg',lat => 57.05,lon => 9.9333333}, 
		 { name => '������rhus',lat => 56.15,lon => 10.2166667}, 
		 { name => 'Copenhagen',lat => 55.6666667,lon => 12.5833333}, 
		 { name => 'Kjobenhavn',lat => 55.6666667,lon => 12.5833333}, 
		 { name => 'K������benhavn',lat => 55.6666667,lon => 12.5833333}, 
		 { name => 'Odense',lat => 55.4,lon => 10.3833333}, 
		 { name => 'Qaryat `Al������ Bin Ab������ ������������lib',lat => 26.7005556,lon => 31.4236111}, 
		 { name => 'Baile ������tha Cliath',lat => 53.3330556,lon => -6.2488889}, 
		 { name => 'Corcaigh',lat => 51.8986111,lon => -8.4958333}, 
		 { name => 'Cork',lat => 51.8986111,lon => -8.4958333}, 
		 { name => 'Dublin',lat => 53.3330556,lon => -6.2488889}, 
		 { name => 'D������n Laoghaire',lat => 53.2925,lon => -6.1286111}, 
		 { name => 'Dunleary',lat => 53.2925,lon => -6.1286111}, 
		 { name => 'Kingstown',lat => 53.2925,lon => -6.1286111}, 
		 { name => 'Derpt',lat => 58.3661111,lon => 26.7361111}, 
		 { name => 'Dorpat',lat => 58.3661111,lon => 26.7361111}, 
		 { name => 'Kallinn',lat => 59.4338889,lon => 24.7280556}, 
		 { name => 'Kolyvan',lat => 59.4338889,lon => 24.7280556}, 
		 { name => 'R������������veli',lat => 59.4338889,lon => 24.7280556}, 
		 { name => 'Reval',lat => 59.4338889,lon => 24.7280556}, 
		 { name => 'Revel',lat => 59.4338889,lon => 24.7280556}, 
		 { name => 'Talinas',lat => 59.4338889,lon => 24.7280556}, 
		 { name => 'Tallin',lat => 59.4338889,lon => 24.7280556}, 
		 { name => 'Tallina',lat => 59.4338889,lon => 24.7280556}, 
		 { name => 'Tallinn',lat => 59.4338889,lon => 24.7280556}, 
		 { name => 'Tallinna',lat => 59.4338889,lon => 24.7280556}, 
		 { name => 'Tartto',lat => 58.3661111,lon => 26.7361111}, 
		 { name => 'Tartu',lat => 58.3661111,lon => 26.7361111}, 
		 { name => 'T������rbata',lat => 58.3661111,lon => 26.7361111}, 
		 { name => 'Yurev',lat => 58.3661111,lon => 26.7361111}, 
		 { name => 'Yuryev',lat => 58.3661111,lon => 26.7361111}, 
		 { name => 'Pilsen',lat => 49.75,lon => 13.3666667}, 
		 { name => 'Plze������',lat => 49.75,lon => 13.3666667}, 
		 { name => '������bo',lat => 60.45,lon => 22.2833333}, 
		 { name => 'Esbo',lat => 60.2166667,lon => 24.6666667}, 
		 { name => 'Espoo',lat => 60.2166667,lon => 24.6666667}, 
		 { name => 'Helsingfors',lat => 60.1755556,lon => 24.9341667}, 
		 { name => 'Helsinki',lat => 60.1755556,lon => 24.9341667}, 
		 { name => 'Khelsinki',lat => 60.1755556,lon => 24.9341667}, 
		 { name => 'Oulu',lat => 65.0166667,lon => 25.4666667}, 
		 { name => 'Tammerfors',lat => 61.5,lon => 23.75}, 
		 { name => 'Tampere',lat => 61.5,lon => 23.75}, 
		 { name => 'Turku',lat => 60.45,lon => 22.2833333}, 
		 { name => 'Ule������borg',lat => 65.0166667,lon => 25.4666667}, 
		 { name => 'Uleoborg',lat => 65.0166667,lon => 25.4666667}, 
		 { name => 'Vanda',lat => 60.3,lon => 24.85}, 
		 { name => 'Vantaa',lat => 60.3,lon => 24.85}, 
		 { name => 'Yelsinki',lat => 60.1755556,lon => 24.9341667}, 
		 { name => 'Paris',lat => 48.8666667,lon => 2.3333333}, 
		 { name => 'Godthaab',lat => 64.1833333,lon =>  -51.75}, 
		 { name => 'Godth������b',lat => 64.1833333,lon =>  -51.75}, 
		 { name => 'Nuk',lat => 64.1833333,lon =>  -51.75}, 
		 { name => 'Nuuk',lat => 64.1833333,lon =>  -51.75}, 
		 { name => 'Augusta Ubiorum',lat => 50.9333333,lon => 6.95}, 
		 { name => 'Berlin',lat => 52.5166667,lon => 13.4}, 
		 { name => 'Bremen',lat => 53.0833333,lon => 8.8}, 
		 { name => 'C������ln',lat => 50.9333333,lon => 6.95}, 
		 { name => 'Cologne',lat => 50.9333333,lon => 6.95}, 
		 { name => 'Colonia Agrippina',lat => 50.9333333,lon => 6.95}, 
		 { name => 'Colonia Agrippinensis',lat => 50.9333333,lon => 6.95}, 
		 { name => 'Dortmund',lat => 51.5166667,lon => 7.45}, 
		 { name => 'Duisburg',lat => 51.4333333,lon => 6.75}, 
		 { name => 'Duisburg and Hamborn',lat => 51.4333333,lon => 6.75}, 
		 { name => 'Duisburg-Hamborn',lat => 51.4333333,lon => 6.75}, 
		 { name => 'D������sseldorf',lat => 51.2166667,lon => 6.7666667}, 
		 { name => 'Essen',lat => 51.45,lon => 7.0166667}, 
		 { name => 'Frankford-on-Main',lat => 50.1166667,lon => 8.6833333}, 
		 { name => 'Frankfort',lat => 50.1166667,lon => 8.6833333}, 
		 { name => 'Frankfort on the Main',lat => 50.1166667,lon => 8.6833333}, 
		 { name => 'Frankfurt',lat => 50.1166667,lon => 8.6833333}, 
		 { name => 'Frankfurt',lat => 50.1166667,lon => 8.6833333}, 
		 { name => 'Hamburg',lat => 53.55,lon => 10.0}, 
		 { name => 'Hannover',lat => 52.3666667,lon => 9.7166667}, 
		 { name => 'Hanover',lat => 52.3666667,lon => 9.7166667}, 
		 { name => 'Koeln',lat => 50.9333333,lon => 6.95}, 
		 { name => 'K������ln',lat => 50.9333333,lon => 6.95}, 
		 { name => 'Monaco',lat => 48.15,lon => 11.5833333}, 
		 { name => 'Muenchen',lat => 48.15,lon => 11.5833333}, 
		 { name => 'M������nchen',lat => 48.15,lon => 11.5833333}, 
		 { name => 'Munich',lat => 48.15,lon => 11.5833333}, 
		 { name => 'Stuttgart',lat => 48.7666667,lon => 9.1833333}, 
		 { name => 'L������risa',lat => 39.6372222,lon => 22.4202778}, 
		 { name => 'L������rissa',lat => 39.6372222,lon => 22.4202778}, 
		 { name => 'Le Pir������e',lat => 37.9613889,lon => 23.6388889}, 
		 { name => 'P������tra',lat => 38.2444444,lon => 21.7344444}, 
		 { name => 'Patrae',lat => 38.2444444,lon => 21.7344444}, 
		 { name => 'P������trai',lat => 38.2444444,lon => 21.7344444}, 
		 { name => 'Patras',lat => 38.2444444,lon => 21.7344444}, 
		 { name => 'Patrasse',lat => 38.2444444,lon => 21.7344444}, 
		 { name => 'Peiraeus',lat => 37.9613889,lon => 23.6388889}, 
		 { name => 'Peirai������',lat => 37.9613889,lon => 23.6388889}, 
		 { name => 'Peirai������fs',lat => 37.9613889,lon => 23.6388889}, 
		 { name => 'Peiraieus',lat => 37.9613889,lon => 23.6388889}, 
		 { name => 'Peiraievs',lat => 37.9613889,lon => 23.6388889}, 
		 { name => 'Piraeus',lat => 37.9613889,lon => 23.6388889}, 
		 { name => 'Pirai������vs',lat => 37.9613889,lon => 23.6388889}, 
		 { name => 'Pir������efs',lat => 37.9613889,lon => 23.6388889}, 
		 { name => 'Pireo',lat => 37.9613889,lon => 23.6388889}, 
		 { name => 'Salonica',lat => 40.6402778,lon => 22.9438889}, 
		 { name => 'Salonika',lat => 40.6402778,lon => 22.9438889}, 
		 { name => 'Salon������ki',lat => 40.6402778,lon => 22.9438889}, 
		 { name => 'Salonique',lat => 40.6402778,lon => 22.9438889}, 
		 { name => 'Selanik',lat => 40.6402778,lon => 22.9438889}, 
		 { name => 'Solun',lat => 40.6402778,lon => 22.9438889}, 
		 { name => 'Thessalon������k������',lat => 40.6402778,lon => 22.9438889}, 
		 { name => 'Thessalon������ki',lat => 40.6402778,lon => 22.9438889}, 
		 { name => 'Agram',lat => 45.8,lon => 16.0}, 
		 { name => 'Andautonia',lat => 45.8,lon => 16.0}, 
		 { name => 'Fiume',lat => 45.3430556,lon => 14.4091667}, 
		 { name => 'Fkumen',lat => 45.3430556,lon => 14.4091667}, 
		 { name => 'Flumen Sancti Viti',lat => 45.3430556,lon => 14.4091667}, 
		 { name => 'R������ka',lat => 45.3430556,lon => 14.4091667}, 
		 { name => 'Rieka',lat => 45.3430556,lon => 14.4091667}, 
		 { name => 'Rijeka',lat => 45.3430556,lon => 14.4091667}, 
		 { name => 'Sankt Veit am Flaum',lat => 45.3430556,lon => 14.4091667}, 
		 { name => 'Sankt Veit am Pflaum',lat => 45.3430556,lon => 14.4091667}, 
		 { name => 'Vitipolis',lat => 45.3430556,lon => 14.4091667}, 
		 { name => 'Zabrag',lat => 45.8,lon => 16.0}, 
		 { name => 'Zabreg',lat => 45.8,lon => 16.0}, 
		 { name => 'Zagabria',lat => 45.8,lon => 16.0}, 
		 { name => 'Z������gr������b',lat => 45.8,lon => 16.0}, 
		 { name => 'Zagrabia',lat => 45.8,lon => 16.0}, 
		 { name => 'Zagreb',lat => 45.8,lon => 16.0}, 
		 { name => 'Djumpandang',lat => -5.1463889,lon =>  119.4386111}, 
		 { name => 'Macassar',lat => -5.1463889,lon =>  119.4386111}, 
		 { name => 'Makasar',lat => -5.1463889,lon =>  119.4386111}, 
		 { name => 'Makassar',lat => 2.45,lon => 99.7833333}, 
		 { name => 'Makassar',lat => -5.1463889,lon =>  119.4386111}, 
		 { name => 'Makasser',lat => -5.1463889,lon =>  119.4386111}, 
		 { name => 'Udjungpadang',lat => 2.45,lon => 99.7833333}, 
		 { name => 'Udjung Pandang',lat => -5.1463889,lon =>  119.4386111}, 
		 { name => 'Ujungpadang',lat => 2.45,lon => 99.7833333}, 
		 { name => 'Ujungpandang',lat => -5.1463889,lon =>  119.4386111}, 
		 { name => 'Asumumbay',lat => 18.975,lon => 72.8258333}, 
		 { name => 'Bombay',lat => 18.975,lon => 72.8258333}, 
		 { name => 'Mumbai',lat => 18.975,lon => 72.8258333}, 
		 { name => 'Numbai',lat => 18.975,lon => 72.8258333}, 
		 { name => 'Ghar������b������-e K������chek',lat => 30.2075,lon => 49.685}, 
		 { name => 'Ghar������b������-e yakom',lat => 30.2075,lon => 49.685}, 
		 { name => 'Ghar������bi K������chik',lat => 30.2075,lon => 49.685}, 
		 { name => 'Ghar������b������-ye K������chek',lat => 30.2075,lon => 49.685}, 
		 { name => 'Ghar������b������-ye `Oly������',lat => 30.2075,lon => 49.685}, 
		 { name => '������oseyn������b������d',lat => 34.8558333,lon => 50.8583333}, 
		 { name => 'Na`������m������b������d',lat => 34.45,lon => 50.8683333}, 
		 { name => 'Kavagbouma',lat => 7.8533333,lon => -6.1066667}, 
		 { name => 'Kavagouma',lat => 7.8533333,lon => -6.1066667}, 
		 { name => 'Edo',lat => 35.685,lon =>  139.7513889}, 
		 { name => 'Tokio',lat => 35.685,lon =>  139.7513889}, 
		 { name => 'Tokyo',lat => 35.685,lon =>  139.7513889}, 
		 { name => 'T������ky������',lat => 35.685,lon =>  139.7513889}, 
		 { name => 'Heij������',lat => 39.0194444,lon =>  125.7547222}, 
		 { name => 'Heij������-fu',lat => 39.0194444,lon =>  125.7547222}, 
		 { name => 'Heizy������',lat => 39.0194444,lon =>  125.7547222}, 
		 { name => 'Heizy������ Hu',lat => 39.0194444,lon =>  125.7547222}, 
		 { name => 'Hpyeng-yang',lat => 39.0194444,lon =>  125.7547222}, 
		 { name => 'P-hj������ng-jang',lat => 39.0194444,lon =>  125.7547222}, 
		 { name => 'Phyeng-yang',lat => 39.0194444,lon =>  125.7547222}, 
		 { name => 'Phyong-yang',lat => 39.0194444,lon =>  125.7547222}, 
		 { name => 'Pienyang',lat => 39.0194444,lon =>  125.7547222}, 
		 { name => 'Pingyang',lat => 39.0194444,lon =>  125.7547222}, 
		 { name => 'Pyengyang',lat => 39.0194444,lon =>  125.7547222}, 
		 { name => 'Py������ngyang',lat => 39.0194444,lon =>  125.7547222}, 
		 { name => 'Pyongyang',lat => 39.0194444,lon =>  125.7547222}, 
		 { name => 'Choei-yuen',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'Hang-yang-tcheng',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'Hans������ng',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'H������n-yang',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'H������-seng',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'Kan-y������-j������',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'Keijo',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'Keizy������e',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'Kiung',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'Ky������ngs������ng',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'Seoul',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'Seul',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'S������ul',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'Suigen',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'Sye-ul',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'Syou-ouen',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'Wang-ching',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'W������-j������',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'R������ga',lat => 56.95,lon => 24.1}, 
		 { name => 'Rija',lat => 56.95,lon => 24.1}, 
		 { name => 'Caunas',lat => 54.9,lon => 23.9}, 
		 { name => 'Kauen',lat => 54.9,lon => 23.9}, 
		 { name => 'Kaunas',lat => 54.9,lon => 23.9}, 
		 { name => 'Kauno',lat => 54.9,lon => 23.9}, 
		 { name => 'Kovno',lat => 54.9,lon => 23.9}, 
		 { name => 'Kowno',lat => 54.9,lon => 23.9}, 
		 { name => 'Palemonas',lat => 54.9,lon => 23.9}, 
		 { name => 'Panevezhi',lat => 55.7333333,lon => 24.35}, 
		 { name => 'Panevezhis',lat => 55.7333333,lon => 24.35}, 
		 { name => 'Panev������������io',lat => 55.7333333,lon => 24.35}, 
		 { name => 'Panev������������ys',lat => 55.7333333,lon => 24.35}, 
		 { name => 'Poniewesch',lat => 55.7333333,lon => 24.35}, 
		 { name => 'Poniewiesh',lat => 55.7333333,lon => 24.35}, 
		 { name => 'Poniewie������',lat => 55.7333333,lon => 24.35}, 
		 { name => 'Schaulen',lat => 55.9333333,lon => 23.3166667}, 
		 { name => 'Shaulyay',lat => 55.9333333,lon => 23.3166667}, 
		 { name => 'Shavli',lat => 55.9333333,lon => 23.3166667}, 
		 { name => 'Shyaulyay',lat => 55.9333333,lon => 23.3166667}, 
		 { name => '������iauliai',lat => 55.9333333,lon => 23.3166667}, 
		 { name => '������iauli������',lat => 55.9333333,lon => 23.3166667}, 
		 { name => 'Sokniai',lat => 55.9333333,lon => 23.3166667}, 
		 { name => 'Szawle',lat => 55.9333333,lon => 23.3166667}, 
		 { name => 'Vilna',lat => 54.6833333,lon => 25.3166667}, 
		 { name => 'Vilnia',lat => 54.6833333,lon => 25.3166667}, 
		 { name => 'Vilnius',lat => 54.6833333,lon => 25.3166667}, 
		 { name => 'Vilno',lat => 54.6833333,lon => 25.3166667}, 
		 { name => 'Vilnyus',lat => 54.6833333,lon => 25.3166667}, 
		 { name => 'Wilna',lat => 54.6833333,lon => 25.3166667}, 
		 { name => 'Wilno',lat => 54.6833333,lon => 25.3166667}, 
		 { name => 'B������l������i',lat => 47.7616667,lon => 27.9288889}, 
		 { name => 'Baltsy',lat => 47.7616667,lon => 27.9288889}, 
		 { name => 'Beltsi',lat => 47.7616667,lon => 27.9288889}, 
		 { name => 'Beltsy',lat => 47.7616667,lon => 27.9288889}, 
		 { name => 'Bendary',lat => 46.8305556,lon => 29.4711111}, 
		 { name => 'Bender',lat => 46.8305556,lon => 29.4711111}, 
		 { name => 'Bendery',lat => 46.8305556,lon => 29.4711111}, 
		 { name => 'Byelcy',lat => 47.7616667,lon => 27.9288889}, 
		 { name => 'Chi������in������u',lat => 47.0055556,lon => 28.8575}, 
		 { name => 'Kischinew',lat => 47.0055556,lon => 28.8575}, 
		 { name => 'Kiscinev',lat => 47.0055556,lon => 28.8575}, 
		 { name => 'Kishinef',lat => 47.0055556,lon => 28.8575}, 
		 { name => 'Kishin������v',lat => 47.0055556,lon => 28.8575}, 
		 { name => 'Kishiniv',lat => 47.0055556,lon => 28.8575}, 
		 { name => 'Tighina',lat => 46.8305556,lon => 29.4711111}, 
		 { name => 'Tigina',lat => 46.8305556,lon => 29.4711111}, 
		 { name => 'Tiraspol',lat => 46.8402778,lon => 29.6433333}, 
		 { name => 'Tiraspol',lat => 46.8402778,lon => 29.6433333}, 
		 { name => 'Al Qunay������irah',lat => 34.2608333,lon => -6.5794444}, 
		 { name => 'Kenitra',lat => 34.2608333,lon => -6.5794444}, 
		 { name => 'Khenifra',lat => 34.2608333,lon => -6.5794444}, 
		 { name => 'Lyautey',lat => 34.2608333,lon => -6.5794444}, 
		 { name => 'Mina Hassan Tani',lat => 34.2608333,lon => -6.5794444}, 
		 { name => 'Port Laoti',lat => 34.2608333,lon => -6.5794444}, 
		 { name => 'Port-Lyautey',lat => 34.2608333,lon => -6.5794444}, 
		 { name => 'Rabat',lat => 34.0252778,lon => -6.8361111}, 
		 { name => 'Sal������',lat => 34.0394444,lon => -6.8027778}, 
		 { name => 'Sali',lat => 34.0394444,lon => -6.8027778}, 
		 { name => 'Sallee',lat => 34.0394444,lon => -6.8027778}, 
		 { name => 'Sla',lat => 34.0394444,lon => -6.8027778}, 
		 { name => 'Ville de Kenitra',lat => 34.2608333,lon => -6.5794444}, 
		 { name => 'Maale',lat => 4.1666667,lon => 73.5}, 
		 { name => 'Male',lat => 4.1666667,lon => 73.5}, 
		 { name => 'Bergen',lat => 60.3911111,lon => 5.3247222}, 
		 { name => 'Christiania',lat => 59.9166667,lon => 10.75}, 
		 { name => 'Kristiania',lat => 59.9166667,lon => 10.75}, 
		 { name => 'Nidaros',lat => 63.4166667,lon => 10.4166667}, 
		 { name => 'Oslo',lat => 59.9166667,lon => 10.75}, 
		 { name => 'Trondheim',lat => 63.4166667,lon => 10.4166667}, 
		 { name => 'Trondhjem',lat => 63.4166667,lon => 10.4166667}, 
		 { name => 'Basti Khokhar',lat => 28.4125,lon => 70.1027778}, 
		 { name => 'Seri Guria',lat => 34.4361111,lon => 73.0194444}, 
		 { name => 'La Palma',lat => 8.4027778,lon =>  -78.1452778}, 
		 { name => 'Felicitas Julia',lat => 38.7166667,lon => -9.1333333}, 
		 { name => 'Lisboa',lat => 38.7166667,lon => -9.1333333}, 
		 { name => 'Lisbon',lat => 38.7166667,lon => -9.1333333}, 
		 { name => 'Lissabon',lat => 38.7166667,lon => -9.1333333}, 
		 { name => 'Olisipo',lat => 38.7166667,lon => -9.1333333}, 
		 { name => 'Al met yevo',lat => 54.8833333,lon => 52.3333333}, 
		 { name => 'Al met yevsk',lat => 54.8833333,lon => 52.3333333}, 
		 { name => 'Archangel',lat => 64.5666667,lon => 40.5333333}, 
		 { name => 'Archangelsk',lat => 64.5666667,lon => 40.5333333}, 
		 { name => 'Arkhangel sk',lat => 64.5666667,lon => 40.5333333}, 
		 { name => 'Brezhnev',lat => 55.7561111,lon => 52.4288889}, 
		 { name => 'Kasan',lat => 55.75,lon => 49.1333333}, 
		 { name => 'Kazan ',lat => 55.75,lon => 49.1333333}, 
		 { name => 'Molotovsk',lat => 64.5666667,lon => 39.8333333}, 
		 { name => 'Molotowsk',lat => 64.5666667,lon => 39.8333333}, 
		 { name => 'Naberezhnyye Chelny',lat => 55.7561111,lon => 52.4288889}, 
		 { name => 'Nizhnekamsk',lat => 55.6383333,lon => 51.8169444}, 
		 { name => 'Nizhnekamskiy',lat => 55.6383333,lon => 51.8169444}, 
		 { name => 'Novo-Kholmogory',lat => 64.5666667,lon => 40.5333333}, 
		 { name => 'Novyye Kholmogory',lat => 64.5666667,lon => 40.5333333}, 
		 { name => 'Severodvinsk',lat => 64.5666667,lon => 39.8333333}, 
		 { name => 'Sudostroy',lat => 64.5666667,lon => 39.8333333}, 
		 { name => 'Dakar',lat => 14.6708333,lon =>  -17.4380556}, 
		 { name => 'Goeteborg',lat => 57.7166667,lon => 11.9666667}, 
		 { name => 'G������teborg',lat => 57.7166667,lon => 11.9666667}, 
		 { name => 'Goteburg',lat => 57.7166667,lon => 11.9666667}, 
		 { name => 'Gothenburg',lat => 57.7166667,lon => 11.9666667}, 
		 { name => 'Gottenborg',lat => 57.7166667,lon => 11.9666667}, 
		 { name => 'Malm������',lat => 55.6,lon => 13.0}, 
		 { name => 'Stockholm',lat => 59.3333333,lon => 18.05}, 
		 { name => 'Tukholma',lat => 59.3333333,lon => 18.05}, 
		 { name => 'Uppsala',lat => 59.85,lon => 17.6333333}, 
		 { name => 'V������ster������s',lat => 59.6166667,lon => 16.55}, 
		 { name => 'B������le',lat => 47.5666667,lon => 7.6}, 
		 { name => 'Basel',lat => 47.5666667,lon => 7.6}, 
		 { name => 'Basilea',lat => 47.5666667,lon => 7.6}, 
		 { name => 'Basle',lat => 47.5666667,lon => 7.6}, 
		 { name => 'Bern',lat => 46.9166667,lon => 7.4666667}, 
		 { name => 'Berna',lat => 46.9166667,lon => 7.4666667}, 
		 { name => 'Berne',lat => 46.9166667,lon => 7.4666667}, 
		 { name => 'Geneva',lat => 46.2,lon => 6.1666667}, 
		 { name => 'Gen������ve',lat => 46.2,lon => 6.1666667}, 
		 { name => 'Genf',lat => 46.2,lon => 6.1666667}, 
		 { name => 'Ginevra',lat => 46.2,lon => 6.1666667}, 
		 { name => 'Lausanne',lat => 46.5333333,lon => 6.6666667}, 
		 { name => 'Z������rich',lat => 47.3666667,lon => 8.55}, 
		 { name => 'Astacus',lat => 40.7669444,lon => 29.9169444}, 
		 { name => 'Boursa',lat => 40.1916667,lon => 29.0611111}, 
		 { name => 'Brossa',lat => 40.1916667,lon => 29.0611111}, 
		 { name => 'Broussa',lat => 40.1916667,lon => 29.0611111}, 
		 { name => 'Brousse',lat => 40.1916667,lon => 29.0611111}, 
		 { name => 'Brusa',lat => 40.1916667,lon => 29.0611111}, 
		 { name => 'Brussa',lat => 40.1916667,lon => 29.0611111}, 
		 { name => 'Bursa',lat => 40.1916667,lon => 29.0611111}, 
		 { name => 'Chalcedon',lat => 40.9855556,lon => 29.0294444}, 
		 { name => 'Cocaeli',lat => 40.7669444,lon => 29.9169444}, 
		 { name => 'Conia',lat => 37.8655556,lon => 32.4825}, 
		 { name => 'Gebze',lat => 40.7977778,lon => 29.4305556}, 
		 { name => 'Guebze',lat => 40.7977778,lon => 29.4305556}, 
		 { name => 'Iconium',lat => 37.8655556,lon => 32.4825}, 
		 { name => 'Ikoniow',lat => 37.8655556,lon => 32.4825}, 
		 { name => 'Ismid',lat => 40.7669444,lon => 29.9169444}, 
		 { name => 'Ismit',lat => 40.7669444,lon => 29.9169444}, 
		 { name => 'Isnimid',lat => 40.7669444,lon => 29.9169444}, 
		 { name => 'Izmid',lat => 40.7669444,lon => 29.9169444}, 
		 { name => '������zmit',lat => 40.7669444,lon => 29.9169444}, 
		 { name => 'Kadi Kioi',lat => 40.9855556,lon => 29.0294444}, 
		 { name => 'Kadik������i',lat => 40.9855556,lon => 29.0294444}, 
		 { name => 'Kad������k������y',lat => 40.9855556,lon => 29.0294444}, 
		 { name => 'Kartal',lat => 40.9033333,lon => 29.1725}, 
		 { name => 'Kocaeli',lat => 40.7669444,lon => 29.9169444}, 
		 { name => 'Kodja-Eli',lat => 40.7669444,lon => 29.9169444}, 
		 { name => 'Koja-Ili',lat => 40.7669444,lon => 29.9169444}, 
		 { name => 'Konia',lat => 37.8655556,lon => 32.4825}, 
		 { name => 'Konieh',lat => 37.8655556,lon => 32.4825}, 
		 { name => 'Konya',lat => 37.8655556,lon => 32.4825}, 
		 { name => 'Kuniyah',lat => 37.8655556,lon => 32.4825}, 
		 { name => 'Nicomedia',lat => 40.7669444,lon => 29.9169444}, 
		 { name => 'Nikomedeia',lat => 40.7669444,lon => 29.9169444}, 
		 { name => 'Pendik',lat => 40.8775,lon => 29.2513889}, 
		 { name => 'Prousa',lat => 40.1916667,lon => 29.0611111}, 
		 { name => 'Qonia',lat => 37.8655556,lon => 32.4825}, 
		 { name => 'Seyhan',lat => 37.0,lon => 35.3044444}, 
		 { name => 'Ouagadouga',lat => 12.3702778,lon => -1.5247222}, 
		 { name => 'Ouagadougou',lat => 12.3702778,lon => -1.5247222}, 
		 { name => 'Wagadugu',lat => 12.3702778,lon => -1.5247222}, 
		 { name => 'Donja Badanja',lat => 44.5,lon => 19.4758333}, 
		 { name => 'Ili������i',lat => 44.0477778,lon => 19.7783333}, 
		 { name => 'Joki������i',lat => 44.0491667,lon => 19.7847222}, 
		 { name => 'Maria-Theresianopel',lat => 46.1,lon => 19.6666667}, 
		 { name => 'Maria-Theresiopel',lat => 46.1,lon => 19.6666667}, 
		 { name => 'Maria-Theresiopolis',lat => 46.1,lon => 19.6666667}, 
		 { name => 'Nich',lat => 43.3247222,lon => 21.9033333}, 
		 { name => 'Ni������',lat => 43.3247222,lon => 21.9033333}, 
		 { name => 'Nisch',lat => 43.3247222,lon => 21.9033333}, 
		 { name => 'Nish',lat => 43.3247222,lon => 21.9033333}, 
		 { name => 'Nissa',lat => 43.3247222,lon => 21.9033333}, 
		 { name => 'Podgorica',lat => 42.4411111,lon => 19.2636111}, 
		 { name => 'Prishtin������',lat => 42.6666667,lon => 21.1666667}, 
		 { name => 'Pri������tina',lat => 42.6666667,lon => 21.1666667}, 
		 { name => 'Sawaditz',lat => 46.1,lon => 19.6666667}, 
		 { name => 'Slatina',lat => 44.6491667,lon => 19.6469444}, 
		 { name => 'Subotica',lat => 46.1,lon => 19.6666667}, 
		 { name => 'Szabadka',lat => 46.1,lon => 19.6666667}, 
		 { name => 'Szent-M������ria',lat => 46.1,lon => 19.6666667}, 
		 { name => 'Theresiopel',lat => 46.1,lon => 19.6666667}, 
		 { name => 'Titograd',lat => 42.4411111,lon => 19.2636111}, 
		 { name => 'Zubotica',lat => 46.1,lon => 19.6666667}, 
		     ) {
	    $loc->{"$t.name"} = $loc->{name};
	    $loc->{"$t.lat"}  = $loc->{lat};
	    $loc->{"$t.lon"}  = $loc->{lon};

	    my $delete_query=sprintf("DELETE FROM $t ".
				     "WHERE name = '%s' AND $type_query",
				     $loc->{"$t.name"});
	    POI::DBFuncs::db_exec( $delete_query);
	    POI::DBFuncs::insert_hash($t, $wp_defaults, $loc );
	}
    }
}

# -----------------------------------------------------------------------------
{   # Fill streets_type database
    my $i=1;
    my $lang;
    my $name;
    my $color;

    # Entries for WDB
    my $streets_type_id=0;
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
	    POI::DBFuncs::db_exec("DELETE FROM `streets_type` WHERE streets_type_id = '$streets_type_id' ;");
	    POI::DBFuncs::db_exec("INSERT INTO `streets_type` ".
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
	POI::DBFuncs::db_exec("DELETE FROM `streets_type` WHERE streets_type_id = '$streets_type_id';");
	POI::DBFuncs::db_exec("INSERT INTO `streets_type` ".
			      "        (streets_type_id,  name, description , color , linetype )".
			      " VALUES ($i,'$name','$name','$color','$linetype');");
	
	$i++;
    }

}

# -----------------------------------------------------------------------------
{ # Insert Street Sample
    my $street_samples= [
			 [ 48.171023, 11.717254 , 48.176608, 11.755316, 'Muenchner Strasse' ],
			 [ 48.176608, 11.755316 , 48.176608, 11.753603, 'Muenchner Strasse' ],
			 [ 48.176608, 11.753603 , 48.176311, 11.752174, 'Muenchner Strasse' ],
			 [ 48.176311, 11.752174 , 48.176365, 11.751729, 'Muenchner Strasse' ],
			 [ 48.176365, 11.751729 , 48.175277, 11.747885, 'Muenchner Strasse' ],
			 [ 48.175277, 11.747885 , 48.174962, 11.746091, 'ST2082' ],
			 [ 48.174962, 11.746091 , 48.174953, 11.745457, 'ST2082' ],
			 [ 48.170636, 11.745687 , 48.172192, 11.754857, 'ST2082' ],
			 [ 48.172192, 11.754857 , 48.172803, 11.760117, 'ST2082' ],
			 [ 48.175730, 11.592952 , 48.183299, 11.600967, 'A9'],
			 [ 48.183299, 11.600967 , 48.185509, 11.611364, 'A9'],
			 [ 48.185509, 11.611364 , 48.221761, 11.629305, 'A9'],
			 [ 48.221761, 11.629305 , 48.257066, 11.647272, 'A9'],
			 [ 48.257066, 11.647272 , 48.316307, 11.621176, 'A9'],
			 [ 48.209321, 11.490270 , 48.224827, 11.531849, 'A99'],
			 [ 48.224827, 11.531849 , 48.228994, 11.598545, 'A99'],
			 [ 48.228994, 11.598545 , 48.220817, 11.630251, 'A99'],
			 [ 48.220817, 11.630251 , 48.196558, 11.677531, 'A99'],
			 [ 48.196558, 11.677531 , 48.186770, 11.717243, 'A99'],
			 [ 48.186770, 11.717243 , 48.174819, 11.733519, 'A99'],
			 [ 48.174819, 11.733519 , 48.165509, 11.742361, 'A99'],
			 [ 48.165509, 11.742361 , 48.142865, 11.758766, 'A99'],
			 [ 48.142865, 11.758766 , 48.102533, 11.753495, 'A99'],
			 [ 48.102533, 11.753495 , 48.079226, 11.736481, 'A99'],
			 [ 48.079226, 11.736481 , 48.055602, 11.694953, 'A99'],
			 [ 48.055602, 11.694953 , 48.036071, 11.677027, 'A99'],
			 [ 48.036071, 11.677027 , 48.026925, 11.658198, 'A99'],
			 [ 48.026925, 11.658198 , 48.027874, 11.635108, 'A99'],
			 [ 48.027874, 11.635108 , 48.040772, 11.600678, 'A99'],
			 [ 48.040772, 11.600678 , 48.059367, 11.595002, 'A99'],
			 [ 48.059367, 11.595002 , 48.074182, 11.598279, 'A99'],
			 [ 48.074182, 11.598279 , 48.078912, 11.598744, 'A99'],
			 [ 48.078912, 11.598744 , 48.096235, 11.586925, 'A99'],
			 [ 48.137857, 11.616146 , 48.142866, 11.757822, 'A94'],
			 [ 48.142866, 11.757822 , 48.162578, 11.861800, 'A94'],
			 [ 48.219020, 11.638297 , 48.260627, 11.655309, 'A9'],
			 [ 48.260627, 11.655309 , 48.351377, 11.598382, 'A9'], 
			 [ 48.351377, 11.598382 , 48.578290, 11.592366, 'A9'],
			 [ 48.578290, 11.592366 , 48.759636, 11.457281, 'A9'],
			 [ 48.759636, 11.457281 , 48.929843, 11.468080, 'A9'],
			 [ 48.929843, 11.468080 , 49.007080, 11.355387, 'A9'],
			 [ 49.007080, 11.355387 , 49.024136, 11.369707, 'A9'],
			 [ 49.024136, 11.369707 , 49.044890, 11.355151, 'A9'],
			 [ 49.044890, 11.355151 , 49.080584, 11.288550, 'A9'],
			 [ 49.080584, 11.288550 , 49.180699, 11.267537, 'A9'],
			 [ 49.180699, 11.267537 , 49.218335, 11.223814, 'A9'],
			 [ 49.218335, 11.223814 , 49.395936, 11.198975, 'A9'],
			 [ 48.578292, 11.595221 , 48.908312, 12.098215, 'A93'],
			 [ 48.908312, 12.098215 , 48.989772, 12.061462, 'A93'],
			 [ 48.991638, 12.067248 , 48.999180, 12.404532, 'A3'],
			 [ 48.999180, 12.404532 , 48.821822, 12.936121, 'A3'],
			 [ 48.319239, 11.624027 , 48.817791, 12.956124, 'A92'],
			 
			 ];

    my $source_name = "Defaults";
    my $source_id = source_name2id($source_name);
    debug("$source_name --> '$source_id'\n");
    die "Unknown Source ID: $source_name\n" unless $source_id;

    delete_all_from_source($source_name);

    for my $segment ( @{$street_samples} ) {
	my $s4db;
	$s4db->{'streets.lat1'} = $segment->[0];
	$s4db->{'streets.lon1'} = $segment->[1];
	$s4db->{'streets.lat2'} = $segment->[2];
	$s4db->{'streets.lon2'} = $segment->[3];

	$s4db->{'streets.name'}            = $segment->[4];

	$s4db->{'streets.source_id'} = $source_id;
	$s4db->{'streets.streets_type_id'} = '';
	if ( $s4db->{'streets.name'} =~ m/^A/ ) {
	    $s4db->{'streets.streets_type_id'} = streets_type_name2id('Strassen.Autobahn');
	} elsif ( $s4db->{'streets.name'} =~ m/^ST/ ) {
	    $s4db->{'streets.streets_type_id'} = streets_type_name2id('Strassen.Bundesstrasse');
	} elsif ( $s4db->{'streets.name'} =~ m/^B/ ) {
	    $s4db->{'streets.streets_type_id'} = streets_type_name2id('Strassen.Bundesstrasse');
	}   else {
	    $s4db->{'streets.streets_type_id'} = streets_type_name2id('Strassen.Allgemein');
	}

	$s4db->{'streets.scale_min'}       = 1;
	$s4db->{'streets.scale_max'}       = 50000;
	$s4db->{'streets.last_modified'}   = localtime(time());

	#print Dumper(\$s4db);
	POI::DBFuncs::insert_hash("streets",$s4db);
    }

}

print "Creation completed\n";
}
# -----------------------------------------------------------------------------

1;
