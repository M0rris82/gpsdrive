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
        @EXPORT = qw( &type_names &type_list &db_disconnect 
		      &add_poi &add_poi_multi
		      &poi_list
		      &column_names
		      &source_name2id &type_name2id
		      &delete_all_from_source);
        %EXPORT_TAGS = ( );     # eg: TAG => [ qw!name1 name2! ],
        # your exported package globals go here,
        # as well as any optionally exported functions
        @EXPORT_OK   = qw();

}
our @EXPORT_OK;


END { } 

# -----------------------------------------------------------------------------

#retrieve Column Names for desired table
sub column_names($){
    my $table = shift;
    my @col;

    my $dbh = db_connect();
    my $query = "SHOW COLUMNS FROM geoinfo.$table;";
    my $sth=$dbh->prepare($query) or die $dbh->errstr;
    $sth->execute() or die $sth->errstr;
    while ( my $array_ref = $sth->fetchrow_arrayref() ) {
	push ( @col ,$array_ref->[0] );
    }
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
# All necessary information for connecting the DB
# these are:  host,  user and passwort; the db is always geoinfo
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
# convert type name to type_id and cache it locally
my $type_id_cache;
sub type_name2id($){
    my $type_name = shift ||'';
    my $type_id;
    return 0 unless $type_name;
    if ( defined $type_id_cache->{$type_name} ) {
	$type_id = $type_id_cache->{$type_name};
    } else {
	my $dbh = db_connect();
	my $query = "SELECT type_id FROM type WHERE type.name = '$type_name' LIMIT 1";
	my $sth=$dbh->prepare($query) or die $dbh->errstr;
	$sth->execute() or die $sth->errstr;
	my $array_ref = $sth->fetchrow_arrayref();
	if ( $array_ref ) {
	    $type_id = $array_ref->[0];
	    $type_id_cache->{$type_name} = $type_id;
	} else {
	    # Nicht gefunden
	    $type_id=0;
	}
	$sth->finish;
    }

#    debug("Type: $type_name -> $type_id");

    return $type_id;
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
sub add_poi_multi($){
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
	add_poi($values);
    }
}

#############################################################################
# Add a single poit into DB
sub add_poi($){
    my $poi = shift;
    my $point = {};
    my @columns = column_names("poi");
    map { $point->{"poi.$_"} = ( $poi->{"poi.$_"} || $poi->{$_} || $poi->{lc($_)}) } @columns;

    # ---------------------- SOURCE
    #print Dumper(\$point);
    if ( $point->{"source.name"} && ! $point->{'poi.source_id'}) {
	my $source_id = source_name2id($point->{"source.name"});
	# print "Source: $point->{'source.name'} -> $source_id\n";
	
	$point->{'source.source_id'} = $source_id;
	$point->{'poi.source_id'}    = $source_id;
    }

    # ---------------------- Type
    my $type_name = $poi->{'type.name'};
    if ( $type_name && ! $point->{'poi.type_id'}) {
	my $type_id = type_name2id($type_name);
	unless ( $type_id ) {
	    my $type_hash= {
		'type.name' => $type_name
	    };
	    insert_hash("type",$type_hash);
	    $type_id = type_name2id($point->{"type.name"});
	}
	$point->{'poi.type_id'}    = $type_id;
    }

    # ---------------------- ADDRESS
    $point->{'poi.address_id'}    ||= 0;

    # ---------------------- TYPE
    $point->{'poi.type_id'}       ||= 0;

    # ---------------------- POI
    $point->{'poi.last_modified'} ||= time();
    $point->{'poi.scale_min'}         ||= 0;
    $point->{'poi.scale_max'}         ||= 0;
    insert_hash("poi",$point);

}

#############################################################################
# Add a single streets into DB
sub streets_add($){
    my $segment = shift;
    my $segment4db = {};
    my @columns = column_names("streets");
    map { 
	$segment4db->{"streets.$_"} = 
	    ( $segment->{"streets.$_"} || $segment->{$_} || $segment->{lc($_)}) 
	} @columns;

    # ---------------------- SOURCE
    #print Dumper(\$segment4db);
    # TODO: put this out here for performance reason
    if ( $segment4db->{"source.name"} && ! $segment4db->{'streets.source_id'}) {
	my $source_id = source_name2id($segment4db->{"source.name"});
	# print "Source: $segment4db->{'source.name'} -> $source_id\n";
	
	$segment4db->{'source.source_id'} = $source_id;
	$segment4db->{'streets.source_id'}    = $source_id;
    }

    # ---------------------- Type
    my $type_name = $segment->{'type.name'};
    if ( $type_name && ! $segment4db->{'streets.type_id'}) {
	my $type_id = type_name2id($type_name);
	unless ( $type_id ) {
	    my $type_hash= {
		'type.name' => $type_name
	    };
	    insert_hash("type",$type_hash);
	    $type_id = type_name2id($segment4db->{"type.name"});
	}
	$segment4db->{'streets.type_id'}    = $type_id;
    }

    # ---------------------- ADDRESS
    $segment4db->{'streets.address_id'}    ||= 0;

    # ---------------------- TYPE
    $segment4db->{'streets.type_id'}       ||= 0;

    # ---------------------- STREETS
    $segment4db->{'streets.last_modified'} ||= time();
    $segment4db->{'streets.scale_min'}         ||= 0;
    $segment4db->{'streets.scale_max'}         ||= 0;
    insert_hash("streets",$segment4db);

}


#############################################################################
# Add a list of street segments into streets-DB
sub segments_add($){
    my $data = shift;
    my $segment4db = {};
    my @columns = column_names("streets");
    map { 
	$segment4db->{"streets.$_"} = 
	    ( $data->{"streets.$_"} || $data->{$_} || $data->{lc($_)}) 
	} @columns;

    # ---------------------- ADDRESS
    $segment4db->{'streets.address_id'}    ||= 0;

    # ---------------------- TYPE
    $segment4db->{'streets.type_id'}       ||= 0;

    # ---------------------- STREETS
    $segment4db->{'streets.last_modified'} ||= time();
    $segment4db->{'streets.scale_min'}         ||= 0;
    $segment4db->{'streets.scale_max'}         ||= 0;
    my $count = scalar @{$data->{segments}};
    debug("Writing $count Segments") if $count ;
    $segment4db->{'streets.lat2'}=0;
    for my $segment ( @{$data->{segments}} ){
	$segment4db->{'streets.lat1'} = $segment4db->{'streets.lat2'};
	$segment4db->{'streets.lon1'} = $segment4db->{'streets.lon2'};
	$segment4db->{'streets.alt2'} = $segment4db->{'streets.alt2'};

	$segment4db->{'streets.lat2'} = $segment->{lat};
	$segment4db->{'streets.lon2'} = $segment->{lon};
	$segment4db->{'streets.alt1'} = $segment->{alt1};

	next unless $segment4db->{'streets.lat1'}; # skip first entry

	$segment4db->{'streets.name'}      = $data->{name}      || $segment->{name};
	$segment4db->{'streets.type_id'}   = $data->{type_id}   || $segment->{type_id};
	$segment4db->{'streets.source_id'} = $data->{source_id} || $segment->{source_id};
	$segment4db->{'streets.scale_min'} = $data->{scale_min} || $segment->{scale_min}||1;
	$segment4db->{'streets.scale_max'} = $data->{scale_max} || $segment->{scale_max}||10000000000;

	#print Dumper(\$segment4db);
	insert_hash("streets",$segment4db);
    }
}

# -----------------------------------------------------------------------------
sub db_exec($){
    my $statement = shift;
    debug("db_exec($statement)");

    my $dbh = db_connect();
    my $sth = $dbh->prepare($statement);
    $sth->execute() 
	or warn $sth->errstr."\n";
}

# -----------------------------------------------------------------------------
sub add_index($){
    my $table = shift;

    if ( $table eq "poi" ){
	for my $key ( qw( last_modified name lat lon ) ){
	    db_exec("ALTER TABLE `$table` ADD INDEX `$key` ( `$key` );");
	    
	}
    } elsif ( $table eq "streets" ){
	for my $key ( qw( last_modified name lat1 lon1 lat2 lon2 ) ){
	    db_exec("ALTER TABLE `$table` ADD INDEX  `$key` ( `$key` );");
	}
    }
#ALTER TABLE `address` ADD FULLTEXT ( `comment` )
}

# -----------------------------------------------------------------------------
sub drop_index($){
    my $table = shift;

    if ( $table eq "poi" ){
	for my $key ( qw( last_modified name lat lon ) ){
	    db_exec("ALTER TABLE `$table` DROP INDEX `$key` ;");
	    
	}
    } elsif ( $table eq "streets" ){
	for my $key ( qw( last_modified name lat1 lon1 lat2 lon2 ) ){
	    db_exec("ALTER TABLE `$table` DROP INDEX `$key` ;");
	}
    }
}

# -----------------------------------------------------------------------------
sub create_db(){
    my $create_statement;
    my $dbh;
    my $sth; 

    $create_statement='CREATE DATABASE IF NOT EXISTS geoinfo;';
    my $drh = DBI->install_driver("mysql");
    my $rc = $drh->func('createdb', 'geoinfo', 'localhost', 
			$main::db_user,$main::db_password, 'admin');
    $dbh = db_connect();
    $sth = $dbh->prepare($create_statement);
    $sth->execute();
    
db_exec('CREATE TABLE IF NOT EXISTS `address` (
  `address_id`    int(11)      NOT NULL auto_increment,
  `country`       varchar(40)  NOT NULL default \'\',
  `state`         varchar(80)  NOT NULL default \'\',
  `zip`           varchar(5)   NOT NULL default \'\',
  `city`          varchar(80)  NOT NULL default \'\',
  `city_part`     varchar(80)  NOT NULL default \'\',
  `street_name`   varchar(80)  NOT NULL default \'\',
  `street_number` varchar(5)   NOT NULL default \'\',
  `phone`         varchar(160) NOT NULL default \'\',
  `comment`       varchar(160) NOT NULL default \'\',
  PRIMARY KEY  (`address_id`)
) TYPE=MyISAM;');
    add_index('address');

db_exec('CREATE TABLE IF NOT EXISTS `poi` (
  `poi_id`        int(11)      NOT NULL auto_increment,
  `name`          varchar(80)           default NULL,
  `type_id`       int(11)      NOT NULL default \'0\',
  `lat`           double                default \'0\',
  `lon`           double                default \'0\',
  `alt`           double                default \'0\',
  `proximity`     float                 default \'0\',
  `comment`       varchar(255)          default NULL,
  `scale_min`     int(12)  NOT NULL default \'0\',
  `scale_max`     int(12)  NOT NULL default \'0\',
  `last_modified` date         NOT NULL default \'0000-00-00\',
  `url`           varchar(160)     NULL ,
  `address_id`    int(11)               default \'0\',
  `source_id`     int(11)      NOT NULL default \'0\',
  PRIMARY KEY  (`poi_id`)
) TYPE=MyISAM;');
    add_index('poi');


db_exec('CREATE TABLE IF NOT EXISTS `streets` (
  `street_id`         int(11)      NOT NULL auto_increment,
  `name`          varchar(80)           default NULL,
  `type_id`       int(11)      NOT NULL default \'0\',
  `lat1`          double                default \'0\',
  `lon1`          double                default \'0\',
  `alt1`           double                default \'0\',
  `lat2`          double                default \'0\',
  `lon2`          double                default \'0\',
  `alt2`           double                default \'0\',
  `proximity`     float                 default \'0\',
  `comment`       varchar(255)          default NULL,
  `scale_min`     int(12)  NOT NULL default \'0\',
  `scale_max`     int(12)  NOT NULL default \'0\',
  `last_modified` date         NOT NULL default \'0000-00-00\',
  `source_id`     int(11)      NOT NULL default \'0\',
  PRIMARY KEY  (`street_id`)
) TYPE=MyISAM;');
    add_index('streets');


db_exec('CREATE TABLE IF NOT EXISTS `source` (
  `source_id`   int(11)      NOT NULL auto_increment,
  `name`        varchar(80)  NOT NULL default \'\',
  `licence`     varchar(160) NOT NULL default \'\',
  `url`         varchar(160) NOT NULL default \'\',
  `comment`     varchar(160) NOT NULL default \'\',
  `last_update` date         NOT NULL default \'0000-00-00\',
  PRIMARY KEY  (`source_id`)
) TYPE=MyISAM;');
    add_index('source');

db_exec('CREATE TABLE IF NOT EXISTS `type` (
  `type_id`     int(11)      NOT NULL auto_increment,
  `lang`        varchar(2)       NULL default \'en\',
  `name`        varchar(80)  NOT NULL default \'\',
  `symbol`      varchar(160)     NULL default \'\',
  `description` varchar(160)     NULL default \'\',
  PRIMARY KEY  (`type_id`)
) TYPE=MyISAM;');
    add_index('type');

my @icons = qw( de.unknown
		de.Ausbildung.GrundSchule
		de.Ausbildung.Gymnasium
		de.Ausbildung.HauptSchule
		de.Ausbildung.UNI
		de.Ausbildung.VHS
		de.Ausbildung.Kindergarten
		de.Ausbildung.Kinderkrippe
		de.Bank
		de.Bank.Geldautomat
		de.Bank.Geldautomat.EC
		de.Transport.Blitzampel
		de.Briefkasten
		de.Camping Platz
		de.EC-Automat
		de.Einkaufszentrum
		de.Essen-Lieferservice
		de.Essen.Gaststaette
		de.Essen.Kneipe
		de.Essen.Restaurant
		de.Essen.Schnellrestaurant
		de.Essen.Schnellrestaurant.Burger-King
		de.Essen.Schnellrestaurant.MC-Donalds
		de.Feuerwehr
		de.Einkaufen.Flohmarkt
		de.Einkaufen.Lebensmittel.Supermarkt
		de.Einkaufen.Lebensmittel
		de.Einkaufen.Baumarkt
		de.Transport.Flugplatz
		de.Freizeit.Kino
		de.Freizeit.Nationalpark
		de.Freizeit.Oper
		de.Freizeit.Reiterhof
		de.Freizeit.Sehenswuerdigkeit
		de.Freizeit.Sehenswuerdigkeit.Museum
		de.Freizeit.Spielplatz
		de.Freizeit.Sport.Fussball-Stadion
		de.Freizeit.Sport.Fussballplatz
		de.Freizeit.Sport.Golfplatz
		de.Freizeit.Sport.Minigolfplatz
		de.Freizeit.Sport.Tennisplatz
		de.Freizeit.Theater
		de.Friedhof
		de.Fussgaenger Zone
		de.Geldwechsel
		de.Geocache
		de.Gesundheit.Ambulanz
		de.Gesundheit.Apotheke
		de.Gesundheit.Arzt
		de.Gesundheit.Krankenhaus
		de.Gesundheit.Notaufnahme
		de.Gift-Shop
		de.Kino
		de.Kirche.Evangelisch
		de.Kirche.Katholisch
		de.Kirche.Synagoge
		de.Messe
		de.Freizeit.Modellflugplatz
		de.Notruf saeule
		de.Transport.Park&Ride
		de.Transport.Parkplatz
		de.Polizei
		de.Post
		de.Recycling.Altglas
		de.Recycling.Altpapier
		de.Recycling.Wertstoffhof
		de.Freizeit.Rummelplatz
		de.Stadt Information
		de.Stadthalle
		de.Telefon-Zelle
		de.Transport.Auto-Verlade-Bahnhof
		de.Transport.Bahnhof
		de.Transport.Bus-Haltestelle
		de.Transport.Faehre
		de.Transport.Gueter-Verlade-Bahnhof
		de.Transport.Lotsendienst
		de.Transport.Mautstation
		de.Transport.Raststaette
		de.Transport.S-Bahn-Haltestelle
		de.Transport.Strassen-Bahn-Haltestelle
		de.Transport.Strassen.30-Zohne
		de.Transport.Strassen.Autobahn
		de.Transport.Strassen.Bundesstrasse
		de.Transport.Strassen.Innerorts
		de.Transport.Strassen.Landstrasse
		de.Transport.Tankstelle
		de.Transport.Taxi-Stand
		de.Transport.U-Bahn-Haltestelle
		de.Transport.Verkehr.Baustelle
		de.Transport.Verkehr.Radarfalle
		de.Transport.Verkehr.Stau
		de.Transport.Werkstatt
		de.Freizeit.Veranstaltungshalle
		de.Verwaltung.Rathaus
		de.Verwaltung.Zulassungsstele
		de.WC
		de.Transport.Auto.Werkstatt
		de.Freizeit.Zoo
		de.Uebernachtung.Hotel
		de.Uebernachtung.Jugend Herberge
		de.Uernachtung.Motel
		de.Uernachtung.ZeltplatzSchwimmbad
		);

    my $i=1;
    for my $icon  ( @icons ) {    
	my $statement;
	$icon =~ s/(..)\.//;
	my $lang =$1;
	$lang ||= 'en';
	db_exec("DELETE FROM `type` WHERE type_id = $i AND `lang` = '$lang';");
	db_exec("INSERT INTO `type` VALUES ($i,'$lang','$icon','$icon.png','$icon');");
	$i++;
    }
    print "Creation completed\n";
}
# -----------------------------------------------------------------------------

1;
