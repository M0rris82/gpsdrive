# Database Functions for poi.pl
#
# $Log$
# Revision 1.15  2005/04/12 06:12:58  tweety
# DBFuncs:
#  delete_all_from_source($source_name) now deletes streets and poi
#  more poi_type_names
#  adjusted some of the allocations old Waypoint-Type --> POI_type
#  read waypoint files with an \t seperator
#  Insert Street Samples
# poi.c:
#  Added Full Path Search for Icons
# streets.c:
#  implement basic colors for streets
#  added text to streets in debug mode
#
# Revision 1.14  2005/04/10 00:15:58  tweety
# changed primary language for poi-type generation to english
# added translation for POI-types
# added some icons classifications to poi-types
# added LOG: Entry for CVS to some *.pm Files
#

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
    @EXPORT = qw( &poi_type_names &poi_type_list  &poi_type_name2id
		  &streets_type_names &streets_type_list  &streets_type_name2id
		  &db_disconnect 
		  &add_poi &add_poi_multi
		  &poi_list
		  &column_names
		  &source_name2id
		  &delete_all_from_source);
    %EXPORT_TAGS = ( );     # eg: TAG => [ qw!name1 name2! ],
    # your exported package globals go here,
    # as well as any optionally exported functions
    @EXPORT_OK   = qw();

}
our @EXPORT_OK;


END { } 

# -----------------------------------------------------------------------------
# switch off updating of index
sub disble_keys($){    
    my $table = shift;
    db_exec("ALTER TABLE  $table DISABLE KEYS;");
}

# -----------------------------------------------------------------------------
# switch on updating of index
sub enable_keys($){
    my $table = shift;
    db_exec("ALTER TABLE  $table ENABLE KEYS;");
}

# -----------------------------------------------------------------------------
#retrieve Column Names for desired table
sub column_names($){
    my $table = shift;
    my @col;

    my $dbh = db_connect();
    my $query = "SHOW COLUMNS FROM geoinfo.$table;";

    my $sth=$dbh->prepare($query) or die $dbh->errstr;
    $sth->execute()               or die $sth->errstr;

    while ( my $array_ref = $sth->fetchrow_arrayref() ) {
	push ( @col ,$array_ref->[0] );
    }
    return @col;
}


# -----------------------------------------------------------------------------
# 
sub show_index($) {
    my $table  = shift;

    my $query = "SHOW INDEX FROM $table;";
    my $dbh = db_connect();
    my $sth=$dbh->prepare($query) or die $dbh->errstr;
    $sth->execute()               or die $sth->errstr;

    my $result;
    foreach my $row ( @{ $sth->fetchall_arrayref({}) } ) {
	$result->{$row->{'Key_name'}} = $row;
    }
    
    return $result;
}

# -----------------------------------------------------------------------------
sub insert_hash {
    my $table = shift;
    my $field_values = shift;

    while ( my $h = shift ) {
	#print "Adding ".Dumper($h);
	map { $field_values->{$_} = $h->{$_} } keys %{$h};
    }
#    my @fields = sort keys %$field_values; # sort required
    $field_values->{"$table.last_update"} ||= time();
    my @fields = map { "$table.$_" } column_names($table);
    my @values = @{$field_values}{@fields};
    my $sql = sprintf ( "insert into %s (%s) values (%s)",
			$table, 
			join(',', @fields),
			join(",", ("?") x scalar(@fields))
			);
    my $dbh = db_connect();
    #print "insert_hash($table, ".Dumper(\$field_values).")\n";
    #print "insert_hash($table, ".join(",",@values).")\n";
    #print "$sql\n";
    my $sth = $dbh->prepare_cached($sql);
    my $res = $sth->execute(@values);
    if ( ! $res ) {
	warn "Error while inserting Hash ".Dumper($field_values)." into table '$table'\n";
	$sth->errstr;
    }
    
    return $res;
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
    $sth->execute()               or die $sth->errstr;
    $sth->finish;

    $query = "DELETE FROM streets WHERE streets.source_id = '$source_id'";
    $sth=$dbh->prepare($query) or die $dbh->errstr;
    $sth->execute()            or die $sth->errstr;

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
	$sth->execute()               or die $sth->errstr;

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
# convert poi_type.name to poi_type_id and cache it locally
my $poi_type_id_cache;
sub poi_type_name2id($){
    my $type_name = shift ||'';
    my $poi_type_id;

    return 0 unless $type_name;

    if ( defined $poi_type_id_cache->{$type_name} ) {
	$poi_type_id = $poi_type_id_cache->{$type_name};
    } else {
	my $dbh = db_connect();
	my $query = "SELECT poi_type_id FROM poi_type WHERE poi_type.name = '$type_name' LIMIT 1";

	my $sth=$dbh->prepare($query) or die $dbh->errstr;
	$sth->execute()               or die $sth->errstr;

	my $array_ref = $sth->fetchrow_arrayref();
	if ( $array_ref ) {
	    $poi_type_id = $array_ref->[0];
	    $poi_type_id_cache->{$type_name} = $poi_type_id;
	} else {
	    # Nicht gefunden
	    $poi_type_id=0;
	}
	$sth->finish;
    }

    debug("Type: $type_name -> $poi_type_id");

    return $poi_type_id;
}

# -----------------------------------------------------------------------------
# get a list of all type names
sub poi_type_names(){
    my @poi_type_names;

    my $dbh = db_connect();
    
    my $query = "SELECT name FROM poi_type";
    
    my $sth=$dbh->prepare($query) or die $dbh->errstr;
    $sth->execute()               or die $sth->errstr;

    while (my $row = $sth->fetchrow_arrayref) {
	push(@poi_type_names,$row->[0]);
    }
    $sth->finish;

    return @poi_type_names;
}

# -----------------------------------------------------------------------------
# retrieve a complete list of known types
# This returns a list with a hash for each type with all relevant data
sub poi_type_list(){
    my @poi_type_list;

    my $dbh = db_connect();
    
    my @columns = column_names("poi_type");
    my $query = "SELECT ".join(',', @columns)."  FROM poi_type";
    
    my $sth=$dbh->prepare($query) or die $dbh->errstr;
    $sth->execute()               or die $sth->errstr;

    while (my $row = $sth->fetchrow_arrayref) {
	my $poi_type = {};
	for my $i ( 0.. $#columns) {
	    $poi_type->{$columns[$i]} = $row->[$i];
	}
	push(@poi_type_list,$poi_type);
    }
    $sth->finish;

    return @poi_type_list;
}





# -----------------------------------------------------------------------------
# convert streets_type.name to streets_type_id and cache it locally
my $streets_type_id_cache;
sub streets_type_name2id($){
    my $type_name = shift ||'';
    my $streets_type_id;

    return 0 unless $type_name;

    if ( defined $streets_type_id_cache->{$type_name} ) {
	$streets_type_id = $streets_type_id_cache->{$type_name};
    } else {
	my $dbh = db_connect();
	my $query = "SELECT streets_type_id FROM streets_type WHERE streets_type.name = '$type_name' LIMIT 1";

	my $sth=$dbh->prepare($query) or die $dbh->errstr;
	$sth->execute()               or die $sth->errstr;

	my $array_ref = $sth->fetchrow_arrayref();
	if ( $array_ref ) {
	    $streets_type_id = $array_ref->[0];
	    $streets_type_id_cache->{$type_name} = $streets_type_id;
	} else {
	    # Nicht gefunden
	    $streets_type_id=0;
	}
	$sth->finish;
    }

#    debug("Type: $type_name -> $streets_type_id");

    return $streets_type_id;
}

# -----------------------------------------------------------------------------
# get a list of all streets_type names
sub streets_type_names(){
    my @streets_type_names;

    my $dbh = db_connect();
    
    my $query = "SELECT name FROM streets_type";
    
    my $sth=$dbh->prepare($query) or die $dbh->errstr;
    $sth->execute()               or die $sth->errstr;

    while (my $row = $sth->fetchrow_arrayref) {
	push(@streets_type_names,$row->[0]);
    }
    $sth->finish;

    return @streets_type_names;
}

# -----------------------------------------------------------------------------
# retrieve a complete list of known types
sub streets_type_list(){
    my @streets_type_list;

    my $dbh = db_connect();
    
    my @columns = column_names("streets_type");
    my $query = "SELECT ".join(',', @columns)."  FROM streets_type";
    
    my $sth=$dbh->prepare($query) or die $dbh->errstr;
    $sth->execute()               or die $sth->errstr;

    while (my $row = $sth->fetchrow_arrayref) {
	my $streets_type = {};
	for my $i ( 0.. $#columns) {
	    $streets_type->{$columns[$i]} = $row->[$i];
	}
	push(@streets_type_list,$streets_type);
    }
    $sth->finish;

    return @streets_type_list;
}


# -----------------------------------------------------------------------------
# retrieve first n entries from  poi Table
# default is 100 Entries
sub poi_list(;$){
    my $limit = shift || 100;
    my @poi_list;

    my $dbh = db_connect();
    
    my @columns = column_names("poi");
    my $query = "SELECT ".join(',', @columns)."  FROM poi LIMIT $limit";
    
    my $sth=$dbh->prepare($query) or die $dbh->errstr;
    $sth->execute()               or die $sth->errstr;

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

	# TODO: Check if this is obsolete
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
# Add a single poi into DB
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

    # ---------------------- POI_Type
    my $type_name = $poi->{'poi_type.name'};
    if ( $type_name && ! $point->{'poi.poi_type_id'}) {
	my $poi_type_id = type_name2id($type_name);
	unless ( $poi_type_id ) {
	    my $type_hash= {
		'poi_type.name' => $type_name
		};
	    insert_hash("poi_type",$type_hash);
	    $poi_type_id = type_name2id($point->{"type.name"});
	}
	$point->{'poi.poi_type_id'}    = $poi_type_id;
    }

    # ---------------------- ADDRESS
    $point->{'poi.address_id'}    ||= 0;

    # ---------------------- TYPE
    $point->{'poi.poi_type_id'}       ||= 0;

    # ---------------------- POI
    $point->{'poi.last_modified'} ||= time();
    $point->{'poi.scale_min'}         ||= 0;
    $point->{'poi.scale_max'}         ||= 9999999999999;
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
    my $type_name = $segment->{'streets_type.name'};
    if ( $type_name && ! $segment4db->{'streets.streets_type_id'}) {
	my $poi_type_id = type_name2id($type_name);
	unless ( $poi_type_id ) {
	    my $type_hash= {
		'streets_type.name' => $type_name
		};
	    insert_hash("streets_type",$type_hash);
	    $poi_type_id = type_name2id($segment4db->{"streets_type.name"});
	}
	$segment4db->{'streets.streets_type_id'}    = $poi_type_id;
    }

    # ---------------------- ADDRESS
    $segment4db->{'streets.address_id'}      ||= 0;

    # ---------------------- TYPE
    $segment4db->{'streets.streets_type_id'} ||= 0;

    # ---------------------- STREETS
    $segment4db->{'streets.last_modified'}   ||= time();
    $segment4db->{'streets.scale_min'}       ||= 0;
    $segment4db->{'streets.scale_max'}       ||= 99999999999999;
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
    $segment4db->{'streets.streets_type_id'}       ||= 0;

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

	$segment4db->{'streets.name'}            = $data->{name}            || $segment->{name};
	$segment4db->{'streets.streets_type_id'} = $data->{streets_type_id} || $segment->{streets_type_id};
	$segment4db->{'streets.source_id'}       = $data->{source_id}       || $segment->{source_id};
	$segment4db->{'streets.scale_min'}       = $data->{scale_min}       || $segment->{scale_min}||1;
	$segment4db->{'streets.scale_max'}       = $data->{scale_max}       || $segment->{scale_max}||10000000000;

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
    unless ( $sth->execute() ) {
	warn "Error in query '$statement'\n";
	$sth->errstr;
	return 0;
    }
    return 1;
}


# -----------------------------------------------------------------------------
# create known indices for given table
# if they dont exist already
sub add_if_not_exist_index($$;$){
    my $table = shift;
    my $name  = shift;
    my $keys  = shift || $name;
    
    my $indices = show_index($table);
    #print Dumper(\$indices);
    #printf "Index for $table: %s\n" , join(",",keys %{$indices});
    if ( $keys =~ m/,/ &&
	 defined $indices->{$name}->{'Column_name'} ) {
	print "Multi Index $name	Exists\n";
    } elsif ( $indices->{$name}->{'Column_name'} eq $keys ) {
	print "Index $table.$name	Exists\n";
    } else {
	if ( defined $indices->{$name}->{'Column_name'} ) {
	    print "Droping Index: $table.$name\n";
	    db_exec("ALTER TABLE `$table` DROP INDEX `$name`;");
	}

	print "Adding Index: $table.$name ( `$keys` )\n";
	db_exec("ALTER TABLE `$table` ADD INDEX `$name` ( `$keys` );");
    }
}

# -----------------------------------------------------------------------------
# create known indices for given table
sub add_index($){
    my $table = shift;

    if ( $table eq "poi" ){
	for my $key ( qw( last_modified name lat lon ) ){
	    add_if_not_exist_index($table,$key);
	}
	add_if_not_exist_index( $table,'combi1','lat`,`lon`,`scale_min`,`scale_max');
    } elsif ( $table eq "streets" ){
	for my $key ( qw( last_modified name lat1 lon1 lat2 lon2 ) ){
	    add_if_not_exist_index($table,$key);
	}
	add_if_not_exist_index($table,'combi1','lat1`,`lon1`,`lat2`,`lon2`,`scale_min`,`scale_max');
    } elsif ( $table eq "waypoints" ){
	for my $key ( qw( macaddr type name typenr ) ){
	    add_if_not_exist_index($table,$key);
	}
    } elsif ( $table eq "source" ){
	for my $key ( qw( name ) ){
	    add_if_not_exist_index($table,$key);
	}
    } elsif ( $table eq "poi_type" ){
	for my $key ( qw( name ) ){
	    add_if_not_exist_index($table,$key);
	}
    } elsif ( $table eq "streets_type" ){
	for my $key ( qw( name ) ){
	    add_if_not_exist_index($table,$key);
	}
    } 
    
    # TODO: add more index
    #ALTER TABLE `address` ADD FULLTEXT ( `comment` )
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
    $sth->execute()
	or die $sth->errstr;
    
    db_exec('CREATE TABLE IF NOT EXISTS `address` (
                      `address_id`     int(11)      NOT NULL auto_increment,
                      `country`        varchar(40)  NOT NULL default \'\',
                      `state`          varchar(80)  NOT NULL default \'\',
                      `zip`            varchar(5)   NOT NULL default \'\',
                      `city`           varchar(80)  NOT NULL default \'\',
                      `city_part`      varchar(80)  NOT NULL default \'\',
                      `streets_name`   varchar(80)  NOT NULL default \'\',
                      `streets_number` varchar(5)   NOT NULL default \'\',
                      `phone`          varchar(160) NOT NULL default \'\',
                      `comment`        varchar(160) NOT NULL default \'\',
                      PRIMARY KEY  (`address_id`)
                    ) TYPE=MyISAM;') or die;
    add_index('address');

    db_exec('CREATE TABLE IF NOT EXISTS `poi` (
                      `poi_id`        int(11)      NOT NULL auto_increment,
                      `name`          varchar(80)           default NULL,
                      `poi_type_id`   int(11)      NOT NULL default \'0\',
                      `lat`           double                default \'0\',
                      `lon`           double                default \'0\',
                      `alt`           double                default \'0\',
                      `proximity`     float                 default \'0\',
                      `comment`       varchar(255)          default NULL,
                      `scale_min`     int(12)      NOT NULL default \'0\',
                      `scale_max`     int(12)      NOT NULL default \'0\',
                      `last_modified` date         NOT NULL default \'0000-00-00\',
                      `url`           varchar(160)     NULL ,
                      `address_id`    int(11)               default \'0\',
                      `source_id`     int(11)      NOT NULL default \'0\',
                      PRIMARY KEY  (`poi_id`)
                    ) TYPE=MyISAM;') or die;
    add_index('poi');


    db_exec('CREATE TABLE IF NOT EXISTS `streets` (
                      `streets_id`      int(11)      NOT NULL auto_increment,
                      `name`            varchar(80)           default NULL,
                      `streets_type_id` int(11)      NOT NULL default \'0\',
                      `lat1`            double                default \'0\',
                      `lon1`            double                default \'0\',
                      `alt1`            double                default \'0\',
                      `lat2`            double                default \'0\',
                      `lon2`            double                default \'0\',
                      `alt2`            double                default \'0\',
                      `proximity`       float                 default \'0\',
                      `comment`         varchar(255)          default NULL,
                      `scale_min`       int(12)      NOT NULL default \'0\',
                      `scale_max`       int(12)      NOT NULL default \'0\',
                      `last_modified`   date         NOT NULL default \'0000-00-00\',
                      `source_id`       int(11)      NOT NULL default \'0\',
                      PRIMARY KEY  (`streets_id`)
                    ) TYPE=MyISAM;') or die;
    add_index('streets');

    db_exec('CREATE TABLE IF NOT EXISTS `streets_type` (
                      `streets_type_id` int(11)      NOT NULL auto_increment,
                      `name`            varchar(80)  NOT NULL default \'\',
                      `name_de`         varchar(80)      NULL default \'en\',
                      `description`     varchar(160)     NULL default \'\',
                      `description_de`  varchar(160)     NULL default \'\',
                      `color`           varchar(80)      NULL default \'\',
                      `linetype`        varchar(80)      NULL default \'\',
                      PRIMARY KEY  (`streets_type_id`)
                    ) TYPE=MyISAM;') or die;
    add_index('streets_type');

    db_exec('CREATE TABLE IF NOT EXISTS `source` (
                      `source_id`      int(11)      NOT NULL auto_increment,
                      `name`           varchar(80)  NOT NULL default \'\',
                      `comment`        varchar(160) NOT NULL default \'\',
                      `last_update`    date         NOT NULL default \'0000-00-00\',
                      `url`            varchar(160) NOT NULL default \'\',
                      `licence`        varchar(160) NOT NULL default \'\',
                      PRIMARY KEY  (`source_id`)
                    ) TYPE=MyISAM;') or die;
    add_index('source');

    db_exec('CREATE TABLE IF NOT EXISTS `poi_type` (
                      `poi_type_id` int(11)      NOT NULL auto_increment,
                      `name`        varchar(80)  NOT NULL default \'\',
                      `name_de`     varchar(80)       NULL default \'en\',
                      `symbol`      varchar(160)     NULL default \'\',
                      `description` varchar(160)     NULL default \'\',
                      `description_de` varchar(160)     NULL default \'\',
                      PRIMARY KEY  (`poi_type_id`)
                    ) TYPE=MyISAM;') or die;
    add_index('poi_type');


    # For compatibility to old DB Structure
    db_exec('CREATE TABLE  IF NOT EXISTS waypoints (
                      `name`       char(40)          default \'\',
                      `type`	   char(40)          default \'\',
                      `id`	       int(11)  NOT NULL auto_increment,
                      `lat`	       double 	         default \'0\',
                      `lon`	       double            default \'0\',
                      `comment`    char(160)         default \'\',
                      `wep`	   	   int(11)  NOT NULL default \'0\',
                      `macaddr`	   char(20)          default \'0\',
                      `nettype`	   int(11)  NOT NULL default \'0\',
                      `typenr`	   int(11) 	         default NULL,
                      PRIMARY KEY  (id)
                    ) TYPE=MyISAM;') or die;
    add_index('waypoints');



    # -----------------------------------------------------------------------------
    # Set Priviledges
    db_exec('grant select,insert,update,delete on geoinfo.* to gast@localhost identified by \'gast\'');
    db_exec('flush privileges;');


# -----------------------------------------------------------------------------
{ # Fill poi_type database
    my @poi_type_names
	= qw( unknown
	      accomodation
	      accomodation.camping_ground
	      accomodation.camping_place
	      accomodation.hotel
	      accomodation.motel
	      accomodation.youth_hostel
	      administration.admission_office
	      administration.guildhall
	      bank
	      bank.cash_dispenser
	      bank.cash_dispenser.ec
	      bank.ec-automat
	      bank.money_exchange
	      church.cementary
	      church.katholic
	      church.protestantic
	      church.synagoge
	      cithall
	      country.area
	      country.city
	      country.forrest
	      country.lake
	      country.mountain
	      country.pedestrian_zone
	      country.state
	      education
	      education.elentry_scool
	      education.gymnasium
	      education.hauptschule
	      education.kindergarten
	      education.nursery
	      education.university
	      education.vhs
	      entrance
	      fire_brigade
	      food.beergarden
	      food.cafe
	      food.delivery_service
	      food.disco
	      food.drinking_water
	      food.fastfood
	      food.fastfood.burger_king
	      food.fastfood.mc_donalds
	      food.pub
	      food.restaurant
	      health
	      health.ambulance
	      health.doctor
	      health.emergency
	      health.hospital
	      health.pharmacy
	      import_way.txt
	      information
	      police
	      postal.letter-box
	      postal.office
	      recreation
	      recreation.cinema
	      recreation.exhibition
	      recreation.fairground
	      recreation.geocache
	      recreation.gift_shop
	      recreation.guildhall
	      recreation.landmark
	      recreation.landmark.monument
	      recreation.landmark.museum
	      recreation.landmark.parc
	      recreation.model_aircrafts
	      recreation.national_park
	      recreation.night_club
	      recreation.opera
	      recreation.playground
	      recreation.sport
	      recreation.sport.bicycling
	      recreation.sport.golf_place
	      recreation.sport.horse_riding
	      recreation.sport.ice_scating
	      recreation.sport.minigolf_place
	      recreation.sport.motorbike
	      recreation.sport.rock_climbing
	      recreation.sport.skiing
	      recreation.sport.socker_place
	      recreation.sport.socker_stadion
	      recreation.sport.swimming
	      recreation.sport.tennis_place
	      recreation.swimming_area
	      recreation.theatre
	      recreation.zoo
	      recycling
	      recycling.old_glas
	      recycling.old_paper
	      recycling.wertstoffhof
	      restroom
	      shopping
	      shopping.do-it-yourself_store
	      shopping.flea_market
	      shopping.groceries
	      shopping.groceries.supermarcet
	      shopping.shopping_center
	      telephone
	      traffic.emergency_call
	      transport
	      transport.airport
	      transport.bus-stop
	      transport.car
	      transport.car.ferry
	      transport.car.flashing_traffic_light
	      transport.car.garage
	      transport.car.gas_station
	      transport.car.maut_station
	      transport.car.parking
	      transport.car.parking.handicaped
	      transport.car.parkinglot
	      transport.car.rest_area
	      transport.car_loading_terminal
	      transport.container_loading_terminal
	      transport.driver-guide_service
	      transport.garage
	      transport.interurban_train_stop
	      transport.parkride
	      transport.railroad_station
	      transport.taxi_stand
	      transport.traffic.construction
	      transport.traffic.speedtrap
	      transport.traffic.traffic_jam
	      transport.tram_stop
	      transport.underground_stop
	      w-lan.open
	      w-lan.public
	      w-lan.wep
	      );

    my $translate_de = { 
	'accomodation'          => 'Uebernachtung',
	'administration'        => 'Verwaltung',
	'admission_office'      => 'Zulassungsstele',
	'airport'               => 'Flugplatz',
	'ambulance'             => 'Ambulanz',
	'area'                  => "Gebiet",
	'bank'                  => 'Bank',
	'beergarden'            => 'Biergarten',
	'bicycling'             => "Fahrrad",
	'burger_king'           => 'Burger_King',
	'bus-stop'              => 'Bus-Haltestelle',
	'cafe'                  => 'cafe',
	'camping_ground'        => 'Zeltplatz',
	'camping_place'         => 'Camping_Platz',
	'car'                   => 'Auto',
	'car_loading_terminal'  => 'Auto_Verlade_Bahnhof',
	'cash_dispenser'        => 'Geldautomat',
	'cementary'             => 'Friedhof',
	'church'                => 'Kirche',
	'cinema'                => 'Kino',
	'cithall'               => 'Stadthalle',
	'city'                  => 'Stadt',
	'construction'          => 'Baustelle',
	'container_loading_terminal' => 'Gueter_Verlade_Bahnhof',
	'country'               => "landschaft",
	'delivery_service'      => 'Lieferservice',
	'disco'                 => 'disco',
	'do-it-yourself_store'  => 'Baumarkt',
	'doctor'                => 'Arzt',
	'drinking_water'        => "Trinkwasser",
	'driver-guide_service'  => 'Lotsendienst',
	'ec'                    => 'EC',
	'ec-automat'            => 'EC-Automat',
	'education'             => 'Ausbildung',
	'elentry_scool'         => 'GrundSchule',
	'emergency'             => 'Notaufnahme',
	'emergency_call'        => 'Notruf_saeule',
	'entrance'              => 'Eingang',
	'exhibition'            => 'Messe',
	'fairground'            => 'Rummelplatz',
	'fastfood'              => 'Schnellrestaurant',
	'ferry'                 => 'Faehre',
	'fire_brigade'          => 'Feuerwehr',
	'flashing_traffic_light' => 'Blitzampel',
	'flea_market'           => 'Flohmarkt',
	'food'                  => 'Essen',
	'forrest'               => "Wald",
	'garage'                => 'Werkstatt',
	'gas_station'           => 'Tankstelle',
	'geocache'              => 'Geocache',
	'gift_shop'             => 'Gift_Shop',
	'golf_place'            => 'Golfplatz',
	'groceries'             => 'Lebensmittel',
	'guildhall'             => 'Rathaus',
	'guildhall'             => 'Veranstaltungshalle',
	'gymnasium'             => 'Gymnasium',
	'handicaped'            => "Behinderte",
	'hauptschule'           => 'HauptSchule',
	'health'                => 'Gesundheit',
	'horse_riding'          => 'Reiten',
	'hospital'              => 'Krankenhaus',
	'hotel'                 => 'Hotel',
	'ice_scating'           => "Eislaufen",
	'import_way'            => 'import_way',
	'information'           => 'Information',
	'interurban_train_stop' => 'S-Bahn-Haltestelle',
	'katholic'              => 'Katholisch',
	'kindergarten'          => 'Kindergarten',
	'lake'                  => "See",
	'landmark'              => 'Sehenswuerdigkeit',
	'letter-box'            => 'Briefkasten',
	'maut_station'          => 'Mautstation',
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
	'old_glas'              => 'Altglas',
	'old_paper'             => 'Altpapier',
	'open'                  => 'Offen',
	'opera'                 => 'Oper',
	'parc'                  => "Park",
	'parking'               => "Parken",
	'parkinglot'            => 'Parkplatz',
	'parkride'              => 'ParkRide',
	'pedestrian_zone'       => 'Fussgaenger_Zone',
	'pharmacy'              => 'Apotheke',
	'playground'            => 'Spielplatz',
	'police'                => 'Polizei',
	'postal'                => 'Post',
	'postoffice'            => 'Post',
	'protestantic'          => "evangelisch",
	'pub'                   => 'Kneipe',
	'public'                => 'oeffentlich',
	'publoic'               => 'Oeffentlich',
	'railroad_station'      => 'Bahnhof',
	'recreation'            => 'Freizeit',
	'recycling'             => 'Recycling',
	'rest_area'             => 'Raststaette',
	'restaurant'            => 'Gaststaette',
	'restaurant'            => 'Restaurant',
	'restroom'              => 'WC',
	'rock_climbing'         => "Fels klettern",
	'shopping'              => 'Einkaufen',
	'shopping_center'       => 'Einkaufszentrum',
	'skiing'                => 'Skifahren',
	'socker_place'          => 'Fussballplatz',
	'socker_stadion'        => 'Fussball_Stadion',
	'speedtrap'             => 'Radarfalle',
	'sport'                 => 'Sport',
	'state'                 => "Staat",
	'supermarcet'           => 'Supermarkt',
	'swimming'              => "schwimmen",
	'swimming_area'         => 'Schwimmbad',
	'synagoge'              => 'Synagoge',
	'taxi_stand'            => 'Taxi-Stand',
	'telephone'             => 'Telefon-Zelle',
	'tennis_place'          => 'Tennisplatz',
	'theatre'               => 'Theater',
	'traffic'               => 'Verkehr',
	'traffic_jam'           => 'Stau',
	'tram_stop'             => 'Strassen-Bahn_Haltestelle',
	'transport'             => 'Transport',
	'txt'                   => 'txt',
	'underground_stop'      => 'U-Bahn_Haltestelle',
	'university'            => 'UNI',
	'unknown'               => 'unknown',
	'vhs'                   => 'VHS',
	'w-lan'                 => 'W-LAN',
	'wep'                   => 'WEP',
	'wertstoffhof'          => 'Wertstoffhof',
	'youth_hostel'          => 'Jugend_Herberge',
	'zoo'                   => 'Zoo',
    };

    my $i=1;
    my $unknown_translations;
    
    # for debug purpose
    db_exec("TRUNCATE TABLE `poi_type`;");


    for my $name  ( @poi_type_names ) {

	# Translate the entries
	my $name_de ='';
	#print Dumper(\$translate_de);
	for my $part ( split(/\./,$name)){
	    if ( ! defined( $translate_de->{$part}) ) {
#				$translate_de->{$part} = lc($part);
		$unknown_translations .= sprintf("	%-23s => \"".lc($part)."\",\n","'".$part."'");
	    }
	    $name_de .= ".$translate_de->{$part}";
	}
	$name_de    =~ s/^\.//;
	$name_de    =~ s/_/ /g;
	$name =~ s/_/ /g;
	

	# Icon
	my $icons = {
	    '1' 			     => 'ppl1',
	    '2' 			     => 'ppl2',
	    '3' 			     => 'ppl3',
	    '4' 			     => 'ppl4',
	    '5' 			     => 'ppl5',
	    'accomodation' 	             => "hotel",      ,
	    'circle' 			     => 'ppl',
	    'food.cafe' 		     => "cafe",
	    'food.fastfood.burger_king'      => "burgerking",
	    'food.fastfood.mc_donalds' 	     => "mcdonalds",
	    'health.pharmacy'                => "pharmacy",
	    'landmark' 		             => "monument",
	    'bank.cash_dispenser.ec'         => "ec",
	    'transport.car.parkinglot'       => "parking",
	    'transport.car.parking.handicaped' => "handicaped",
	    'mine' 			     => 'mine',
	    'open' 			     => 'open',
	    'recreation.geocache' 	     => "geocache",
	    'recreation.night_club' 	     => "girls",
	    'recreation.sport.golf_place'    => "golf",
	    'recreation.sport.skiing' 	     => 'skiing',
	    'shopping'                       => "shop",
	    'summit'                         => 'summit',
	    'transport.airport' 	     => "airport",
	    'transport.car.gas_station'      => "fuel",
	    'transport.car.parkinglot'       => "parking",
	    'transport.car.restarea' 	     => 'rest',
	    'transport.traffic.construction' => "speedtrap",
	    'transport.traffic.speedtrap'    => "speedtrap",
	    'transport.traffic.speedtrap'    => "speedtrap",
	    'transport.traffic.traffic_jam'  => "speedtrap",
	    'w-lan.open' 	             => "WLAN",
	    'w-lan.public' 	             => 'tower',		
	    'w-lan.wep' 	             => "WLAN-WEP",
	};
	
	my $icon;
	my $icon_name = "$name.png";
	while ( $icon_name =~ s/\.[^\.]+$// ){
	    if (  defined ( $icons->{$icon_name} )) {
		$icon = $icons->{$icon_name};
		last;
	    }
	}
#		$icon ||=  "$name";
	$icon ||=  "unknown";
	$icon =~ s/ /_/g;

	my $description     = $name;    $description    =~ s/\./ /g;
	my $description_de  = $name_de; $description_de =~ s/\./ /g;
	db_exec("DELETE FROM `poi_type` WHERE poi_type_id = $i ;");
	db_exec("INSERT INTO `poi_type` ".
		"       (poi_type_id, name,name_de, symbol, description,description_de ) ".
		"VALUES ($i,'$name','$name_de','$icon.png','$description','$description_de');") or die;
	$i++;
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

	db_exec("DELETE FROM `source` WHERE source.name = '$name';");
	my $source_hash = {
	    'source.url'     => "http://www.evl.uic.edu/pape/data/WDB/WDB-text.tar.gz",
	    'source.name'    => $name,
	    'source.comment' => "GeoData for $coutry2name->{$country}($country)",
	    'source.licence' => ""
	    };
	POI::DBFuncs::insert_hash("source", $source_hash);
    }

    for my $source  ( qw( import_way.txt
			  default_values
			  ) ) {    
	my $name ="$source";
	$name =~ s/_/ /g;

	db_exec("DELETE FROM `source` WHERE source.name = '$name';");
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
    my $t = "waypoints";
    my $wp_defaults = { "$t.wep"      => 0 ,
			"$t.nettype"  => '',
			"$t.type"     => 'Developer'
			};
    for my $loc (
	     { "$t.name" => "Joerg", "$t.lat" => '48.1', "$t.lon" => '11.7'} 
		 ) {
	my $delete_query=sprintf("DELETE FROM $t WHERE name = '%s' AND type = '%s' ",$loc->{"$t.name"},$wp_defaults->{"$t.type"});
	db_exec( $delete_query);
	insert_hash($t, $wp_defaults, $loc );
    }
}

{ # Import Points from way*.txt into DB

    my $type2poi_type = { 
	AIRPORT    => "transport.airport" ,
	BURGERKING => "food.fastfood.burger king" ,
	CAFE       => "food.cafe" ,
	GASSTATION => "transport.car.gas station" ,
	GEOCACHE   => "recreation.geocache" ,
	GOLF       => "recreation.sport.golf place" ,
	HOTEL      => "accomodation" ,
	MCDONALDS  => "food.fastfood.mc donalds" ,
	MONU       => "recreation.landmark" ,
	NIGHTCLUB  => "recreation.night club" ,
	SHOP       => "shopping" ,
	SPEEDTRAP  => "transport.traffic.construction" ,
	WLAN       => "w-lan.open" ,
	'WLAN-WEP' => "w-lan.wep" ,
	'Automatic_key' => "unknown" ,
	'Reiten'   => 'recreation.sport.horse riding',
    };

    for my $k ( keys %{$type2poi_type} ) {
	if ( ! poi_type_name2id($type2poi_type->{$k}) ) {
	    print "Undefined Type $k => $type2poi_type->{$k}\n";
	} else {
	    #print " Type $k => $type2poi_type->{$k} => ".poi_type_name2id($type2poi_type->{$k})."\n";
	}
    }

    for my $file_name ( glob('~/.gpsdrive/way*.txt') ) {
	my ($file_type ) = ( $file_name =~ m/way[-_]?(.*)\.txt$/);
	my $default_poi_type = 'unknown';
	$default_poi_type = $file_type if poi_type_name2id($file_type);
	$default_poi_type = $type2poi_type->{$file_type} if poi_type_name2id($type2poi_type->{$file_type});
	print "Default Type: $default_poi_type\n" if $default_poi_type ne "unknown";
	next if $file_name =~ m,/way-SQLRESULT.txt$,;
	print "Reading $file_name\n";

	my $source_id = source_name2id("import way.txt");

	my $fh = IO::File->new($file_name);
	my $count =0;
	while ( my $line = $fh->getline() ) {
	    my @columns = ('')x10;
	    @columns = split(/\t/,$line);
	    next unless $columns[0] && $columns[1] && $columns[2];
	    $columns[0] =~ s/'/\\'/g;
	    my $type = "unknown";
	    $type =  $columns[3];
	    $type ||= $default_poi_type;
	    $type = $default_poi_type if $type eq"-";
	    my $wep = 0;
	    $wep ='1' if $type eq "WLAN-WEP";
	    $type = "WLAN" if $columns[0] =~ m/\d\d\:\d\d\:\d\d\:\d\d\:\d\d\:\d\d/;

	    my $poi_type_id = poi_type_name2id($type2poi_type->{"$type"});
	    $poi_type_id ||= poi_type_name2id("$type");
	    $poi_type_id || printf "Unknown Waypoint Type '$type' in %s\n".join(",",@columns);
	    $poi_type_id ||= poi_type_name2id("import way.txt");

	    for my $t ( qw(waypoints poi)) {
		my $wp = { "waypoints.wep"      => 0 ,
			   "waypoints.nettype"  => '',
			   "waypoints.type"     => 'import_way.txt',
			   "poi.poi_type_id"    => $poi_type_id,
			   "poi.source_id"      => $source_id,
			   "poi.scale_min"      => 1,
			   "poi.scale_max"      => 5000,
			   "poi.last_modified"  => localtime(time()),
			   "$t.name"            => $columns[0],
			   "$t.lat"             => $columns[1],
			   "$t.lon"             => $columns[2],
			   "$t.type"            => $type,
			   "$t.wep"             => $wep,
		       };
		
		db_exec("DELETE FROM $t ".
			"WHERE $t.name = '$columns[0]' ".
			"AND   $t.lat  = '$columns[1]' ".
			"AND   $t.lon  = '$columns[2]' ");
		
		insert_hash($t,	$wp );
	    }
	    $count++;
	}
	$fh->close();
	print "Inserted $count Entries from $file_name\n";
    }
}

# -----------------------------------------------------------------------------
{   # Fill streets_type database
    my $i=1;
    my $lang;
    my $name;
    my $color;

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
	    db_exec("DELETE FROM `streets_type` WHERE streets_type_id = '$i' ;");
	    db_exec("INSERT INTO `streets_type` ".
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

	db_exec("DELETE FROM `streets_type` WHERE streets_type_id = '$i';");
	db_exec("INSERT INTO `streets_type` ".
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

    my $source_name = "default values";
    my $source_id = source_name2id($source_name);
    debug("$source_name --> '$source_id'\n");
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
	insert_hash("streets",$s4db);
    }

}

print "Creation completed\n";

}
# -----------------------------------------------------------------------------

1;
