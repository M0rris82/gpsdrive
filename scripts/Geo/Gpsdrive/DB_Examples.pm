# Database Defaults for poi/streets Table for poi.pl
#
# $Log$
# Revision 1.1  2005/10/11 08:28:35  tweety
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

package Geo::Gpsdrive::DB_Examples;

use strict;
use warnings;

use POSIX qw(strftime);
use Time::Local;
use DBI;
use Data::Dumper;
use IO::File;

use Geo::Gpsdrive::DBFuncs;
use Geo::Gpsdrive::Utils;

my $example_source_name = "Examples";
my $example_source_id=0;

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
sub fill_example_waypoints() {
    # Insert Example Waypoints


    die "No Example Source ID defined\n" unless $example_source_id;

    my @way_points= 
	qw(
	   Joerg                   48.17       11.75     developer

	   HBF_Muenchen            48.140544   11.557022 transport.public.railway
	   Reiten_Windhof          48.194209   11.900162 recreation.sports.horse.riding
	   Dr._Batze               48.175205   11.754619 health.doctor
	   Kirchheim_Zentrum       48.176734   11.754147 area.city
	   Tankstelle              48.188241   11.779348 transport.car.gas_station
	   Tankstelle_Shell_Blechinger  48.171572   11.720981 transport.car.gas_station
	   Baywa                   48.170465   11.799288 shopping.hardware_store
	   Raeter_Hotel            48.163887   11.756737 accomodation.hotel
	   Raeter_Stuben           48.163833   11.756723 food.restaurant
	   La_Piazza               48.162924   11.756845 food.restaurant.italian
	   Thai                    48.162133   11.756346 food.restaurant.asia
	   Apotheke                48.161818   11.757155 health.pharmacy
	   Baguette_Laden          48.161791   11.757034 food.restaurant.fastfood
	   Magoo                   48.163626   11.756090 food.restaurant
	   Tengelmann              48.163689   11.757546 shopping
	   Lidl                   48.177516   11.762886 shopping.supermarket.lidl
	   Dr._Hingerle            48.174135   11.751360 health.doctor
	   Apotheke                48.174206   11.751172 health.pharmacy
	   Schreibwaren            48.174072   11.751118 shopping
	   Sparkasse               48.176653   11.752825 bank
	   Baecker                 48.176482   11.754983 shopping
	   Spar_Greger             48.176374   11.757478 shopping
	   Post                    48.176500   11.756615 postal.office
	   Briefkasten             48.176500   11.756615 postal.letter-box
	   Metzger                 48.176806   11.756493 shopping
	   Second_Hand_Laden       48.176428   11.756992 shopping
	   Markt_(Donnerstag)      48.176617   11.755940 shopping.food.market
	   Da_Rosa                 48.176230   11.755064 shopping.food.vegetables
	   Da_Rosa                 48.176230   11.755064 food.restaurant.ice_cream_parlor
	   Pizzeria                48.175978   11.755091 food.restaurant
	   Pizzeria                48.171913   11.720469 food.restaurant
	   Pizzeria                48.171455   11.721494 food.restaurant
	   Supermarrkt_xy          48.171661   11.718851 shopping
	   Elektroladen            48.171769   11.719889 shopping
	   Eisdiele                48.170978   11.716464 food.restaurant
	   Baeckerei               48.170861   11.716558 shopping
	   Fussballfeld            48.172786   11.724528 recreation.sports.soccer_field
	   Aldi                    48.165622   11.729018 shopping.supermarket
	   Tankstelle_xy           48.165748   11.730758 transport.car.gas_station
	   Plus                    48.165353   11.730110 shopping
	   Squash                  48.164669   11.723948 recreation.sport
	   Wasserski               48.164076   11.724973 recreation.sport
	   Tauchschule             48.163950   11.724933 recreation.sport
	   Tennis                  48.164525   11.723045 recreation.sport
	   Biergarten              48.163311   11.721697 food.restaurant
	   Eisstockschiessen       48.163860   11.723395 recreation.sport
	   Kiramers                48.176482   11.754349 food.restaurant
	   Apotheke                48.174918   11.754311 health.pharmacy
	   Fahrschule              48.175005   11.754557 shopping
	   Friseur                 48.175046   11.754804 shopping
	   Blumenladen             48.175579   11.755145 shopping
	   Juwelier                48.175446   11.755172 shopping
	   GetrÃ¤nke_Jansen        48.175862   11.754794 shopping.beverages
	   Buecherladen            48.176356   11.755023 shopping
	   Rathaus                 48.176734   11.753756 office.town_hall
	   RolladenLaden           48.175160   11.755226 shopping
	   Sanitaer_Laden          48.175295   11.755226 shopping
	   Schneider               48.176482   11.754848 shopping
	   Biergarten              48.176842   11.756426 food.restaurant
	   Brotzeit_Stand          48.176383   11.766568 food.restaurant
	   Gockerl_Stand_Donnerstag    48.176644   11.753324 food.restaurant
	   Kirche                  48.176689   11.754551 church
	   Jugendzentrum           48.168826   11.754294 recreation
	   Edeka                   48.162034   11.756751 shopping
	   Uebler_Reinigung        48.163320   11.757249 shopping.dry_cleaner
	   Baecker                 48.163635   11.756737 shopping.food.bakery
	   Sportladen              48.162780   11.757627 shopping.sport
	   S-Bahn                  48.158912   11.759474 transport.public.station.subway_regional
	   Kirche                  48.191175   11.869115 church
	   Fahrschule              48.190905   11.868224 shopping
	   Optiker                 48.190959   11.868777 shopping
	   Eisdiele                48.190977   11.867239 food.restaurant.ice_cream_parlor
	   Zooladen                48.190950   11.867442 shopping
	   Second_Hand_Laden       48.190959   11.867617 shopping
	   Asiatisch               48.190905   11.869492 food.restaurant.asia
	   Copy_Shop               48.192973   11.871031 shopping
	   Reiten_Wind1            48.192518   11.899406 recreation.sports.horse.riding
	   Reiterhof_Spitzauer     48.130794   11.784921 recreation.sports.horse.riding
	   Segmueller              48.146962   11.784560 shopping.furniture
	   Segmueller_Warenlage    48.145361   11.780786 shopping
	   Anglerbedarf            48.146414   11.786878 shopping
	   Sportgastaette          48.174585   11.811596 food.restaurant
	   Tankstelle              48.168559   11.792290 transport.car.gas_station
	   Getraenkemarkt          48.170187   11.799126 shopping
	   Bauhof                  48.173496   11.739196 recycling
	   Feuerwehr               48.173011   11.757114 office.fire_department
	   Grundschule             48.169188   11.756346 education
	   Gymnasium               48.169323   11.756845 education
	   Collegium_2000          48.164094   11.759946 accomodation.nursing_home
	   Heimstetter_See         48.155602   11.738648 recreation.sports.swimming_area
	   Fussballfeld            48.162854   11.746864 recreation.sports.soccer_field
	   Sportgaststaette        48.163376   11.748455 food.restaurant
	   Gaststaette_xy          48.158462   11.756751 food.restaurant
	   Metzger                 48.159496   11.754850 shopping
	   Halfpipes               48.167691   11.741677 recreation
	   Halfpipe                48.172012   11.750573 recreation
	   Kirche                  48.173199   11.750241 church
	   Altglas/Papier          48.174162   11.741462 recycling
	   EC_Automat              48.176473   11.756628 bank.ATM.EC
	   Reifeisen_Banke         48.176761   11.755576 bank.agency.Reifeisenbank
	   Friseur                 48.175079   11.755738 shopping
	   Pizzeria                48.187856   11.776186 food.restaurant
	   Baeckerei               48.187793   11.774661 shopping
	   Baeckerei               48.197500   11.798252 shopping
	   Gaststaette             48.198373   11.798130 food.restaurant
	   Aldi                    48.190932   11.851441 shopping.supermarket
	   Marktkauf               48.192128   11.850861 shopping.supermarket
	   Doener_Stand            48.191876   11.850956 food.restaurant
	   Pizza_Stand             48.191157   11.851765 food.restaurant
	   Schuhladen              48.190275   11.850214 shopping
	   S-Bahn                  48.192940   11.861357 transport.public.station.subway_regional
	   WuW_House_of_Noteboo    48.136191   11.560120 shopping.computer
	   Litec_Computer          48.137324   11.561805 shopping.computer
	   PC_Fun                  48.136991   11.560794 shopping.computer.games
	   Allnet_Factory_Outle    48.136659   11.560808 shopping.computer
	   Holzinger               48.136185   11.561050 shopping.electronics
	   Bus                     48.176554   11.754335 transport.public.station.bus
	   Bus                     48.176608   11.754349 transport.public.station.bus
	   Bus                     48.171059   11.717057 transport.public.station.bus
	   Bus                     48.171230   11.717273 transport.public.station.bus
	   Sport_Laden             48.171940   11.719673 shopping.sport
	   Billard_Cafee           48.173640   11.770177 food.restaurant
	   Tauchladen              48.173514   11.770231 shopping.sport.diving
	   Biergarten              48.157494   11.736825 food.restaurant
	   Sportgastaette          48.174477   11.762279 food.restaurant
	   Minigolfplatz           48.174081   11.760863 recreation.sports.minigolf_place
	   HypoVereinsbank         48.158372   11.754764 bank.agency.HypoVereinsbank
	   EC                      48.158372   11.754764 bank.ATM.EC
	   VinzenzMurr             48.171374   11.808239 shopping
	   Tengelmann              48.171266   11.807969 shopping
	   Klamotten               48.171221   11.807740 shopping
	   Penny                   48.171041   11.807322 shopping
	   Doener_Stand            48.170465   11.799383 food.restaurant.fastfood
	   Penny                   48.170753   11.805717 shopping
	   Esprit                  48.170843   11.805987 shopping
	   Takko_Fschion           48.170897   11.806310 shopping
	   Entrance                48.170942   11.806553 entrance
	   EC                      48.170960   11.806661 bank.ATM.EC
	   Video_Verleih           48.170987   11.806850 shopping
	   Herz-Apotheke           48.171014   11.807052 health.pharmacy
	   Mueller_Brot            48.171050   11.807187 shopping
	   Tengelmann              48.171095   11.807376 shopping
	   Zahnarzt                48.171158   11.807605 health.doctor
	   Feuerwehr               48.173838   11.810315 office.fire_department
	   Gasthaus                48.172741   11.806553 food.restaurant
	   Grieche                 48.172201   11.806081 food.restaurant
	   Pasterlle               48.171509   11.808845 food.restaurant.italian
	   FTI                     48.171437   11.808913 transport.travel-agency
	   Allianz                 48.171356   11.808980 shopping
	   Mueller                 48.171284   11.809048 shopping.food.bakery
	   PC-Ware                 48.171563   11.808967 shopping.computer
	   New_Mandarin            48.171491   11.809344 food.restaurant.chinese
	   Sparkasse               48.170852   11.812284 bank.agency.Sparkasse
	   Kosmetic                48.170672   11.812271 shopping
	   Schlecker               48.170250   11.812176 shopping
	   Gelateria_Segafredo     48.170510   11.812284 food.restaurant
	   OeMV-_Heizoehl          48.170000   11.811286 shopping
	   Gartencenter            48.170000   11.811286 shopping
	   Heimtierbedarf          48.170000   11.811286 shopping
	   Obst_Gemuese            48.170061   11.812109 shopping
	   Gastwirtschaft          48.169872   11.812068 food.restaurant
	   Immobilien_Markler      48.170052   11.811799 shopping
	   Gastwirtschaft          48.170097   11.812487 food.restaurant
	   Maler                   48.170088   11.812972 shopping
	   Edeka                   48.170663   11.814698 shopping
	   Blumen_Kunst_mehr       48.170960   11.814105 shopping
	   Win_Haeusl              48.170969   11.814348 shopping
	   Baeckerei_Nagler        48.170996   11.814860 shopping
	   Cafee_Nagler            48.171005   11.815049 shopping
	   EP_Wondra               48.171041   11.815318 shopping
	   Sportplatz              48.174818   11.811961 recreation.sports.soccer_field
	   Getraenkehandel         48.157949   11.754063 shopping
	   Getraenke               48.162518   11.758661 shopping
	   Tankstelle              48.165811   11.730475 transport.car.gas_station 
	   );

    my $error=0;
    # Insert Example Waypoints

    while (@way_points){
	my $name      = shift @way_points;
	my $lat       = shift @way_points;
	my $lon       = shift @way_points;
	my $type_name = shift @way_points;
	
	debug("\n\nInsert Waypoint '$name' '$lat','$lon' '$type_name'");

	die "Fehler in lat,lon: '$lat','$lon' for $name,'$type_name'\n"
	    unless ( $lat =~ m/^\s*\d+\.\d+\s*$/  &&
		     $lon =~ m/^\s*\d+\.\d+\s*$/ );

	$type_name =~ s/_/ /g;
	my $type_id = poi_type_name2id($type_name);
	unless ( $type_id ){
	    $error++;
	    die "--------------- Type for '$type_name' not found \n";
	    next;
	}
	
	#for  my $t ( qw(waypoints poi)) {
	{
	    my $t = 'poi';
	    my $loc  = { "$t.name" => $name, "$t.lat" => $lat, "$t.lon" => $lon};
	    my $wp_defaults = { "$t.wep"         => 0 ,
				"$t.nettype"     => '',
				"$t.scale_min"   => 1,
				"$t.scale_max"   => 1000000000,
				"$t.source_id"   => $example_source_id,
				"$t.poi_type_id" => $type_id,
				"$t.last_modified" => time(),
			    };
	    #print "Sample WP:$t	'$name'\n";
	    Geo::Gpsdrive::DBFuncs::insert_hash($t, $wp_defaults, $loc );
	}
    }
}

# ------------------------------------------------------------------
sub fill_example_cities(){
    # Insert Example Cities
    my $type_name = "area.city";
    my $type_id = poi_type_name2id($type_name);
    die "Type for '$type_name' not found \n" unless $type_id;

    die "No Example Source ID defined\n" unless $example_source_id;

#    for  my $t ( qw(waypoints poi)) {
    {
	my $t = 'poi';
	my $wp_defaults = { "$t.wep"         => 0 ,
			    "$t.nettype"     => '',
			    "$t.scale_min"   => 1,
			    "$t.scale_max"   => 1000000000,
			    "$t.type"        => 'City',
			    "$t.source_id"   => $example_source_id,
			    "$t.poi_type_id" => $type_id,
			    "$t.last_modified" => time(),
			};
	my $type_query = $t  eq "poi" 
	    ? " poi_type_id = '$type_id'" 
	    : " type = '". $wp_defaults->{"$t.type"} ."'";
	
	for my $loc (
		 { name => 'Mechtat A_2ne el Msad',lat => 36.5033333,lon => 6.0758333}, 
		 { name => 'Halte A_2n Lehma',lat => 36.1166667,lon => 6.4833333}, 
		 { name => 'Halte d A_2n-Lehms',lat => 36.1166667,lon => 6.4833333}, 
		 { name => 'Berteau A_2n Lehma',lat => 36.1166667,lon => 6.4833333}, 
		 { name => 'Berteaux-A_2n Lehma',lat => 36.1166667,lon => 6.4833333}, 
		 { name => 'Bordjel El Hassane',lat => 36.6027778,lon => 7.6916667}, 
		 { name => 'Borj Scander',lat => 36.8347222,lon => 7.7013889}, 
		 { name => 'Bouchegout',lat => 36.4722222,lon => 7.7333333}, 
		 { name => 'Dar Lahleme',lat => 36.8347222,lon => 7.575}, 
		 { name => 'Djenane El Bey',lat => 36.8305556,lon => 7.1180556}, 
		 { name => 'Henchir Mokhtar Abdullah el Medjid',lat => 36.6388889,lon => 7.7083333}, 
		 { name => 'Mechetet Bellchari',lat => 36.6083333,lon => 7.5805556}, 
		 { name => 'Mechta A_2n el Msad',lat => 36.5033333,lon => 6.0758333}, 
		 { name => 'Mechta Beni Hameza',lat => 36.6694444,lon => 7.4347222}, 
		 { name => 'Mechta Fedj el Ouathia',lat => 36.2583333,lon => 7.9861111}, 
		 { name => 'Mechta Fetmiat',lat => 36.7680556,lon => 7.3166667}, 
		 { name => 'Mechta Guebel A_2n',lat => 36.0013889,lon => 7.7819444}, 
		 { name => 'Mechta GuebelA_2n',lat => 36.0027778,lon => 7.7805556}, 
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
		 { name => 'Potos_2',lat =>  -19.5836111,lon =>  -65.7530556}, 
		 { name => 'Sucre',lat =>  -19.0430556,lon =>  -65.2591667}, 
		 { name => 'Bazargic',lat => 43.5666667,lon => 27.8333333}, 
		 { name => 'Bourgas',lat => 42.5,lon => 27.4666667}, 
		 { name => 'Burgas',lat => 42.5,lon => 27.4666667}, 
		 { name => 'Burgaz',lat => 42.5,lon => 27.4666667}, 
		 { name => 'Burghaz',lat => 42.5,lon => 27.4666667}, 
		 { name => 'Choum_2n',lat => 43.2766667,lon => 26.9291667}, 
		 { name => 'Dobri_2',lat => 43.5666667,lon => 27.8333333}, 
		 { name => 'Dobrich',lat => 43.5666667,lon => 27.8333333}, 
		 { name => 'Dobritch',lat => 43.5666667,lon => 27.8333333}, 
		 { name => 'Dobritsch',lat => 43.5666667,lon => 27.8333333}, 
		 { name => 'Eski Zagra',lat => 42.4327778,lon => 25.6419444}, 
		 { name => 'Eumolpias',lat => 42.15,lon => 24.75}, 
		 { name => 'Filiba',lat => 42.15,lon => 24.75}, 
		 { name => 'Filipopol',lat => 42.15,lon => 24.75}, 
		 { name => 'Filippopol',lat => 42.15,lon => 24.75}, 
		 { name => 'Flavia',lat => 42.15,lon => 24.75}, 
		 { name => 'Had_2i-Oghlu-Pazard_2ik',lat => 43.5666667,lon => 27.8333333}, 
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
		 { name => 'Rouss_2',lat => 43.8563889,lon => 25.9708333}, 
		 { name => 'Roustchouk',lat => 43.8563889,lon => 25.9708333}, 
		 { name => 'Ruschuk',lat => 43.8563889,lon => 25.9708333}, 
		 { name => 'Ruschuq',lat => 43.8563889,lon => 25.9708333}, 
		 { name => 'Rusciuk',lat => 43.8563889,lon => 25.9708333}, 
		 { name => 'Rusclink',lat => 43.8563889,lon => 25.9708333}, 
		 { name => 'Ru_2_2uk',lat => 43.8563889,lon => 25.9708333}, 
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
		 { name => '_2umen',lat => 43.2766667,lon => 26.9291667}, 
		 { name => '_2umla',lat => 43.2766667,lon => 26.9291667}, 
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
		 { name => 'Kombo Lot_2',lat => 3.4830556,lon => 9.7280556}, 
		 { name => 'Bangui',lat => 4.3666667,lon => 18.5833333}, 
		 { name => 'Aalborg',lat => 57.05,lon => 9.9333333}, 
		 { name => 'Aarhus',lat => 56.15,lon => 10.2166667}, 
		 { name => '_2lborg',lat => 57.05,lon => 9.9333333}, 
		 { name => '_2rhus',lat => 56.15,lon => 10.2166667}, 
		 { name => 'Copenhagen',lat => 55.6666667,lon => 12.5833333}, 
		 { name => 'Kjobenhavn',lat => 55.6666667,lon => 12.5833333}, 
		 { name => 'K_2benhavn',lat => 55.6666667,lon => 12.5833333}, 
		 { name => 'Odense',lat => 55.4,lon => 10.3833333}, 
		 { name => 'Qaryat `Al_2 Bin Ab_2 _2_2lib',lat => 26.7005556,lon => 31.4236111}, 
		 { name => 'Baile _2tha Cliath',lat => 53.3330556,lon => -6.2488889}, 
		 { name => 'Corcaigh',lat => 51.8986111,lon => -8.4958333}, 
		 { name => 'Cork',lat => 51.8986111,lon => -8.4958333}, 
		 { name => 'Dublin',lat => 53.3330556,lon => -6.2488889}, 
		 { name => 'D_2n Laoghaire',lat => 53.2925,lon => -6.1286111}, 
		 { name => 'Dunleary',lat => 53.2925,lon => -6.1286111}, 
		 { name => 'Kingstown',lat => 53.2925,lon => -6.1286111}, 
		 { name => 'Derpt',lat => 58.3661111,lon => 26.7361111}, 
		 { name => 'Dorpat',lat => 58.3661111,lon => 26.7361111}, 
		 { name => 'Kallinn',lat => 59.4338889,lon => 24.7280556}, 
		 { name => 'Kolyvan',lat => 59.4338889,lon => 24.7280556}, 
		 { name => 'R_2_2veli',lat => 59.4338889,lon => 24.7280556}, 
		 { name => 'Reval',lat => 59.4338889,lon => 24.7280556}, 
		 { name => 'Revel',lat => 59.4338889,lon => 24.7280556}, 
		 { name => 'Talinas',lat => 59.4338889,lon => 24.7280556}, 
		 { name => 'Tallin',lat => 59.4338889,lon => 24.7280556}, 
		 { name => 'Tallina',lat => 59.4338889,lon => 24.7280556}, 
		 { name => 'Tallinn',lat => 59.4338889,lon => 24.7280556}, 
		 { name => 'Tallinna',lat => 59.4338889,lon => 24.7280556}, 
		 { name => 'Tartto',lat => 58.3661111,lon => 26.7361111}, 
		 { name => 'Tartu',lat => 58.3661111,lon => 26.7361111}, 
		 { name => 'T_2rbata',lat => 58.3661111,lon => 26.7361111}, 
		 { name => 'Yurev',lat => 58.3661111,lon => 26.7361111}, 
		 { name => 'Yuryev',lat => 58.3661111,lon => 26.7361111}, 
		 { name => 'Pilsen',lat => 49.75,lon => 13.3666667}, 
		 { name => 'Plze_2',lat => 49.75,lon => 13.3666667}, 
		 { name => '_2bo',lat => 60.45,lon => 22.2833333}, 
		 { name => 'Esbo',lat => 60.2166667,lon => 24.6666667}, 
		 { name => 'Espoo',lat => 60.2166667,lon => 24.6666667}, 
		 { name => 'Helsingfors',lat => 60.1755556,lon => 24.9341667}, 
		 { name => 'Helsinki',lat => 60.1755556,lon => 24.9341667}, 
		 { name => 'Khelsinki',lat => 60.1755556,lon => 24.9341667}, 
		 { name => 'Oulu',lat => 65.0166667,lon => 25.4666667}, 
		 { name => 'Tammerfors',lat => 61.5,lon => 23.75}, 
		 { name => 'Tampere',lat => 61.5,lon => 23.75}, 
		 { name => 'Turku',lat => 60.45,lon => 22.2833333}, 
		 { name => 'Ule_2borg',lat => 65.0166667,lon => 25.4666667}, 
		 { name => 'Uleoborg',lat => 65.0166667,lon => 25.4666667}, 
		 { name => 'Vanda',lat => 60.3,lon => 24.85}, 
		 { name => 'Vantaa',lat => 60.3,lon => 24.85}, 
		 { name => 'Yelsinki',lat => 60.1755556,lon => 24.9341667}, 
		 { name => 'Paris',lat => 48.8666667,lon => 2.3333333}, 
		 { name => 'Godthaab',lat => 64.1833333,lon =>  -51.75}, 
		 { name => 'Godth_2b',lat => 64.1833333,lon =>  -51.75}, 
		 { name => 'Nuk',lat => 64.1833333,lon =>  -51.75}, 
		 { name => 'Nuuk',lat => 64.1833333,lon =>  -51.75}, 
		 { name => 'Augusta Ubiorum',lat => 50.9333333,lon => 6.95}, 
		 { name => 'Berlin',lat => 52.5166667,lon => 13.4}, 
		 { name => 'Bremen',lat => 53.0833333,lon => 8.8}, 
		 { name => 'C_2ln',lat => 50.9333333,lon => 6.95}, 
		 { name => 'Cologne',lat => 50.9333333,lon => 6.95}, 
		 { name => 'Colonia Agrippina',lat => 50.9333333,lon => 6.95}, 
		 { name => 'Colonia Agrippinensis',lat => 50.9333333,lon => 6.95}, 
		 { name => 'Dortmund',lat => 51.5166667,lon => 7.45}, 
		 { name => 'Duisburg',lat => 51.4333333,lon => 6.75}, 
		 { name => 'Duisburg and Hamborn',lat => 51.4333333,lon => 6.75}, 
		 { name => 'Duisburg-Hamborn',lat => 51.4333333,lon => 6.75}, 
		 { name => 'D_2sseldorf',lat => 51.2166667,lon => 6.7666667}, 
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
		 { name => 'K_2ln',lat => 50.9333333,lon => 6.95}, 
		 { name => 'Monaco',lat => 48.15,lon => 11.5833333}, 
		 { name => 'Muenchen',lat => 48.15,lon => 11.5833333}, 
		 { name => 'M_2nchen',lat => 48.15,lon => 11.5833333}, 
		 { name => 'Munich',lat => 48.15,lon => 11.5833333}, 
		 { name => 'Stuttgart',lat => 48.7666667,lon => 9.1833333}, 
		 { name => 'L_2risa',lat => 39.6372222,lon => 22.4202778}, 
		 { name => 'L_2rissa',lat => 39.6372222,lon => 22.4202778}, 
		 { name => 'Le Pir_2e',lat => 37.9613889,lon => 23.6388889}, 
		 { name => 'P_2tra',lat => 38.2444444,lon => 21.7344444}, 
		 { name => 'Patrae',lat => 38.2444444,lon => 21.7344444}, 
		 { name => 'P_2trai',lat => 38.2444444,lon => 21.7344444}, 
		 { name => 'Patras',lat => 38.2444444,lon => 21.7344444}, 
		 { name => 'Patrasse',lat => 38.2444444,lon => 21.7344444}, 
		 { name => 'Peiraeus',lat => 37.9613889,lon => 23.6388889}, 
		 { name => 'Peirai_2',lat => 37.9613889,lon => 23.6388889}, 
		 { name => 'Peirai_2fs',lat => 37.9613889,lon => 23.6388889}, 
		 { name => 'Peiraieus',lat => 37.9613889,lon => 23.6388889}, 
		 { name => 'Peiraievs',lat => 37.9613889,lon => 23.6388889}, 
		 { name => 'Piraeus',lat => 37.9613889,lon => 23.6388889}, 
		 { name => 'Pirai_2vs',lat => 37.9613889,lon => 23.6388889}, 
		 { name => 'Pir_2efs',lat => 37.9613889,lon => 23.6388889}, 
		 { name => 'Pireo',lat => 37.9613889,lon => 23.6388889}, 
		 { name => 'Salonica',lat => 40.6402778,lon => 22.9438889}, 
		 { name => 'Salonika',lat => 40.6402778,lon => 22.9438889}, 
		 { name => 'Salon_2ki',lat => 40.6402778,lon => 22.9438889}, 
		 { name => 'Salonique',lat => 40.6402778,lon => 22.9438889}, 
		 { name => 'Selanik',lat => 40.6402778,lon => 22.9438889}, 
		 { name => 'Solun',lat => 40.6402778,lon => 22.9438889}, 
		 { name => 'Thessalon_2k_2',lat => 40.6402778,lon => 22.9438889}, 
		 { name => 'Thessalon_2ki',lat => 40.6402778,lon => 22.9438889}, 
		 { name => 'Agram',lat => 45.8,lon => 16.0}, 
		 { name => 'Andautonia',lat => 45.8,lon => 16.0}, 
		 { name => 'Fiume',lat => 45.3430556,lon => 14.4091667}, 
		 { name => 'Fkumen',lat => 45.3430556,lon => 14.4091667}, 
		 { name => 'Flumen Sancti Viti',lat => 45.3430556,lon => 14.4091667}, 
		 { name => 'R_2ka',lat => 45.3430556,lon => 14.4091667}, 
		 { name => 'Rieka',lat => 45.3430556,lon => 14.4091667}, 
		 { name => 'Rijeka',lat => 45.3430556,lon => 14.4091667}, 
		 { name => 'Sankt Veit am Flaum',lat => 45.3430556,lon => 14.4091667}, 
		 { name => 'Sankt Veit am Pflaum',lat => 45.3430556,lon => 14.4091667}, 
		 { name => 'Vitipolis',lat => 45.3430556,lon => 14.4091667}, 
		 { name => 'Zabrag',lat => 45.8,lon => 16.0}, 
		 { name => 'Zabreg',lat => 45.8,lon => 16.0}, 
		 { name => 'Zagabria',lat => 45.8,lon => 16.0}, 
		 { name => 'Z_2gr_2b',lat => 45.8,lon => 16.0}, 
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
		 { name => 'Ghar_2b_2-e K_2chek',lat => 30.2075,lon => 49.685}, 
		 { name => 'Ghar_2b_2-e yakom',lat => 30.2075,lon => 49.685}, 
		 { name => 'Ghar_2bi K_2chik',lat => 30.2075,lon => 49.685}, 
		 { name => 'Ghar_2b_2-ye K_2chek',lat => 30.2075,lon => 49.685}, 
		 { name => 'Ghar_2b_2-ye `Oly_2',lat => 30.2075,lon => 49.685}, 
		 { name => '_2oseyn_2b_2d',lat => 34.8558333,lon => 50.8583333}, 
		 { name => 'Na`_2m_2b_2d',lat => 34.45,lon => 50.8683333}, 
		 { name => 'Kavagbouma',lat => 7.8533333,lon => -6.1066667}, 
		 { name => 'Kavagouma',lat => 7.8533333,lon => -6.1066667}, 
		 { name => 'Edo',lat => 35.685,lon =>  139.7513889}, 
		 { name => 'Tokio',lat => 35.685,lon =>  139.7513889}, 
		 { name => 'Tokyo',lat => 35.685,lon =>  139.7513889}, 
		 { name => 'T_2ky_2',lat => 35.685,lon =>  139.7513889}, 
		 { name => 'Heij_2',lat => 39.0194444,lon =>  125.7547222}, 
		 { name => 'Heij_2-fu',lat => 39.0194444,lon =>  125.7547222}, 
		 { name => 'Heizy_2',lat => 39.0194444,lon =>  125.7547222}, 
		 { name => 'Heizy_2 Hu',lat => 39.0194444,lon =>  125.7547222}, 
		 { name => 'Hpyeng-yang',lat => 39.0194444,lon =>  125.7547222}, 
		 { name => 'P-hj_2ng-jang',lat => 39.0194444,lon =>  125.7547222}, 
		 { name => 'Phyeng-yang',lat => 39.0194444,lon =>  125.7547222}, 
		 { name => 'Phyong-yang',lat => 39.0194444,lon =>  125.7547222}, 
		 { name => 'Pienyang',lat => 39.0194444,lon =>  125.7547222}, 
		 { name => 'Pingyang',lat => 39.0194444,lon =>  125.7547222}, 
		 { name => 'Pyengyang',lat => 39.0194444,lon =>  125.7547222}, 
		 { name => 'Py_2ngyang',lat => 39.0194444,lon =>  125.7547222}, 
		 { name => 'Pyongyang',lat => 39.0194444,lon =>  125.7547222}, 
		 { name => 'Choei-yuen',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'Hang-yang-tcheng',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'Hans_2ng',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'H_2n-yang',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'H_2-seng',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'Kan-y_2-j_2',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'Keijo',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'Keizy_2e',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'Kiung',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'Ky_2ngs_2ng',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'Seoul',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'Seul',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'S_2ul',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'Suigen',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'Sye-ul',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'Syou-ouen',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'Wang-ching',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'W_2-j_2',lat => 37.5663889,lon =>  126.9997222}, 
		 { name => 'R_2ga',lat => 56.95,lon => 24.1}, 
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
		 { name => 'Panev_2_2io',lat => 55.7333333,lon => 24.35}, 
		 { name => 'Panev_2_2ys',lat => 55.7333333,lon => 24.35}, 
		 { name => 'Poniewesch',lat => 55.7333333,lon => 24.35}, 
		 { name => 'Poniewiesh',lat => 55.7333333,lon => 24.35}, 
		 { name => 'Poniewie_2',lat => 55.7333333,lon => 24.35}, 
		 { name => 'Schaulen',lat => 55.9333333,lon => 23.3166667}, 
		 { name => 'Shaulyay',lat => 55.9333333,lon => 23.3166667}, 
		 { name => 'Shavli',lat => 55.9333333,lon => 23.3166667}, 
		 { name => 'Shyaulyay',lat => 55.9333333,lon => 23.3166667}, 
		 { name => '_2iauliai',lat => 55.9333333,lon => 23.3166667}, 
		 { name => '_2iauli_2',lat => 55.9333333,lon => 23.3166667}, 
		 { name => 'Sokniai',lat => 55.9333333,lon => 23.3166667}, 
		 { name => 'Szawle',lat => 55.9333333,lon => 23.3166667}, 
		 { name => 'Vilna',lat => 54.6833333,lon => 25.3166667}, 
		 { name => 'Vilnia',lat => 54.6833333,lon => 25.3166667}, 
		 { name => 'Vilnius',lat => 54.6833333,lon => 25.3166667}, 
		 { name => 'Vilno',lat => 54.6833333,lon => 25.3166667}, 
		 { name => 'Vilnyus',lat => 54.6833333,lon => 25.3166667}, 
		 { name => 'Wilna',lat => 54.6833333,lon => 25.3166667}, 
		 { name => 'Wilno',lat => 54.6833333,lon => 25.3166667}, 
		 { name => 'B_2l_2i',lat => 47.7616667,lon => 27.9288889}, 
		 { name => 'Baltsy',lat => 47.7616667,lon => 27.9288889}, 
		 { name => 'Beltsi',lat => 47.7616667,lon => 27.9288889}, 
		 { name => 'Beltsy',lat => 47.7616667,lon => 27.9288889}, 
		 { name => 'Bendary',lat => 46.8305556,lon => 29.4711111}, 
		 { name => 'Bender',lat => 46.8305556,lon => 29.4711111}, 
		 { name => 'Bendery',lat => 46.8305556,lon => 29.4711111}, 
		 { name => 'Byelcy',lat => 47.7616667,lon => 27.9288889}, 
		 { name => 'Chi_2in_2u',lat => 47.0055556,lon => 28.8575}, 
		 { name => 'Kischinew',lat => 47.0055556,lon => 28.8575}, 
		 { name => 'Kiscinev',lat => 47.0055556,lon => 28.8575}, 
		 { name => 'Kishinef',lat => 47.0055556,lon => 28.8575}, 
		 { name => 'Kishin_2v',lat => 47.0055556,lon => 28.8575}, 
		 { name => 'Kishiniv',lat => 47.0055556,lon => 28.8575}, 
		 { name => 'Tighina',lat => 46.8305556,lon => 29.4711111}, 
		 { name => 'Tigina',lat => 46.8305556,lon => 29.4711111}, 
		 { name => 'Tiraspol',lat => 46.8402778,lon => 29.6433333}, 
		 { name => 'Tiraspol',lat => 46.8402778,lon => 29.6433333}, 
		 { name => 'Al Qunay_2irah',lat => 34.2608333,lon => -6.5794444}, 
		 { name => 'Kenitra',lat => 34.2608333,lon => -6.5794444}, 
		 { name => 'Khenifra',lat => 34.2608333,lon => -6.5794444}, 
		 { name => 'Lyautey',lat => 34.2608333,lon => -6.5794444}, 
		 { name => 'Mina Hassan Tani',lat => 34.2608333,lon => -6.5794444}, 
		 { name => 'Port Laoti',lat => 34.2608333,lon => -6.5794444}, 
		 { name => 'Port-Lyautey',lat => 34.2608333,lon => -6.5794444}, 
		 { name => 'Rabat',lat => 34.0252778,lon => -6.8361111}, 
		 { name => 'Sal_2',lat => 34.0394444,lon => -6.8027778}, 
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
		 { name => 'G_2teborg',lat => 57.7166667,lon => 11.9666667}, 
		 { name => 'Goteburg',lat => 57.7166667,lon => 11.9666667}, 
		 { name => 'Gothenburg',lat => 57.7166667,lon => 11.9666667}, 
		 { name => 'Gottenborg',lat => 57.7166667,lon => 11.9666667}, 
		 { name => 'Malm_2',lat => 55.6,lon => 13.0}, 
		 { name => 'Stockholm',lat => 59.3333333,lon => 18.05}, 
		 { name => 'Tukholma',lat => 59.3333333,lon => 18.05}, 
		 { name => 'Uppsala',lat => 59.85,lon => 17.6333333}, 
		 { name => 'V_2ster_2s',lat => 59.6166667,lon => 16.55}, 
		 { name => 'B_2le',lat => 47.5666667,lon => 7.6}, 
		 { name => 'Basel',lat => 47.5666667,lon => 7.6}, 
		 { name => 'Basilea',lat => 47.5666667,lon => 7.6}, 
		 { name => 'Basle',lat => 47.5666667,lon => 7.6}, 
		 { name => 'Bern',lat => 46.9166667,lon => 7.4666667}, 
		 { name => 'Berna',lat => 46.9166667,lon => 7.4666667}, 
		 { name => 'Berne',lat => 46.9166667,lon => 7.4666667}, 
		 { name => 'Geneva',lat => 46.2,lon => 6.1666667}, 
		 { name => 'Gen_2ve',lat => 46.2,lon => 6.1666667}, 
		 { name => 'Genf',lat => 46.2,lon => 6.1666667}, 
		 { name => 'Ginevra',lat => 46.2,lon => 6.1666667}, 
		 { name => 'Lausanne',lat => 46.5333333,lon => 6.6666667}, 
		 { name => 'Z_2rich',lat => 47.3666667,lon => 8.55}, 
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
		 { name => '_2zmit',lat => 40.7669444,lon => 29.9169444}, 
		 { name => 'Kadi Kioi',lat => 40.9855556,lon => 29.0294444}, 
		 { name => 'Kadik_2i',lat => 40.9855556,lon => 29.0294444}, 
		 { name => 'Kad_2k_2y',lat => 40.9855556,lon => 29.0294444}, 
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
		 { name => 'Ili_2i',lat => 44.0477778,lon => 19.7783333}, 
		 { name => 'Joki_2i',lat => 44.0491667,lon => 19.7847222}, 
		 { name => 'Maria-Theresianopel',lat => 46.1,lon => 19.6666667}, 
		 { name => 'Maria-Theresiopel',lat => 46.1,lon => 19.6666667}, 
		 { name => 'Maria-Theresiopolis',lat => 46.1,lon => 19.6666667}, 
		 { name => 'Nich',lat => 43.3247222,lon => 21.9033333}, 
		 { name => 'Ni_2',lat => 43.3247222,lon => 21.9033333}, 
		 { name => 'Nisch',lat => 43.3247222,lon => 21.9033333}, 
		 { name => 'Nish',lat => 43.3247222,lon => 21.9033333}, 
		 { name => 'Nissa',lat => 43.3247222,lon => 21.9033333}, 
		 { name => 'Podgorica',lat => 42.4411111,lon => 19.2636111}, 
		 { name => 'Prishtin_2',lat => 42.6666667,lon => 21.1666667}, 
		 { name => 'Pri_2tina',lat => 42.6666667,lon => 21.1666667}, 
		 { name => 'Sawaditz',lat => 46.1,lon => 19.6666667}, 
		 { name => 'Slatina',lat => 44.6491667,lon => 19.6469444}, 
		 { name => 'Subotica',lat => 46.1,lon => 19.6666667}, 
		 { name => 'Szabadka',lat => 46.1,lon => 19.6666667}, 
		 { name => 'Szent-M_2ria',lat => 46.1,lon => 19.6666667}, 
		 { name => 'Theresiopel',lat => 46.1,lon => 19.6666667}, 
		 { name => 'Titograd',lat => 42.4411111,lon => 19.2636111}, 
		 { name => 'Zubotica',lat => 46.1,lon => 19.6666667}, 
		     ) {
	    $loc->{"$t.name"} = $loc->{name};
	    $loc->{"$t.lat"}  = $loc->{lat};
	    $loc->{"$t.lon"}  = $loc->{lon};

	    #my $delete_query=sprintf("DELETE FROM $t ".
	    #"WHERE name = '%s' AND $type_query",
	    #$loc->{"$t.name"});
	    #Geo::Gpsdrive::DBFuncs::db_exec( $delete_query);
	    Geo::Gpsdrive::DBFuncs::insert_hash($t, $wp_defaults, $loc );
	}
    }
} # of fill_example_cities()


# -----------------------------------------------------------------------------
sub fill_example_streets() { # Insert Street Sample
    my $street_samples = 
	[
	 [ 'ST2062',
	   [ 48.112336, 11.510902],
	   [ 48.112336, 11.510902],
	   [ 48.070705, 11.494056],
	   [ 48.070705, 11.494056],
	   [ 48.017610, 11.420729],
	   [ 47.583639, 11.161964],
	   ],
	 [ 'A99',
	   [ 48.095682, 11.586455],
	   [ 48.077125, 11.598074],
	   [ 48.077125, 11.598074],
	   [ 48.056545, 11.594481],
	   [ 48.040064, 11.599710],
	   [ 48.040064, 11.599710],
	   [ 48.027972, 11.625048],
	   [ 48.027972, 11.625048],
	   [ 48.027240, 11.658669],
	   [ 48.027240, 11.658669],
	   [ 48.036089, 11.675191],
	   [ 48.053050, 11.691906],
	   [ 48.053050, 11.691906],
	   [ 48.075921, 11.731036],
	   [ 48.075921, 11.731036],
	   [ 48.091657, 11.745508],
	   [ 48.091657, 11.745508],
	   [ 48.102847, 11.752759],
	   [ 48.119793, 11.758636],
	   [ 48.130990, 11.759533],
	   [ 48.143239, 11.758395],
	   [ 48.151710, 11.753862],
	   [ 48.162556, 11.744781],
	   [ 48.177074, 11.730703],
	   [ 48.185242, 11.717988],
	   [ 48.192932, 11.694612],
	   [ 48.198809, 11.671218],
	   [ 48.206055, 11.653498],
	   [ 48.217746, 11.634896],
	   [ 48.221377, 11.628536],
	   [ 48.221377, 11.628536],
	   [ 48.228468, 11.601503],
	   [ 48.228304, 11.585829],
	   [ 48.226757, 11.548598],
	   [ 48.224919, 11.531336],
	   [ 48.218173, 11.507746],
	   [ 48.209317, 11.487908],
	   [ 48.209302, 11.479392],
	   [ 48.209302, 11.479392],
	   [ 48.204214, 11.452935],
	   [ 48.204191, 11.442531],
	   [ 48.206590, 11.393346],
	   ],
	 [ 'A8',
	   [ 48.166920, 11.455637],
	   [ 48.254054, 11.308324],
	   [ 48.275308, 11.284280],
	   [ 48.275308, 11.284280],
	   [ 48.288566, 11.255891],
	   [ 48.298424, 11.220360],
	   [ 48.298424, 11.220360],
	   [ 48.340641, 11.155905],
	   [ 48.347567, 11.138835],
	   [ 48.362685, 11.125538],
	   [ 48.368658, 11.105128],
	   [ 48.395090, 11.065203],
	   [ 48.395090, 11.065203],
	   [ 48.396931, 11.033393],
	   [ 48.400053, 11.018198],
	   [ 48.403737, 10.967858],
	   [ 48.403737, 10.967858],
	   [ 48.420368, 10.680812],
	   [ 48.398929, 10.552328],
	   [ 48.411761, 10.421588],
	   [ 48.439212, 10.325261],
	   [ 48.424988, 10.228094],
	   [ 48.424988, 10.228094],
	   [ 48.463738, 10.114712],
	   [ 48.463738, 10.114712],
	   [ 48.457313,  9.949232],
	   [ 48.518591,  9.807234],
	   [ 48.545679,  9.647524],
	   [ 48.579587,  9.687756],
	   [ 48.640469,  9.556543],
	   [ 48.625523,  9.447761],
	   [ 48.670966,  9.361957],
	   [ 48.701206,  9.258845],
	   [ 48.693575,  9.201564],
	   [ 48.716204,  9.155636],
	   [ 48.731064,  9.040913],
	   [ 48.731064,  9.040913],
	   [ 48.854843,  8.793033],
	   [ 48.854843,  8.793033],
	   [ 48.907174,  8.688887],
	   [ 48.907174,  8.688887],
	   [ 48.906708,  8.619849],
	   [ 48.971429,  8.437445],
	   ],
	 [ 'A96',
	   [ 47.550013, 9.745248],
	   [ 47.646443, 9.748812],
	   [ 47.708577, 9.813953],
	   [ 47.813442, 10.000801],
	   [ 47.997584, 10.155805],
	   [ 48.026604, 10.481288],
	   [ 48.044739, 10.829448],
	   [ 48.067698, 10.863030],
	   [ 48.056619, 10.897151],
	   [ 48.056619, 10.897151],
	   [ 48.085376, 11.315663],
	   [ 48.131055, 11.417356],
	   [ 48.123690, 11.519369],
	   ],
	 [ 'A95',
	   [ 48.118028, 11.522214],
	   [ 48.025180, 11.423516],
	   [ 47.987444, 11.429310],
	   [ 47.881473, 11.407236],
	   [ 47.881473, 11.407236],
	   [ 47.786786, 11.362659],
	   [ 47.786786, 11.362659],
	   [ 47.760209, 11.329054],
	   [ 47.707265, 11.335025],
	   [ 47.657726, 11.234258],
	   [ 47.608302, 11.178550],
	   [ 47.581831, 11.178797],
	   ],
	 [ 'A8',
	   [ 48.112087, 11.615228],
	   [ 48.100107, 11.613355],
	   [ 48.079946, 11.619029],
	   [ 48.059780, 11.636972],
	   [ 48.027002, 11.657727],
	   [ 48.027002, 11.657727],
	   [ 47.960210, 11.690060],
	   [ 47.960210, 11.690060],
	   [ 47.896058, 11.724493],
	   [ 47.896058, 11.724493],
	   [ 47.861879, 11.846608],
	   [ 47.841026, 11.871892],
	   [ 47.827736, 11.895327],
	   [ 47.830225, 11.912234],
	   [ 47.821169, 11.930062],
	   [ 47.824380, 11.959631],
	   [ 47.820613, 11.971848],
	   [ 47.820613, 11.971848],
	   [ 47.816926, 12.046969],
	   [ 47.808760, 12.095788],
	   [ 47.801502, 12.189173],
	   [ 47.795188, 12.202302],
	   [ 47.797685, 12.229515],
	   [ 47.798284, 12.258614],
	   [ 47.803304, 12.278339],
	   [ 47.803271, 12.299927],
	   [ 47.803271, 12.299927],
	   [ 47.816995, 12.374143],
	   [ 47.816995, 12.374143],
	   [ 47.832709, 12.392989],
	   [ 47.834854, 12.441319],
	   [ 47.834854, 12.441319],
	   [ 47.840162, 12.487830],
	   [ 47.830972, 12.524898],
	   [ 47.830972, 12.524898],
	   [ 47.829219, 12.615509],
	   [ 47.824759, 12.678403],
	   [ 47.824759, 12.678403],
	   [ 47.831314, 12.739916],
	   [ 47.826245, 12.755872],
	   [ 47.829327, 12.799079],
	   [ 47.828666, 12.815035],
	   [ 47.768419, 12.907604],
	   [ 47.771502, 12.963957],
	   ],
	 [ 'A93',
	   [ 47.605334, 12.194570],
	   [ 47.615106, 12.203468],
	   [ 47.625190, 12.202548],
	   [ 47.664916, 12.182005],
	   [ 47.681320, 12.167510],
	   [ 47.698970, 12.160959],
	   [ 47.759490, 12.108485],
	   [ 47.798980, 12.089689],
	   [ 47.809075, 12.096257],
	   ],
	 [ 'A9',
	   [ 48.175256, 11.592803],
	   [ 48.175256, 11.592803],
	   [ 48.184101, 11.600317],
	   [ 48.186267, 11.610635],
	   [ 48.221071, 11.628752],
	   [ 48.257222, 11.646232],
	   [ 48.262969, 11.647135],
	   [ 48.282034, 11.640762],
	   [ 48.315747, 11.621650],
	   [ 48.361743, 11.595999],
	   [ 48.391054, 11.595958],
	   [ 48.403029, 11.590252],
	   [ 48.459122, 11.591117],
	   [ 48.459122, 11.591117],
	   [ 48.476758, 11.586880],
	   [ 48.476758, 11.586880],
	   [ 48.494388, 11.596446],
	   [ 48.514895, 11.583669],
	   [ 48.525602, 11.586552],
	   [ 48.533780, 11.596098],
	   [ 48.556480, 11.590464],
	   [ 48.579806, 11.592440],
	   [ 48.585565, 11.576252],
	   [ 48.624709, 11.526781],
	   [ 48.631644, 11.525842],
	   [ 48.651817, 11.514429],
	   [ 48.670736, 11.507773],
	   [ 48.709197, 11.485860],
	   [ 48.759648, 11.463010],
	   [ 48.926079, 11.479607],
	   [ 49.001415, 11.358315],
	   [ 49.027913, 11.369684],
	   [ 49.076820, 11.294345],
	   [ 49.176888, 11.258888],
	   [ 49.237209, 11.217846],
	   [ 49.365662, 11.193450],
	   [ 49.411032, 11.193013],
	   [ 49.532559, 11.320022],
	   [ 49.570390, 11.325577],
	   [ 49.612135, 11.371968],
	   [ 49.657753, 11.470995],
	   [ 49.714474, 11.470767],
	   [ 49.741019, 11.517467],
	   [ 49.835513, 11.493726],
	   [ 49.866736, 11.525882],
	   [ 49.885721, 11.596248],
	   [ 50.029486, 11.607820],
	   [ 50.067302, 11.631332],
	   [ 50.127766, 11.743325],
	   [ 50.199566, 11.778876],
	   [ 50.237370, 11.778965],
	   [ 50.286465, 11.820505],
	   [ 50.358347, 11.785184],
	   [ 50.375338, 11.800042],
	   [ 50.398057, 11.770460],
	   [ 50.432032, 11.806144],
	   [ 50.484959, 11.806300],
	   [ 50.500097, 11.794458],
	   [ 50.534101, 11.812386],
	   [ 50.575737, 11.776797],
	   [ 50.605959, 11.794745],
	   [ 50.643747, 11.806771],
	   [ 50.711703, 11.860699],
	   [ 50.730633, 11.848832],
	   [ 50.777798, 11.890874],
	   [ 50.815598, 11.897031],
	   [ 50.881860, 11.849423],
	   [ 50.949930, 11.837689],
	   [ 50.972579, 11.855790],
	   [ 50.987564, 11.915908],
	   [ 51.025195, 11.940105],
	   [ 51.119640, 11.958736],
	   [ 51.234118, 12.053035],
	   [ 51.350595, 12.199266],
	   [ 51.426184, 12.200146],
	   [ 51.478967, 12.225040],
	   [ 51.501850, 12.188887],
	   [ 51.543318, 12.207603],
	   [ 51.584965, 12.195932],
	   [ 51.758582, 12.240704],
	   [ 51.960939, 12.479466],
	   [ 52.229996, 12.925428],
	   [ 52.286772, 12.917784],
	   ],
	 [ 'A2',
	   [ 52.314655,   13.520880 ],
	   [ 52.307528,   13.289167 ],
	   [ 52.292588,   12.912473 ],
	   [ 52.336605,   12.803149 ],
	   [ 52.337094,   12.659419 ],
	   [ 52.351516,   12.539090 ],
	   [ 52.282321,   12.457413 ],
	   [ 52.268141,   12.336735 ],
	   [ 52.247571,   12.305554 ],
	   [ 52.253841,   12.209951 ],
	   [ 52.229992,   12.067738 ],
	   [ 52.234343,   11.894996 ],
	   [ 52.232687,   11.759221 ],
	   [ 52.223286,   11.679008 ],
	   [ 52.162704,   11.524978 ],
	   [ 52.189009,   11.444749 ],
	   [ 52.186838,   11.346133 ],
	   [ 52.219372,   11.049786 ],
	   [ 52.260450,   10.978206 ],
	   [ 52.313960,   10.819718 ],
	   [ 52.307379,   10.724035 ],
	   [ 52.316405,   10.516735 ],
	   [ 52.339202,   10.380064 ],
	   [ 52.340703,   10.341620 ],
	   [ 52.334334,   10.223958 ],
	   [ 52.355432,   10.190347 ],
	   [ 52.367696,   10.082267 ],
	   [ 52.389239,    9.980462 ],
	   [ 52.399185,    9.903174 ],
	   [ 52.412718,    9.850689 ],
	   [ 52.431821,    9.810613 ],
	   [ 52.430505,    9.680444 ],
	   [ 52.421457,    9.547120 ],
	   [ 52.242164,    9.280873 ],
	   [ 52.217490,    9.179127 ],
	   [ 52.224941,    9.123568 ],
	   [ 52.218968,    9.015642 ],
	   [ 52.210686,    8.846091 ],
	   [ 52.149992,    8.803635 ],
	   [ 52.140192,    8.745250 ],
	   [ 52.111781,    8.733317 ],
	   [ 52.071666,    8.669288 ],
	   [ 51.995747,    8.618124 ],
	   [ 51.974959,    8.618456 ],
	   [ 51.913624,    8.512196 ],
	   [ 51.827704,    8.238538 ],
	   [ 51.832944,    8.201710 ],
	   [ 51.769934,    8.004766 ],
	   [ 51.768205,    8.007838 ],
	   [ 51.704125,    7.991166 ],
	   [ 51.623985,    7.876767 ],
	   [ 51.597209,    7.684573 ],
	   [ 51.572007,    7.477408 ],
	   [ 51.583835,    7.368037 ],
	   [ 51.608566,    7.322586 ],
	   [ 51.554278,    6.927051 ],
	   ],
	 [ 'A3',
	   [ 50.056007, 8.729759],
	   [ 50.049447, 8.924135],
	   [ 49.985470, 9.024586],
	   [ 49.997211, 9.206862],
	   [ 49.766340, 9.576275],
	   [ 49.734074, 10.009036],
	   [ 49.774126, 10.094502],
	   [ 49.779411, 10.434223],
	   [ 49.734812, 10.810574],
	   [ 49.562589, 10.945266],
	   [ 49.559223, 10.990461],
	   [ 49.460449, 11.250709],
	   [ 49.012474, 12.055815],
	   [ 48.940570, 12.628217],
	   [ 48.602797, 13.365175],
	   [ 48.413993, 13.413118],
	   ],
	 [ 'A5',
	   [ 47.475311, 7.604408],
	   [ 47.475311, 7.604408],
	   [ 47.598210, 7.608628],
	   [ 47.659213, 7.519473],
	   [ 47.700793, 7.525514],
	   [ 47.738407, 7.565270],
	   [ 47.780156, 7.537573],
	   [ 47.780156, 7.537573],
	   [ 47.971969, 7.717537],
	   [ 48.058113, 7.820472],
	   [ 48.187236, 7.748788],
	   [ 48.187236, 7.748788],
	   [ 48.285255, 7.790014],
	   [ 48.359645, 7.794010],
	   [ 48.445658, 7.909378],
	   [ 48.445658, 7.909378],
	   [ 48.500123, 7.900721],
	   [ 48.786956, 8.150867],
	   [ 48.977153, 8.443097],
	   [ 49.148228, 8.555809],
	   [ 49.275423, 8.626247],
	   [ 49.275423, 8.626247],
	   [ 49.543931, 8.634024],
	   [ 49.755405, 8.595863],
	   [ 49.837930, 8.638579],
	   [ 50.525011, 8.768384],
	   [ 50.606629, 8.829962],
	   [ 50.648937, 8.996452],
	   [ 50.721023, 9.079534],
	   [ 50.711782, 9.184081],
	   [ 50.808116, 9.527759],
	   ],
	 [ 'A45',
	   [ 51.433743,    7.533565],
	   [ 51.403370,    7.557446],
	   [ 51.403370,    7.557446],
	   [ 51.327968,    7.523256],
	   [ 51.289897,    7.571189],
	   [ 51.224979,    7.672979],
	   [ 51.190919,    7.678520],
	   [ 51.172194,    7.654130],
	   [ 51.172194,    7.654130],
	   [ 51.050732,    7.724567],
	   [ 51.031448,    7.832450],
	   [ 50.965118,    7.847591],
	   [ 50.836911,    7.998350],
	   [ 50.815513,    8.094697],
	   [ 50.815513,    8.094697],
	   [ 50.777794,    8.101682],
	   [ 50.740131,    8.272925],
	   [ 50.672171,    8.280433],
	   [ 50.657436,    8.316543],
	   [ 50.623603,    8.335159],
	   [ 50.623603,    8.335159],
	   [ 50.580066,    8.532556],
	   [ 50.527611,    8.592915],
	   [ 50.524186,    8.640541],
	   [ 50.505608,    8.688366],
	   [ 50.479230,    8.700608],
	   [ 50.471859,    8.730413],
	   [ 50.471859,    8.730413],
	   [ 50.415599,    8.808264],
	   [ 50.419675,    8.867552],
	   [ 50.382195,    8.939064],
	   [ 50.329197,    8.921731],
	   [ 50.166990,    9.017574],
	   [ 50.129057,    8.982441],
	   [ 50.083903,    9.047573],
	   [ 50.083903,    9.047573],
	   [ 49.989209,    9.012800],
	   ],
	 [ 'A94',
	   [ 48.136899,   11.624203 ],
	   [ 48.138266,   11.667645 ],
	   [ 48.138266,   11.667645 ],
	   [ 48.136993,  11.679736 ],
	   [ 48.141538,   11.692423 ],
	   [ 48.141848,   11.697409 ],
	   [ 48.141848,   11.697409 ],
	   [ 48.141865,   11.720537 ],
	   [ 48.142942,   11.757951 ],
	   [ 48.142942,   11.757951 ],
	   [ 48.142945,   11.772911 ],
	   [ 48.148998,   11.786498 ],
	   [ 48.150507,   11.804642 ],
	   [ 48.159089,   11.832541 ],
	   [ 48.161782,   11.856125 ],
	   [ 48.164042,   11.874493 ],
	   [ 48.169773,   11.890847 ],
	   [ 48.175204,   11.900380 ],
	   ],
	 [ 'ST2082',
	   [ 48.139421,   11.712965 ],
	   [ 48.140931,   11.710699 ],
	   [ 48.141686,   11.709337 ],
	   [ 48.143052,   11.707974 ],
	   [ 48.144715,   11.707514 ],
	   [ 48.147431,   11.706838 ],
	   [ 48.150004,   11.707293 ],
	   [ 48.152424,   11.709326 ],
	   [ 48.157196,   11.721348 ],
	   [ 48.157952,   11.722695 ],
	   [ 48.158564,   11.724056 ],
	   [ 48.160220,   11.729502 ],
	   [ 48.160823,   11.731093 ],
	   [ 48.162335,   11.732899 ],
	   [ 48.165662,   11.733355 ],
	   [ 48.166674,   11.734649 ],
	   [ 48.167735,   11.735553 ],
	   [ 48.168337,   11.737144 ],
	   [ 48.168939,   11.738951 ],
	   [ 48.169694,   11.741905 ],
	   [ 48.170296,   11.743942 ],
	   [ 48.170324,   11.744731 ],
	   [ 48.170396,   11.744879 ],
	   [ 48.170576,   11.745526 ],
	   [ 48.170675,   11.745715 ],
	   [ 48.171593,   11.750893 ],
	   [ 48.171800,   11.751689 ],
	   [ 48.172213,   11.754885 ],
	   [ 48.173202,   11.763004 ],
	   [ 48.173589,   11.764393 ],
	   [ 48.174200,   11.765081 ],
	   [ 48.174677,   11.765189 ],
	   [ 48.175226,   11.765081 ],
	   [ 48.175711,   11.764744 ],
	   [ 48.179720,   11.762397 ],
	   [ 48.180628,   11.762519 ],
	   [ 48.180979,   11.762816 ],
	   [ 48.181815,   11.764475 ],
	   [ 48.182598,   11.766687 ],
	   [ 48.185434,   11.774472 ],
	   [ 48.185964,   11.775376 ],
	   [ 48.186288,   11.775565 ],
	   [ 48.187550,   11.774933 ],
	   [ 48.187775,   11.774974 ],
	   [ 48.188027,   11.775419 ],
	   [ 48.188486,   11.779196 ],
	   [ 48.189637,   11.780289 ],
	   [ 48.193261,   11.786873 ],
	   [ 48.197901,   11.796715 ],
	   [ 48.198279,   11.798415 ],
	   [ 48.198809,   11.799211 ],
	   [ 48.199645,   11.799549 ],
	   [ 48.208363,   11.798647 ],
	   [ 48.216188,   11.799608 ],
	   [ 48.219776,   11.800351 ],
	   [ 48.220684,   11.801148 ],
	   [ 48.224730,   11.807386 ],
	   [ 48.226465,   11.808413 ],
	   [ 48.226924,   11.809318 ],
	   [ 48.226843,   11.810344 ],
	   [ 48.228209,   11.812842 ],
	   [ 48.229361,   11.813984 ],
	   [ 48.234910,   11.815862 ],
	   [ 48.244406,   11.823696 ],
	   [ 48.245269,   11.825574 ],
	   [ 48.251410,   11.843574 ],
	   [ 48.251482,   11.844587 ],
	   [ 48.253334,   11.849018 ],
	   [ 48.254844,   11.850275 ],
	   [ 48.256696,   11.852883 ],
	   [ 48.257155,   11.854247 ],
	   [ 48.257532,   11.855720 ],
	   [ 48.257154,   11.857881 ],
	   [ 48.257072,   11.860380 ],
	   [ 48.257907,   11.864473 ],
	   [ 48.258631,   11.868436 ],
	   [ 48.261734,   11.871841 ],
	   [ 48.263932,   11.880029 ],
	   [ 48.264759,   11.882191 ],
	   [ 48.266575,   11.884232 ],
	   [ 48.269075,   11.887071 ],
	   [ 48.270968,   11.890016 ],
	   [ 48.272704,   11.890814 ],
	   [ 48.275515,   11.894680 ],
	   [ 48.276945,   11.895694 ],
	   [ 48.282736,   11.897912 ],
	   [ 48.290830,   11.899173 ],
	   [ 48.292985,   11.899971 ],
	   ],
	 [ 'Muenchner Strasse',
	   [ 48.158699,   11.698494 ],
	   [ 48.161504,   11.702013 ],
	   [ 48.166877,   11.712061 ],
	   [ 48.168083,   11.713881 ],
	   [ 48.170655,   11.714554 ],
	   [ 48.170961,   11.715134 ],
	   [ 48.172248,   11.723629 ],
	   [ 48.172024,   11.723980 ],
	   [ 48.172320,   11.724317 ],
	   [ 48.173535,   11.734175 ],
	   [ 48.174363,   11.741768 ],
	   [ 48.175204,   11.747525 ],
	   [ 48.176338,   11.751948 ],
	   [ 48.176563,   11.755360 ],
	   [ 48.176869,   11.756156 ],
	   [ 48.178083,   11.756156 ],
	   [ 48.180655,   11.762495 ],
	   ],
	 [ 'Hans-Dasch-Weg',
	   [ 48.175469, 11.754615 ],
	   [ 48.174992, 11.753199 ],
	   ],
	 [ 'Garten-Strasse',
	   [ 48.174965,   11.755384 ],
	   [ 48.173931,   11.752350 ],
	   ],
	 [ 'Am Brunnen',
	   [ 48.175802,   11.749854 ],
	   [ 48.175226,   11.750421 ],
	   [ 48.174722,   11.750704 ],
	   [ 48.174525,   11.751068 ],
	   [ 48.174525,   11.751419 ],
	   [ 48.174408,   11.751837 ],
	   [ 48.174237,   11.752080 ],
	   [ 48.173751,   11.752633 ],
	   [ 48.173400,   11.752781 ],
	   [ 48.173122,   11.753186 ],
	   [ 48.173149,   11.753388 ],
	   [ 48.172978,   11.753698 ],
	   [ 48.172978,   11.753847 ],
	   [ 48.172879,   11.753981 ],
	   [ 48.172735,   11.753941 ],
	   [ 48.172492,   11.753536 ],
	   [ 48.172204,   11.753442 ],
	   [ 48.171970,   11.753199 ],
	   ],
	 [ 'Heimstetter-Moosweg',
	   [ 48.174259,   11.741632 ],
	   [ 48.170410,   11.744640 ],
	   [ 48.162607,   11.751690 ],
	   [ 48.161132,   11.753052 ],
	   ],
	 [ 'Emmeran-Strasse',  
	   [ 48.174345,   11.753617 ],
	   [ 48.174974,   11.753186 ],
	   [ 48.175478,   11.752862 ],
	   [ 48.175757,   11.752579 ],
	   [ 48.176323,   11.752147 ],
	   ],
	 [ 'Ludwig-Strasse',
	   [ 48.175019,   11.746281 ],
	   [ 48.171368,   11.749423 ],
	   ],
	 [ 'Martin-Luther-Strasse',
	   [ 48.172807,   11.748075 ],
	   [ 48.173085,   11.749383 ],
	   [ 48.172618,   11.750408 ],
	   [ 48.173400,   11.752808 ],
	   [ 48.173391,   11.752862 ],
	   ],
	 [ 'Hess-Strasse',
	   [ 48.150036,   11.572727 ],
	   [ 48.153446,   11.560933 ],
	   [ 48.154166,   11.558156 ],
	   [ 48.155452,   11.555999 ],
	   [ 48.160552,   11.549717 ],
	   [ 48.160516,   11.549838 ],
	   ],
	 [ 'Goerres-Strasse',
	   [ 48.156774,   11.558615 ],
	   [ 48.155982,   11.561837 ],
	   [ 48.154695,   11.566609 ],
	   [ 48.153068,   11.565867 ],
	   ],	 
	 [ 'x-Strasse',
	   ],
	 ];


    for my $segments ( @{$street_samples} ) {	
	my $name = splice(@{$segments},0,1);
	my $multi_segment;
	#print Dumper($segments);
	
	$multi_segment->{'source_id'} = $example_source_id;
	$multi_segment->{'streets_type_id'} = '';
	if ( $name =~ m/^A/ ) {
	    $multi_segment->{'streets_type_id'} = streets_type_name2id('Strassen.Autobahn');
	} elsif ( $name =~ m/^ST/ ) {
	    $multi_segment->{'streets_type_id'} = streets_type_name2id('Strassen.Bundesstrasse');
	} elsif ( $name =~ m/^B/ ) {
	    $multi_segment->{'streets_type_id'} = streets_type_name2id('Strassen.Bundesstrasse');
	}   else {
	    $multi_segment->{'streets_type_id'} = streets_type_name2id('Strassen.Allgemein');
	}

	$multi_segment->{'name'}            = $name;
	$multi_segment->{'scale_min'}       = 1;
	$multi_segment->{'scale_max'}       = 50000000;

	$multi_segment->{'segments'} = $segments;

#	street_segments_add_from_segment_array($multi_segment);
	street_segments_add($multi_segment);
    }
}; # of fill_example_streets()

# ------------------------------------------------------------------
sub fill_examples(){

    $example_source_id = source_name2id($example_source_name);
    die "Unknown Source ID: $example_source_name\n" unless $example_source_id;

    delete_all_from_source($example_source_name);

    print "\nCreate Examples ...(source_id=$example_source_id)\n";

    fill_example_waypoints();
    fill_example_cities();
    fill_example_streets();
    print "Create Examples completed\n";
}


1;
