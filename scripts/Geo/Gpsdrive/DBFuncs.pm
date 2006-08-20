# Database Functions for poi.pl

package Geo::Gpsdrive::DBFuncs;

use strict;
use warnings;

use POSIX qw(strftime);
use Time::Local;
use DBI;
use Geo::Gpsdrive::Utils;
use Data::Dumper;
use IO::File;

$|= 1;                          # Autoflush

BEGIN {
    use Exporter   ();
    our ($VERSION, @ISA, @EXPORT, @EXPORT_OK, %EXPORT_TAGS);

    # set the version for version checking
    $VERSION     = 1.00;
    # if using RCS/CVS, this may be preferred
    $VERSION = sprintf "%d.%03d", q$Revision$ =~ /(\d+)/g;

    @ISA         = qw(Exporter);
    @EXPORT = qw( &poi_type_names &poi_type_list  &poi_type_name2id &poi_type_id2name
		  &streets_type_names &streets_type_list  &streets_type_name2id
		  &db_disconnect 
		  &add_poi &add_poi_multi
		  &street_segments_add
		  &poi_list
		  &column_names
		  &source_name2id
		  &insert_hash
		  &delete_all_from_source
		  &enable_keys &disable_keys);
    %EXPORT_TAGS = ( );     # eg: TAG => [ qw!name1 name2! ],
    # your exported package globals go here,
    # as well as any optionally exported functions
    @EXPORT_OK   = qw();

}
our @EXPORT_OK;


END { } 


sub street_segments_add($);

# -----------------------------------------------------------------------------
# switch off updating of index
sub disable_keys($){    
    my $table = shift;
    print "disable_keys($table)\n" if $verbose;
    db_exec("ALTER TABLE  $table DISABLE KEYS;");
}

# -----------------------------------------------------------------------------
# switch on updating of index
sub enable_keys($){
    my $table = shift;
    print "enable_keys($table)\n" if $verbose;
    db_exec("ALTER TABLE  $table ENABLE KEYS;");
}

# -----------------------------------------------------------------------------
# retrieve Column Names for desired table
my %COLUMN_NAMES;
sub column_names($){
    my $table = shift;
    my @col;

    # look for cached result
    if ( defined($COLUMN_NAMES{$table} ) ){
	return @{$COLUMN_NAMES{$table}};
    }

    my $dbh = db_connect();
    my $query = "SHOW COLUMNS FROM $main::GPSDRIVE_DB_NAME.$table;";

    my $sth=$dbh->prepare($query) or die $dbh->errstr;
    $sth->execute()               or die $sth->errstr;

    while ( my $array_ref = $sth->fetchrow_arrayref() ) {
	push ( @col ,$array_ref->[0] );
    }
    $COLUMN_NAMES{$table}=\@col;
    return @col;
}


# -----------------------------------------------------------------------------
# get all indix info for $table
# RETURNS:
#    	    $result->{$name}->{...}
# where $name is name of index
sub show_index($) {
    my $table  = shift;

    my $query = "SHOW INDEX FROM $table;";
    my $dbh = db_connect();
    my $sth=$dbh->prepare($query) or die $dbh->errstr;
    $sth->execute()               or die $sth->errstr;

    my $result ={};
    foreach my $row ( @{ $sth->fetchall_arrayref({}) } ) {
	my $name =$row->{'Key_name'}; 
	my $seq_index = $row->{'Seq_in_index'};
	if ( $seq_index ) {
	    $result->{$name}->{Columns}->[$seq_index-1] = $row->{'Column_name'};
	    for my $k ( keys %{$row} ) {
		if ( ! exists($result->{$name} ) ||
		     ! exists($result->{$name}->{$k}) ) {
		    $result->{$name}->{$k} = $row->{$k};
		} elsif ( !defined($result->{$name}->{$k}) && !defined($row->{$k}) ) {
		    # Avoid undef warnings
		} elsif ( $result->{$name}->{$k} ne $row->{$k} ) {
		    $result->{$name}->{$k} .= ",$row->{$k}";
		}
	    }
	} else {
	    $result->{$name} = $row;
	}
    }
    
    return $result;
}

# -----------------------------------------------------------------------------
# insert hash into database
my $last_insert_table_name='';
my @fields;
my $insert_hash_sth;
sub insert_hash($$;$) {
    my $table = shift;
    my $field_values = shift;

    # read  multiple hashreference and override Hash Values
    # of field_values
    while ( my $h = shift ) {
#	print "Adding ".Dumper($h);
	map { $field_values->{$_} = $h->{$_} } sort keys %{$h};
    }

    $field_values->{"$table.last_update"} ||= time();

    # get Table info and create sql query if 
    # the table differs from the last insert Statement
    if ( $last_insert_table_name ne $table ) {
	# get column names of table
	@fields = map { "$table.$_" } column_names($table);
	
	my $sql = sprintf ( "insert into %s (%s) values (%s)",
			    $table, 
			    join(',', @fields),
			    join(",", ("?") x scalar(@fields))
			    );
	
	my $dbh = db_connect();
	#print "insert_hash($table, ".Dumper(\$field_values).")\n";
	#print "$sql\n";
	#print "insert_hash($table, ".join(",",map { $_ || '' } @values).")\n";
	$insert_hash_sth = $dbh->prepare_cached($sql);
	$last_insert_table_name = $table;
    }

    # get the values into the right order
    my @values = @{$field_values}{@fields};

    my $res = $insert_hash_sth->execute(@values);
    if ( ! $res ) {
	warn "Error while inserting Hash ".Dumper($field_values)." into table '$table'\n";
	$insert_hash_sth->errstr;
    }
    
    return $res;
}


#############################################################################
# All necessary information for connecting the DB
# these are:  host,  user and passwort; the db is always $main::GPSDRIVE_DB_NAME
my $dbh;
sub db_connect() {
    my $db           = $main::GPSDRIVE_DB_NAME;
    my $opt_user     = $main::db_user;
    my $opt_password = $main::db_password;
    my $host         = $main::db_host;
    #$host         = 'mysql_socket=/home/tweety/.gpsdrive/mysql/mysqld.sock';

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
    if ( $main::no_delete ){
	print "Keeping old entries for '$source_name'\n" if $verbose;
	return;
    }

    print "Delete all from '$source_name'\n" if $verbose;
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
    print "Deleted all from '$source_name'\n" if $verbose>3;
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
# TODO: if we get a Hash; create the source entry if not already existent
my $poi_type_id_cache;
my $poi_type_id_2_name_cache;
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
	    $poi_type_id_2_name_cache->{$poi_type_id} = $type_name;
	} else {
	    # Nicht gefunden
	    $poi_type_id=0;
	}
	$sth->finish;
    }

    debug("Type: $type_name -> $poi_type_id")
	if $verbose;

    return $poi_type_id;
}

# ------------------------------------------------------------------
sub poi_type_id2name($){
    my $poi_type_id=shift;
    
    my $poi_type_name='';
    return '' unless $poi_type_id;
    
    if ( defined $poi_type_id_2_name_cache->{$poi_type_id} ) {
	$poi_type_name = $poi_type_id_2_name_cache->{$poi_type_id};
    } else {
	my $dbh = db_connect();
	my $query = "SELECT poi_type.name FROM poi_type WHERE poi_type.pi_type_id = '$poi_type_id' LIMIT 1";

	my $sth=$dbh->prepare($query) or die $dbh->errstr;
	$sth->execute()               or die $sth->errstr;

	my $array_ref = $sth->fetchrow_arrayref();
	if ( $array_ref ) {
	    $poi_type_name = $array_ref->[0];
	    $poi_type_id_cache->{$poi_type_name} = $poi_type_id;
	    $poi_type_id_2_name_cache->{$poi_type_id} = $poi_type_name;
	} else {
	    # Nicht gefunden
	    $poi_type_name='';
	}
	$sth->finish;
    }

    debug("Type: $poi_type_id --> $poi_type_name");

    return $poi_type_name;
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
# TODO: if we get a Hash; create the source entry if not already existent
my $streets_type_id_cache;
sub streets_type_name2id($){
    my $type_name = shift ||'';

    return 0 unless $type_name;

    my $streets_type_id;

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

    if ( ! $streets_type_id ) {
	warn "No Type named '$type_name' found in streets_type\n";
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
    $segment4db->{'streets.scale_min'}       ||= 1;
    $segment4db->{'streets.scale_max'}       ||= 9999999999999999;
    insert_hash("streets",$segment4db);
}


#############################################################################
# Add a list of street segments into streets-DB
# 
sub street_segments_add($){
    my $data = shift;

    my $segment4db = {};

    my @columns = column_names("streets");
    map { 
	$segment4db->{"streets.$_"} = 
	    ( $data->{"streets.$_"} || $data->{$_} || $data->{lc($_)}) 
	} @columns;

    # ---------------------- ADDRESS
    $segment4db->{'streets.address_id'}        ||= 0;

    if ( ! $segment4db->{'streets.streets_type_id'} ) {
	print "ERROR: street_segments_add: Error streets.streets_type_id missing:\n";
	print Dumper(\$data);
	return;
    }

    # ---------------------- STREETS
    $segment4db->{'streets.last_modified'}     ||= time();
    $segment4db->{'streets.scale_min'}         ||= 1;
    $segment4db->{'streets.scale_max'}         ||= 100000000000;

    $segment4db->{'streets.lat1'}=0;

    my $count = scalar @{$data->{segments}};
    debug("Writing $count Segments") if $count ;

    my ($lat2,$lon2,$alt2) = (0,0,0);
    #my $sql ="insert into streets (streets.streets_id,streets.name,streets.streets_type_id,streets.lat1,streets.lon1,streets.alt1,streets.lat2,streets.lon2,streets.alt2,streets.proximity,streets.comment,streets.scale_min,streets.scale_max,streets.last_modified,streets.source_id) values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";
    #my $sth = $dbh->prepare_cached($sql);
    my $segment_nr=0;
    for my $segment ( @{$data->{segments}} ){
	my $lat1 = $segment4db->{'streets.lat1'} = $segment4db->{'streets.lat2'};
	my $lon1 = $segment4db->{'streets.lon1'} = $segment4db->{'streets.lon2'};
	my $alt1 = $segment4db->{'streets.alt1'} = $segment4db->{'streets.alt2'};

	if ( ref($segment) eq "ARRAY" ) {
	    $lat2 = $segment->[0];
	    $lon2 = $segment->[1];
	    $alt2 = $segment->[2];
	} elsif ( defined($segment->{'lat1'} ))  {
	    $segment4db->{'streets.lat1'} = $lat1 = $segment->{'lat1'};
	    $segment4db->{'streets.lon1'} = $lon1 = $segment->{'lon1'};
	    $segment4db->{'streets.alt1'} = $alt1 = $segment->{'alt1'};

	    $lat2 = $segment->{'lat2'};
	    $lon2 = $segment->{'lon2'};
	    $alt2 = $segment->{'alt2'};

	} else {
	    $lat2 = $segment->{'lat'};
	    $lon2 = $segment->{'lon'};
	    $alt2 = $segment->{'alt'};
	};

	$segment4db->{'streets.lat2'} = $lat2;
	$segment4db->{'streets.lon2'} = $lon2;
	$segment4db->{'streets.alt2'} = $alt2;

	next unless $segment4db->{'streets.lat1'}; # skip first entry

#	my $d_lat=abs($lat1-$lat2);
#	my $d_lon=abs($lon1-$lon2);
#	my $dist = $d_lat+$d_lon;
#	print "Dist: $dist	($lat1,$lon1),($lat2,lon2) $segment4db->{'streets.name'}\n"
#	    if $dist > 0.05;;
	$segment4db->{'streets.comment'}         = "Segment: $segment_nr";
	$segment4db->{'streets.comment'}         = $segment->{'comment'}    if defined $segment->{'comment'};
#	$segment4db->{'streets.name'}          ||= $segment->{'name'}       if defined $segment->{'name'};
	$segment4db->{'streets.name'}            = $data->{name}            || $segment->{name};
	$segment4db->{'streets.streets_type_id'} = $data->{'streets.streets_type_id'} || $data->{'streets_type_id'} || $segment->{streets_type_id};
#	$segment4db->{'streets.source_id'}       = $data->{source_id}       || $segment->{source_id};
#	$segment4db->{'streets.scale_min'}       = $data->{scale_min}       || $segment->{scale_min}||1;
#	$segment4db->{'streets.scale_max'}       = $data->{scale_max}       || $segment->{scale_max}||10000000000;

	#print "segment4db:".Dumper(\$segment4db);
	insert_hash("streets",$segment4db);
	$segment_nr++;
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
    my $keys  = shift || $name || '';
    
    my $indices = show_index($table);
    debug( "If not exist; adding Index $table.$name: '$keys'");
    if ( $keys && $keys =~ m/\,/ ) { # Multi Key
	my $ist ='';
	if ( exists $indices->{$name}->{Columns} &&
	     @{$indices->{$name}->{Columns}} ) {
	    $ist = join('`,`',@{$indices->{$name}->{Columns}});
	    if ( $ist eq $keys ) { # exists and correct
		return;
	    } else {
		print "Dropping Index: $table.$name\n";
		db_exec("ALTER TABLE `$table` DROP INDEX `$name`;");
	    }
	}
    } elsif ( defined $indices->{$name}->{'Column_name'} && 
	      $indices->{$name}->{'Column_name'} eq $keys ) {
	return;
    } elsif ( defined $indices->{$name}->{'Column_name'} ) {
	print "Droping Index: $table.$name\n";
	db_exec("ALTER TABLE `$table` DROP INDEX `$name`;");
    }

    debug( "Adding Index: $table.$name: `$keys`");
    db_exec("ALTER TABLE `$table` ADD INDEX `$name` ( `$keys` );");
}

# -----------------------------------------------------------------------------
# create known indices for given table
sub add_index($){
    my $table = shift;

    if ( $table eq "poi" ){
	for my $key ( qw( last_modified name lat lon ) ){
	    add_if_not_exist_index($table,$key);
	}
	add_if_not_exist_index( $table,'combi1','lat`,`lon`,`poi_type_id');
    } elsif ( $table eq "streets" ){
	for my $key ( qw( last_modified name lat1 lon1 lat2 lon2 ) ){
	    add_if_not_exist_index($table,$key);
	}
	add_if_not_exist_index($table,'combi1','lat1`,`lon1`,`lat2`,`lon2`,`streets_type_id');
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

    $create_statement="CREATE DATABASE IF NOT EXISTS $main::GPSDRIVE_DB_NAME;";
    my $drh = DBI->install_driver("mysql");
    my $rc = $drh->func('createdb', $main::GPSDRIVE_DB_NAME, $main::db_host, 
			$main::db_user,$main::db_password, 'admin');
    die "$@" if $rc;

    $dbh = db_connect();
    $sth = $dbh->prepare($create_statement);
    $sth->execute()
	or die $sth->errstr;
    
    # ------- Address
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

    # ------- POI
    db_exec('CREATE TABLE IF NOT EXISTS `poi_type` (
                      `poi_type_id` int(11)      NOT NULL auto_increment,
                      `name`        varchar(80)  NOT NULL default \'\',
                      `scale_min`   int(12)      NOT NULL default \'1\',
                      `scale_max`   int(12)      NOT NULL default \'0\',
                      `description` varchar(160)     NULL default \'\',
                      PRIMARY KEY  (`poi_type_id`)
                    ) TYPE=MyISAM;') or die;
    add_index('poi_type');


    db_exec('CREATE TABLE IF NOT EXISTS `poi` (
                      `poi_id`        int(11)      NOT NULL auto_increment,
                      `name`          varchar(80)           default NULL,
                      `poi_type_id`   int(11)      NOT NULL default \'0\',
                      `lat`           double                default \'0\',
                      `lon`           double                default \'0\',
                      `alt`           double                default \'0\',
                      `proximity`     float                 default \'0\',
                      `comment`       varchar(255)          default NULL,
                      `last_modified` date         NOT NULL default \'0000-00-00\',
                      `url`           varchar(160)     NULL ,
                      `address_id`    int(11)               default \'0\',
                      `source_id`     int(11)      NOT NULL default \'0\',
                      PRIMARY KEY  (`poi_id`)
                    ) TYPE=MyISAM;') or die;
    add_index('poi');


    # ------- Streets
    db_exec('CREATE TABLE IF NOT EXISTS `streets` (
                      `streets_id`      int(11)      NOT NULL auto_increment,
                      `name`            varchar(80)           default NULL,
                      `streets_type_id` int(11)      NOT NULL default \'0\',
                      `lat1`            double       NOT NULL default \'0\',
                      `lon1`            double       NOT NULL default \'0\',
                      `alt1`            double                default \'0\',
                      `lat2`            double       NOT NULL default \'0\',
                      `lon2`            double       NOT NULL default \'0\',
                      `alt2`            double                default \'0\',
                      `proximity`       float                 default \'0\',
                      `comment`         varchar(255)          default NULL,
                      `scale_min`       int(12)      NOT NULL default \'1\',
                      `scale_max`       int(12)      NOT NULL default \'0\',
                      `last_modified`   date         NOT NULL default \'0000-00-00\',
                      `source_id`       int(11)      NOT NULL default \'0\',
                      PRIMARY KEY  (`streets_id`)
                    ) TYPE=MyISAM;') or die;
    add_index('streets');

    db_exec('CREATE TABLE IF NOT EXISTS `streets_type` (
                      `streets_type_id` int(11)      NOT NULL auto_increment,
                      `name`            varchar(80)  NOT NULL default \'\',
                      `scale_min`       int(12)      NOT NULL default \'1\',
                      `scale_max`       int(12)      NOT NULL default \'0\',
                      `description`     varchar(160)     NULL default \'\',
                      `color`           varchar(20)      NULL default \'\',
                      `color_bg`        varchar(20)      NULL default \'\',
                      `width`           int(2)           NULL default \'1\',
                      `width_bg`        int(2)           NULL default \'2\',
                      `linetype`        varchar(80)      NULL default \'\',
                      PRIMARY KEY  (`streets_type_id`)
                    ) TYPE=MyISAM;') or die;
    add_index('streets_type');

    # ------- Tracks
    db_exec('CREATE TABLE IF NOT EXISTS `tracks_type` (
                      `track_type_id` int(11)    NOT NULL auto_increment,
                      `name`        varchar(80)  NOT NULL default \'\',
                      `symbol`      varchar(160)     NULL default \'\',
                      `description` varchar(160)     NULL default \'\',
                      `color`           varchar(20)  NULL default \'\',
                      `color_bg`        varchar(20)  NULL default \'\',
                      `width`           int(2)       NULL default \'1\',
                      `width_bg`        int(2)       NULL default \'2\',
                      `linetype`        varchar(80)  NULL default \'\',
'. #                  vvvvvvvvvvv For later use
'                     `scale_min`     int(12)      NOT NULL default \'1\',
                      `scale_max`     int(12)      NOT NULL default \'0\',
'. #                  ^^^^^^^^^^^ For later use
'                     PRIMARY KEY  (`track_type_id`)
                    ) TYPE=MyISAM;') or die;
    add_index('tracks_type');

    db_exec('CREATE TABLE IF NOT EXISTS `tracks` (
                      `track_id`       int(11)      NOT NULL  auto_increment,
                      `name`           varchar(80)            default NULL,
                      `track_type_id`  int(11)      NOT NULL  default \'0\',
                      `lat1`            double                default \'0\',
                      `lon1`            double                default \'0\',
                      `alt1`            double                default \'0\',
                      `time1`           date                  default \'0000-00-00\',
                      `lat2`            double                default \'0\',
                      `lon2`            double                default \'0\',
                      `alt2`            double                default \'0\',
                      `time2`           date                  default \'0000-00-00\',
                      `speed`          double                 default \'0\',
                      `direction`      double                 default \'0\',
                      `acuracy`        float                  default \'0\',
                      `time_delta`     float                  default \'0\',
                      `source_id`      int(11)      NOT NULL  default \'0\',
                      PRIMARY KEY  (`track_id`)
                    ) TYPE=MyISAM;') or die;
    add_index('track');

    # ------- Source
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

    # --------- Waypoints: For compatibility to old DB Structure
    db_exec('CREATE TABLE  IF NOT EXISTS waypoints (
                      `name`       char(40)          default \'\',
                      `type`	   char(40)          default \'\',
                      `id`	   int(11)  NOT NULL auto_increment,
                      `lat`	   double 	     default \'0\',
                      `lon`	   double            default \'0\',
                      `comment`    char(160)         default \'\',
                      `wep`	   int(11)  NOT NULL default \'0\',
                      `macaddr`	   char(20)          default \'0\',
                      `nettype`	   int(11)  NOT NULL default \'0\',
                      `typenr`	   int(11) 	         default NULL,
                      PRIMARY KEY  (id)
                    ) TYPE=MyISAM;') or die;
    add_index('waypoints');

    # -------- traffic: For Traffic Information
    db_exec('CREATE TABLE IF NOT EXISTS traffic (
      			`id`        int(11)      NOT NULL auto_increment ,
			`status`    int(11)               default NULL ,
			`street`    varchar(40)           default NULL ,
			`descshort` varchar(100)          default NULL ,
			`desclong`  text         NOT NULL ,
			`future`    int(11)      NOT NULL default \'0\',
			`time`      time                  default \'00:00:00\',
			`timestamp` timestamp    NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
		        PRIMARY KEY  (`id`)
		      ) ENGINE=MyISAM DEFAULT CHARSET=latin1;') or die;
    add_index('traffic');


    # -----------------------------------------------------------------------------
    # Set Privileges
    # TODO: Split priviledges
    db_exec("grant select,insert,update,delete,lock tables on $main::GPSDRIVE_DB_NAME.* to gast\@localhost identified by \'gast\'");
    db_exec('flush privileges;');

    print "!!! WARNING: Created a user gast with password gast\n";
    print "!!! WARNING: this might be a security issue if you have your mysql \n";
    print "!!! WARNING: database accessible from outside of your computer\n";
    print "\n";

    print "Creation completed\n";

}
# -----------------------------------------------------------------------------

1;
