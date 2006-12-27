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



# -----------------------------------------------------------------------------
# Fill poi_type database
sub fill_default_poi_types() {
    my $i=1;
    my $used_icons ={};
    my $poi_type_id=20;

    # for debug purpose
    Geo::Gpsdrive::DBFuncs::db_exec("TRUNCATE TABLE `poi_type`;");

    # insert base poi-types into database
    Geo::Gpsdrive::DBFuncs::db_exec("DELETE FROM `poi_type` WHERE poi_type_id < $poi_type_id ;");
    for my $val (
		 (
		  "1,'unknown','1','25000','Unassigned POI'",
		  "2,'accomodation','1','25000','Places to stay'",
		  "3,'education','1','25000','Schools and other educational facilities'",
		  "4,'food','1','25000','Restaurants, Bars, and so on...'",
		  "5,'geocache','1','25000','Geocaches'",
		  "6,'health','1','25000','Hospital, Doctor, Pharmacy, etc.'",
		  "7,'money','1','25000','Bank, ATMs, and other money-related places'",
		  "8,'nautical','1','25000','Special Aeronautical Points'",
		  "9,'people','1','25000','You, work, your friends, and other people'",
		  "10,'places','1','25000','Settlements, Mountains, and other geographical stuff'",
		  "11,'public','1','25000','Public facilities'",
		  "12,'recreation','1','25000','Places used for recreation (no sports)'",
		  "13,'religion','1','25000','Places and facilities related to religion'",
		  "14,'shopping','1','25000','All the places, where you can buy something'",
		  "15,'sightseeing','1','25000','Historic places and other interesting buildings'",
		  "16,'sports','1','25000','Sports clubs, stadiums, and other sports facilities'",
		  "17,'transport','1','25000','Public transportation'",
		  "18,'vehicle','1','25000','Facilites for drivers, like gas stations or parking places'",
		  "19,'wlan','1','25000','WiFi-related points (Kismet)'",
		  "20,'misc','1','25000','POIs not suitable for another category, and custom types'")
		 ){
	my $insert="INSERT INTO poi_type ".
	    "(poi_type_id,name,scale_min,scale_max,title) ".
	    "VALUES ($val);";
	Geo::Gpsdrive::DBFuncs::db_exec($insert)
	    or die "Error in Database Insert: $val\n$insert";
    }

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
#	next if $icon =~ m,icons/classic/,;
#	next unless -s $icon;
	$icon =~s,([^/]+/){4},,;
	$icon =~s,.*(classic|square\.big|square\.small)/?,,;
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
    while ( my $line = $fh->getline() ) {
	chomp $line;
	next if $line =~  m/^\#/;
	next if not $line =~ m/\./;	# skip base icons
	my ($name,$scale_min,$scale_max) = split(/\s+/,$line);
	#print "($name,$scale_min,$scale_max)\n";
	my $icon = $name;
	$icon =~ s,\.,\/,g;
	$icon = "$icon.png";
	warn "Icon $icon missing in Filesystem\n for Line:\n$line\n"
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
			    x00FFFF_x555555_w1_2_z00100000_Strassen.30_Zone

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


# Here comes some icon translation stuff that isn't used anymore, and can be
# removed in the future, when we realized a way to implement the translation
# of the icon names.
#
#
# Some suggestions for pictures
#  education.nursery ==> Schnuller
#  Entrance          ==> Door
#  police            ==> Sherrif Stern
#  park_and_ride     ==> blaues P + Auto + xxx
#  recreation.fairground    ==> Riessenrad
#  gift_shop         ==> Geschenk Paeckchen
#  model.airplanes   ==> Airplane + Fernsteuerung
#  Bei Sportarten evtl. die Olympischen Symbole verwenden
#  Zoo               ==> Giraffe+ Voegel
#  WC                ==> WC oder 00
#  Recycling         ==> R oder Gruene Punkt

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


1;
