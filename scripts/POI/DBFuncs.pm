package POI::DBFuncs;

use strict;
use warnings;

use POSIX qw(strftime);
use Time::Local;
use DBI;
use POI::Utils;
use Data::Dumper;

$|= 1;                          # Autoflush

BEGIN {
        use Exporter   ();
        our ($VERSION, @ISA, @EXPORT, @EXPORT_OK, %EXPORT_TAGS);

        # set the version for version checking
        $VERSION     = 1.00;
        # if using RCS/CVS, this may be preferred
        $VERSION = sprintf "%d.%03d", q$Revision$ =~ /(\d+)/g;

        @ISA         = qw(Exporter);
        @EXPORT = qw( &type_names &type_list &db_disconnect &poi_add 
		      &delete_all_from_source &poi_list &column_names);
        %EXPORT_TAGS = ( );     # eg: TAG => [ qw!name1 name2! ],
        # your exported package globals go here,
        # as well as any optionally exported functions
        @EXPORT_OK   = qw();

}
our @EXPORT_OK;


END { } 

# -----------------------------------------------------------------------------

#retrieve Column Names for desired table
# TODO: get it directly from DB
sub column_names($){
    my $table = shift;
    my @col;
    @col = ("address_id","lat","lon","name",
	    "level_min","level_max","alt","comment",
	    "last_modified",
	    "proximity","type_id","url",
	    "wp_id","source_id") if $table eq "poi";

    @col = ("type_id","name","symbol","description") if $table eq "type";

    @col = ("source_id","name","url","licence","comment","last_update") if $table eq "source";

    return @col;
}

# -----------------------------------------------------------------------------

sub insert_hash {
    my ($table, $field_values) = @_;
#    my @fields = sort keys %$field_values; # sort required
    $field_values->{"$table.last_update"} ||= time();
    my @fields = map { "$table.$_" } column_names($table);
    my @values = @{$field_values}{@fields};
    my $sql = sprintf ( "insert into %s (%s) values (%s)",
			$table, 
			join(',', @fields),
			join(",", ("?") x @fields)
			);
    my $dbh = db_connect();
    #print "insert_hash($table, ".Dumper(\$field_values).")\n";
    #print "$sql\n";
    my $sth = $dbh->prepare_cached($sql);
    return $sth->execute(@values);
}


#############################################################################
# Alle noetigen Informationen fuer den connect mit der DB
# das sind: der host, der user und das passwort, die db ist immer die selbe
my $dbh;
sub db_connect() {
  my $db           = 'geoinfo';
  my $opt_user     = $main::db_user;
  my $opt_password = $main::db_password;
  my $host         = 'localhost';


  # First connect to Database
  unless ( $dbh ) {
      $dbh = DBI->connect(
			  "DBI:mysql:$db:$host",
			  $opt_user,$opt_password)
	  || die "Can't connect: $DBI::errstr\n";
  }

  return $dbh;
}

sub db_disconnect(){
    $dbh->disconnect()
	if $dbh;
}

# -----------------------------------------------------------------------------
# Delete all entries matching source with name
sub delete_all_from_source($){
    my $source_name = shift;
    debug("delete_all_from_source($source_name)");
    return unless $source_name;
    my $source_id = source_name2id( $source_name);
    return unless $source_id >0;

    my $query = "DELETE FROM poi WHERE poi.source_id = '$source_id'";
    my $sth=$dbh->prepare($query) or die $dbh->errstr;
    $sth->execute() or die $sth->errstr;
    $sth->finish;
}

# -----------------------------------------------------------------------------
# convert source name to source_id and cache it locally
my $source_id_cache;
sub source_name2id($){
    my $source_name = shift;
    my $source_id;
    if ( defined $source_id_cache->{$source_name} ) {
	$source_id = $source_id_cache->{$source_name};
    } else {
	my $dbh = db_connect();
	my $query = "SELECT source_id FROM source WHERE source.name = '$source_name' LIMIT 1";
	my $sth=$dbh->prepare($query) or die $dbh->errstr;
	$sth->execute() or die $sth->errstr;
	my $array_ref = $sth->fetchrow_arrayref();
	if ( $array_ref ) {
	    $source_id = $array_ref->[0];
	    $source_id_cache->{$source_name} = $source_id;
	} else {
	    # Nicht gefunden --> Neuen Eintrag anlegen
	    $source_id=0;
	}
	$sth->finish;
    }

    debug("Source: $source_name -> $source_id");

    return $source_id;
}

# -----------------------------------------------------------------------------
# get a list of all type names
sub type_names(){
    my @type_names;

    my $dbh = db_connect();
    
    my $query = "SELECT name FROM type";
    
    my $sth=$dbh->prepare($query) 
	or die $dbh->errstr;
    
    $sth->execute() 
	or die $sth->errstr;

    while (my $row = $sth->fetchrow_arrayref) {
	push(@type_names,$row->[0]);
    }
    $sth->finish;

    return @type_names;
}

# -----------------------------------------------------------------------------
# retrieve a complete list of known types
sub type_list(){
    my @type_list;

    my $dbh = db_connect();
    
    my @columns = column_names("type");
    my $query = "SELECT ".join(',', @columns)."  FROM type";
    
    my $sth=$dbh->prepare($query) 
	or die $dbh->errstr;
    
    $sth->execute() 
	or die $sth->errstr;

    while (my $row = $sth->fetchrow_arrayref) {
	my $type = {};
	for my $i ( 0.. $#columns) {
	    $type->{$columns[$i]} = $row->[$i];
	}
	push(@type_list,$type);
    }
    $sth->finish;

    return @type_list;
}

# -----------------------------------------------------------------------------
# retrieve first 100 entries from  poi Table
sub poi_list(){
    my @poi_list;

    my $dbh = db_connect();
    
    my @columns = column_names("poi");
    my $query = "SELECT ".join(',', @columns)."  FROM poi LIMIT 100";
    
    my $sth=$dbh->prepare($query) 
	or die $dbh->errstr;
    
    $sth->execute() 
	or die $sth->errstr;

    while (my $row = $sth->fetchrow_arrayref) {
	my $poi = {};
	for my $i ( 0.. $#columns) {
	    $poi->{$columns[$i]} = $row->[$i];
	}
	push(@poi_list,$poi);
    }
    $sth->finish;

    return @poi_list;
}

#############################################################################
# Add all Waypoints from Hash into th MySQL POI Database
#############################################################################
sub db_add_waypoints($){
    my $waypoints = shift;
    print "Adding Waypoints to Database\n";

    for my $wp_name ( sort keys  %{$waypoints} ) {
	my $values = $waypoints->{$wp_name};

	unless ( defined($values->{'poi.lat'}) && 
		 defined($values->{'poi.lon'}) ) {
	    print "Error undefined lat/lon: ".Dumper(\$values);
	}

	correct_lat_lon($values);

	for my $t (qw(Wlan Action Sqlnr Proximity) ) {
	    unless ( defined ( $values->{$t})) {
		$values->{$t} = 0;
	    }
	}

	$values->{Proximity} =~ s/\s*m$//;
	poi_add($values);
    }
}

#############################################################################
# Add a single poit into DB
sub poi_add($){
    my $poi = shift;
    my $point = {};
    my @columns = column_names("poi");
    map { $point->{"poi.$_"} = ( $poi->{"poi.$_"} || $poi->{$_} || $poi->{lc($_)}) } @columns;
    @columns = column_names("address");
    map { $point->{"address.$_"} = ( $poi->{"address.$_"} || $poi->{$_} || $poi->{lc($_)}) } @columns;
    @columns = column_names("source");
    map { $point->{"source.$_"} = ( $poi->{"source.$_"} || $poi->{$_} || $poi->{lc($_)}) } @columns;

    # ---------------------- SOURCE
    #print Dumper(\$point);
    if ( $point->{"source.name"} && ! $point->{'poi.source_id'}) {
	my $source_id = source_name2id($point->{"source.name"});
	# print "Source: $point->{'source.name'} -> $source_id\n";
	
	$point->{'source.source_id'} = $source_id;
	$point->{'poi.source_id'}    = $source_id;
    }

    # ---------------------- ADDRESS
    $point->{'poi.address_id'}    ||= 0;

    # ---------------------- TYPE
    $point->{'poi.type_id'}       ||= 0;

    # ---------------------- POI
    $point->{'poi.last_modified'} ||= time();
    $point->{'poi.level_min'}         ||= 0;
    $point->{'poi.level_max'}         ||= 0;
    insert_hash("poi",$point);

}

# -----------------------------------------------------------------------------

sub create_db(){
    my $create_statement;
    my $dbh;
    my $sth; 

    $create_statement='CREATE DATABASE IF NOT EXISTS geoinfo;';
    $dbh = db_connect();
    $sth = $dbh->prepare($create_statement);
    $sth->execute();
    
    $create_statement='CREATE TABLE IF NOT EXISTS `address` (
  `address_id` int(11) NOT NULL auto_increment,
  `country` varchar(40) NOT NULL default \'\',
  `state` varchar(80) NOT NULL default \'\',
  `zip` varchar(5) NOT NULL default \'\',
  `city` varchar(80) NOT NULL default \'\',
  `city_part` varchar(80) NOT NULL default \'\',
  `street_name` varchar(80) NOT NULL default \'\',
  `street_number` varchar(5) NOT NULL default \'\',
  `phone` varchar(160) NOT NULL default \'\',
  `comment` varchar(160) NOT NULL default \'\',
  PRIMARY KEY  (`address_id`)
) TYPE=MyISAM;
';
    $dbh = db_connect();
    $sth = $dbh->prepare($create_statement);
    $sth->execute();

    $create_statement='
CREATE TABLE IF NOT EXISTS `navicaches` (
  `cache_id` int(11) unsigned NOT NULL default \'0\',
  `waypoint` varchar(7) NOT NULL default \'0\',
  `country_code` char(2) default NULL,
  `state` varchar(9) default NULL,
  `city` varchar(64) default NULL,
  `name` varchar(128) default NULL,
  `user_name` varchar(64) default NULL,
  `hidden_date` date default NULL,
  `modified_datetime` datetime default NULL,
  `latitude` float(11,8) default NULL,
  `longitude` float(11,8) default NULL,
  `difficulty` float(2,1) default NULL,
  `terrain` float(2,1) default NULL,
  `retired` varchar(5) default NULL,
  `cache_type` varchar(16) default NULL,
  `cache_size` varchar(16) default NULL,
  `handicapped` varchar(5) default NULL,
  `water` varchar(5) default NULL,
  `restrooms` varchar(5) default NULL,
  `parking_lot` varchar(5) default NULL,
  `pets` varchar(5) default NULL,
  `fees` varchar(5) default NULL,
  `open_cache` varchar(5) default NULL,
  `comments` text,
  `description` text,
  `cluetitle1` varchar(128) default NULL,
  `clue1` text,
  `cluetitle2` varchar(128) default NULL,
  `clue2` text,
  `cluetitle3` varchar(128) default NULL,
  `clue3` text,
  `cluetitle4` varchar(128) default NULL,
  `clue4` text,
  `cluetitle5` varchar(128) default NULL,
  `clue5` text,
  `pictitle1` varchar(128) default NULL,
  `pic1` varchar(128) default NULL,
  `pictitle2` varchar(128) default NULL,
  `pic2` varchar(128) default NULL,
  `pictitle3` varchar(128) default NULL,
  `pic3` varchar(128) default NULL,
  `pictitle4` varchar(128) default NULL,
  `pic4` varchar(128) default NULL,
  `pictitle5` varchar(128) default NULL,
  `pic5` varchar(128) default NULL,
  `source` varchar(64) default NULL,
  PRIMARY KEY  (`cache_id`,`waypoint`)
) TYPE=MyISAM;
';
$dbh = db_connect();
$sth = $dbh->prepare($create_statement);
$sth->execute();

$create_statement='
CREATE TABLE IF NOT EXISTS `poi` (
  `wp_id` int(11) NOT NULL auto_increment,
  `name` varchar(80) default NULL,
  `type_id` int(11) NOT NULL default \'0\',
  `lat` double default \'0\',
  `lon` double default \'0\',
  `alt` double default \'0\',
  `proximity` float default \'0\',
  `comment` varchar(255) default NULL,
  `level_min` smallint(6) NOT NULL default \'0\',
  `level_max` smallint(6) NOT NULL default \'0\',
  `last_modified` date NOT NULL default \'0000-00-00\',
  `url` varchar(160) NULL ,
  `address_id` int(11) default \'0\',
  `source_id` int(11) NOT NULL default \'0\',
  PRIMARY KEY  (`wp_id`),
  KEY `last_modified` (`last_modified`),
  KEY `name` (`name`)
) TYPE=MyISAM;
';
$dbh = db_connect();
$sth = $dbh->prepare($create_statement);
$sth->execute();

$create_statement='
CREATE TABLE IF NOT EXISTS `source` (
  `source_id` int(11) NOT NULL auto_increment,
  `name` varchar(80) NOT NULL default \'\',
  `licence` varchar(160) NOT NULL default \'\',
  `url` varchar(160) NOT NULL default \'\',
  `comment` varchar(160) NOT NULL default \'\',
  `last_update` date NOT NULL default \'0000-00-00\',
  PRIMARY KEY  (`source_id`)
) TYPE=MyISAM;
';
$dbh = db_connect();
$sth = $dbh->prepare($create_statement);
$sth->execute();

$create_statement='
CREATE TABLE IF NOT EXISTS `type` (
  `type_id` int(11) NOT NULL auto_increment,
  `name` varchar(80) NOT NULL default \'\',
  `symbol` varchar(160) NOT NULL default \'\',
  `description` varchar(160) NOT NULL default \'\',
  PRIMARY KEY  (`type_id`),
  FULLTEXT KEY `description` (`description`)
) TYPE=MyISAM;
';
$dbh = db_connect();
$sth = $dbh->prepare($create_statement);
$sth->execute();

my @icons = qw( unknown
		Ausbildung.GrundSchule
		Ausbildung.Gymnasium
		Ausbildung.HauptSchule
		Ausbildung.UNI
		Ausbildung.VHS
		Ausbildung.Kindergarten
		Ausbildung.Kinderkrippe
		Bank
		Bank.Geldautomat
		Bank.Geldautomat.EC
		Transport.Blitzampel
		Briefkasten
		Camping Platz
		EC-Automat
		Einkaufszentrum
		Essen-Lieferservice
		Essen.GaststÃÂ¤tte
		Essen.Kneipe
		Essen.Restaurant
		Essen.Schnellrestaurant
		Essen.Schnellrestaurant.Burger-King
		Essen.Schnellrestaurant.MC-Donalds
		Feuerwehr
		Einkaufen.Flohmarkt
		Einkaufen.Lebensmittel.Supermarkt
		Einkaufen.Lebensmittel
		Einkaufen.Baumarkt
		Transport.Flugplatz
		Freizeit.Kino
		Freizeit.Nationalpark
		Freizeit.Oper
		Freizeit.Reiterhof
		Freizeit.SehenswÃÂ¼rdigkeit
		Freizeit.SehenswÃÂ¼rdigkeit.Museum
		Freizeit.Spielplatz
		Freizeit.Sport.Fussball-Stadion
		Freizeit.Sport.Fussballplatz
		Freizeit.Sport.Golfplatz
		Freizeit.Sport.Minigolfplatz
		Freizeit.Sport.Tennisplatz
		Freizeit.Theater
		Friedhof
		FussgÃÂ¤nger Zone
		Geldwechsel
		Geocache
		Gesundheit.Ambulanz
		Gesundheit.Apotheke
		Gesundheit.Arzt
		Gesundheit.Krankenhaus
		Gesundheit.Notaufnahme
		Gift-Shop
		Kino
		Kirche.Evangelisch
		Kirche.Katholisch
		Kirche.Synagoge
		Messe
		Freizeit.Modellflugplatz
		Notruf sÃÂ¤ule
		Transport.Park&Ride
		Transport.Parkplatz
		Polizei
		Post
		Recycling.Altglas
		Recycling.Altpapier
		Recycling.Wertstoffhof
		Freizeit.Rummelplatz
		Stadt Information
		Stadthalle
		Telefon-Zelle
		Transport.Auto-Verlade-Bahnhof
		Transport.Bahnhof
		Transport.Bus-Haltestelle
		Transport.FÃÂ¤hre
		Transport.GÃÂ¼ter-Verlade-Bahnhof
		Transport.Lotsendienst
		Transport.Mautstation
		Transport.RaststÃï¿½Å¹tte
		Transport.S-Bahn-Haltestelle
		Transport.Strassen-Bahn-Haltestelle
		Transport.Strassen.30-Zohne
		Transport.Strassen.Autobahn
		Transport.Strassen.Bundesstrasse
		Transport.Strassen.Innerorts
		Transport.Strassen.Landstrasse
		Transport.Tankstelle
		Transport.Taxi-Stand
		Transport.U-Bahn-Haltestelle
		Transport.Verkehr.Baustelle
		Transport.Verkehr.Radarfalle
		Transport.Verkehr.Stau
		Transport.Werkstatt
		Freizeit.Veranstaltungshalle
		Verwaltung.Rathaus
		Verwaltung.Zulassungsstele
		WC
		Transport.Auto.Werkstatt
		Freizeit.Zoo
		Ãœbernachtung.Hotel
		Ãœbernachtung.Jugend Herberge
		Ãœbernachtung.Motel
		Ãœbernachtung.ZeltplatzSchwimmbad
		);

    my $i=1;
    for my $icon  ( @icons ) {    
	my $statement;
	$statement = "DELETE FROM `type` WHERE type_id = $i LIMIT 1;\n";
	$dbh = db_connect();
	$sth = $dbh->prepare($statement);
	$sth->execute();
	
	$statement = "INSERT INTO `type` VALUES ($i,'$icon','$icon.png','$icon');\n";
	$dbh = db_connect();
	$sth = $dbh->prepare($statement);
	$sth->execute();
	$i++;
    }
    
    print "Creation completed\n";
}
# -----------------------------------------------------------------------------

1;
