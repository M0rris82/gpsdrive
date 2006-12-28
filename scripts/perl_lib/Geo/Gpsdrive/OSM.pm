# Import Data from http://openstreetmap.org/

use strict;
use warnings;

use Storable ();
package Geo::Gpsdrive::OSM;

sub min($$){
    my $a = shift;
    my $b = shift;
    return $b if ! defined $a;
    return $a if ! defined $b;
    return $a<$b?$a:$b;
}
sub max($$){
    my $a = shift;
    my $b = shift;
    return $b if ! defined $a;
    return $a if ! defined $b;
    return $a>$b?$a:$b;
}

use strict;
use warnings;

use IO::File;
use File::Path;
use Geo::Gpsdrive::DBFuncs;
use Geo::Gpsdrive::Utils;
use Geo::OSM::Planet;
use Data::Dumper;
use XML::Parser;
use Geo::Filter::Area;
use Geo::OSM::Planet;
use Utils::Debug;
use Utils::File;
use Utils::LWP::Utils;
use Utils::Math;

our $OSM_polite  = 10; # Wait n times as long as the request lasted
my $osm_nodes    = {};
my $osm_segments = {};
my $osm_ways     = {};
my $osm_stats    = {};
my $osm_obj      = undef; # OSM Object currently read
our $AREA_FILTER;
our $PARSING_START_TIME=0;
our $PARSING_DISPLAY_TIME=0;
our (%Stats);


# --------------------------------------------
sub display_status($){
    my $mode = shift;
    return unless $VERBOSE || $DEBUG ;
    return unless time()-$PARSING_DISPLAY_TIME >2;

    $PARSING_DISPLAY_TIME= time();
    print STDERR "\r";
    #print STDERR "$mode(".$AREA_FILTER->name()."): ";

    print STDERR time_estimate($PARSING_START_TIME,
			       $Stats{"elem read"},
			       estimated_max_count("elem"));
    
    print STDERR mem_usage();
    print STDERR "\r";

}
###########################################
sub obj_compare($$){
    my $obj1 = shift;
    my $obj2 = shift;
    for my $k ( keys %{$obj1},keys %{$obj2} ){
	# ....
    }
}

sub node_ {
    $osm_obj = undef
}
sub node {
    my($p, $tag, %attrs) = @_;  
    my $id = delete $attrs{id};
    $osm_obj = {};
    $osm_obj->{id} = $id;

    delete $attrs{action};
    $osm_obj->{lat} = delete $attrs{lat};
    $osm_obj->{lon} = delete $attrs{lon};

    delete $attrs{timestamp}; # ignore for now

    if ( keys %attrs ) {
	warn "node $id has extra attrs: ".Dumper(\%attrs);
    }

    if ( $AREA_FILTER->inside($osm_obj) ) {
	#obj_compare($osm_nodes->{$id},$osm_obj);
	$osm_nodes->{$id} = $osm_obj;
    }

    $Stats{"elem read"}++;
    $Stats{"nodes read"}++;
    display_status("node");
}
# --------------------------------------------
sub segment_ {
    $osm_obj = undef
}
sub segment {
    my($p, $tag, %attrs) = @_;  
    my $id = delete $attrs{id};
    $osm_obj = {};
    $osm_obj->{id} = $id;

    delete $attrs{action};
    delete $attrs{timestamp}; # ignore for now

    my $from = $osm_obj->{from} = delete $attrs{from};
    my $to   = $osm_obj->{to}   = delete $attrs{to};

    if ( keys %attrs ) {
	warn "segment $id has extra attrs: ".Dumper(\%attrs);
    }
    if ( defined($osm_nodes->{$from}) && defined($osm_nodes->{$to}) ) {
	$osm_segments->{$id} = $osm_obj;
    }

    $Stats{"elem read"}++;
    $Stats{"segments read"}++;
    display_status("node");
}
# --------------------------------------------
sub way_ {
    $osm_obj = undef
}
sub way {
    my($p, $tag, %attrs) = @_;  
    my $id = delete $attrs{id};
    $osm_obj = {};
    $osm_obj->{id} = $id;
    $osm_obj->{timestamp} = delete $attrs{timestamp} if defined $attrs{timestamp};

    delete $attrs{action};

    if ( keys %attrs ) {
	warn "way $id has extra attrs: ".Dumper(\%attrs);
    }
    $osm_ways->{$id} = $osm_obj;

    $Stats{"elem read"}++;
    $Stats{"ways read"}++;
    display_status("node");
}
# --------------------------------------------
sub seg {
    my($p, $tag, %attrs) = @_;  
    my $id = $attrs{id};
    #print "Seg $id for way($osm_obj->{id})\n";
    push(@{$osm_obj->{seg}},$id);
}
# --------------------------------------------
sub tag {
    my($p, $tag, %attrs) = @_;  
    #print "Tag - $tag: ".Dumper(\%attrs);
    my $k = delete $attrs{k};
    my $v = delete $attrs{v};

    return if $k eq "created_by";

    if ( keys %attrs ) {
	print "Unknown Tag value for ".Dumper($osm_obj)."Tags:".Dumper(\%attrs);
    }
    
    my $id = $osm_obj->{id};
    if ( defined( $osm_obj->{tag}->{$k} ) &&
	 $osm_obj->{tag}->{$k} ne $v
	 ) {
	printf "Tag %8s already exists for obj(id=$id) tag '$osm_obj->{tag}->{$k}' ne '$v'\n",$k ;
    }
    $osm_obj->{tag}->{$k} = $v;
    if ( $k eq "alt" ) {
	$osm_obj->{alt} = $v;
    }	    
}

###########################################

# ------------------------------------------------------------------
# Guess the Street Type if we got a Streetname
sub street_name_2_id($) {
    my $street_name = shift;

    my $streets_type_id =0;
    if ( $street_name =~ m/^A\s*\d+/ ) {
	$streets_type_id = streets_type_name2id('Strassen.Autobahn');
    } elsif ( $street_name =~ m/^ST\s*\d+/ ) {
	$streets_type_id = streets_type_name2id('Strassen.Landstrasse');
    } elsif ( $street_name =~ m/^B\s*\d+/ ) {
	$streets_type_id = streets_type_name2id('Strassen.Bundesstrasse');
    }   
    return $streets_type_id;
}

# -----------------------------------------------------------------------------
#my $step = 5;
my $step = .1;
my $download_area = {
    #world      => [-90,-180,  90-$step,180-$step],
    #europe     => [-10,  10,  10,60],
    #usa        => [-xx,  xx,  xx,xx],
    ireland     => [-11,  51,  -4,56],
    uk          => [ -6,  51,   2,60],
    germany     => [  0,  10,  50,60],
    bayern      => [  9,  47,  12,50],
    muenchen    => [ 10,  44,  12,49],
    schweiz     => [  5,  45,  11,48],
    oesterreich => [  9,  46,  17,49],
    italy       => [  6,  36,  20,47.5],
    italyN     => [  6,  44,  14,47.5],
    italyS     => [  9,  36,  20,44],
};

sub download_osm_streets($;$) { # Insert Streets from osm File  
    my $osm_dir = shift;
    my $area    = shift;

    return 
	if $main::no_mirror;

    print "Download OSM Data\n";



    my $osm_auth = " --http-user=$ENV{OSMUSER} --http-passwd=$ENV{OSMPASSWD} ";
    my $osm_base_url="http://www.openstreetmap.org/api/0.3/map?bbox=";
    #$osm_base_url="http://www.openstreetmap.org/api/0.3/map?bbox=11.0,48.0,12.0,49.0

    my ($lat_min,$lon_min,     $lat_max,$lon_max);
    if ( defined $area ) {
	if ( defined ( $download_area->{$area} ) ) {
	    ($lat_min,$lon_min,     $lat_max,$lon_max) = @{$download_area->{$area}};
	} else {
	    warn "Download Area '$area' not registered\n";
	    warn "Existing Areas:\n\t".join("\n\t",keys %{$download_area})."\n";
	    die "\n";	    
	};
	print "Download OSM Area $area ($lat_min,$lon_min,     $lat_max,$lon_max)\n";
	
    } elsif ( $main::lat_min && $main::lat_max && $main::lon_min && $main::lon_max ){
	$lat_min=$main::lat_min;
	$lat_max=$main::lat_max;
	$lon_min=$main::lon_min;
	$lon_max=$main::lon_max;
    } else {
	die "Undefined OSM-Area for Download\n";
    }

    for ( my $lon = $lon_min; $lon < $lon_max; $lon += $step ) {
	for ( my $lat = $lat_min; $lat< $lat_max; $lat += $step ) {
	    my $lap  = $lat + $step;
	    my $lop  = $lon + $step;
	    print "Get $lat,$lon                       \r";
	    my $abs_filename= sprintf("$osm_dir/Streets_osm_%05.2f,%06.2f,%05.2f,%06.2f.osm",
				      $lon,$lat, $lop,$lap  );
	    my $bbox = sprintf("%.1f,%.1f,%.1f,%.1f",$lat,$lon,$lap,$lop);

	    #my $mirror = mirror_file("$osm_base_url$bbox",$abs_filename);
	    
	    if ( $debug ) {
		print "mirror: $bbox                                       \n";
	    }
	    
	    my $rest_tries=10;
	    while ($rest_tries){
		my $size = (-s $abs_filename)||0;
		if ( $size == 538 || $size < 76 ) {
		    my $get_cmd ='';
		    # $get_cmd .= "time " if $debug;
		    $get_cmd .= "curl --netrc ";
		    $get_cmd .= " -o  $abs_filename ";
		    # $get_cmd .= "wget -O $abs_filename $osm_auth ";
		    #$get_cmd .= " -q " unless $debug || $VERBOSE;
		    $get_cmd .= " -v " if $debug && $VERBOSE;
		    $get_cmd .= " -s " unless $debug || $VERBOSE;
		    $get_cmd .= "    '$osm_base_url$bbox'"; 
		    print "$get_cmd\n";
		    my $start_time=time();
		    `$get_cmd`;		    $size = (-s $abs_filename)||0;
		    my $diff_time = time()-$start_time;
		    my $sleep_time= $OSM_polite*$diff_time;
		    print "sleep ".($sleep_time)." seconds\n";
		    sleep $sleep_time;
		}

		my $count_error = `grep -e '400 Bad Request' -e '<H1>ERROR</H1>' -e '<h1>Internal Server Error</h1>' $abs_filename | wc -l`;
		if ( $count_error >0 ) {
		    `cat $abs_filename`;
		    $rest_tries --;
		    print "	ERROR in downloading\n";
		    print "   $rest_tries retries\n";
		    print "	Got $lat,$lon          $size Bytes\n";
		    my $err_filename = $abs_filename;
		    $err_filename =~ s/\.osm$/.error/;
		    rename($abs_filename,$err_filename);
		} else {
		    print "Got $lat,$lon             $size Bytes\n"
			unless $size == 76;
		    $rest_tries=0;
		}
	    }
	}
    }
}

# -----------------------------------------------------------------------------
sub read_osm_file($) { # Insert Streets from osm File
    my $file_name = shift;
    my $area_name = shift;

    my $start_time=time();

    $AREA_FILTER = Geo::Filter::Area->new( area => $area_name );

    print("\rReading $file_name for $area_name\n") if $VERBOSE || $debug;
    print "$file_name:	".(-s $file_name)." Bytes\n" if $debug;

    if ( $file_name =~ m/planet-\d+.osm/ &&
	 -s "$file_name-$area_name.storable.node" &&
         -s "$file_name-$area_name.storable.segment" &&
         -s "$file_name-$area_name.storable.way" 
         ) {
        $osm_nodes    = Storable::retrieve("$file_name-$area_name.storable.node");
        $osm_segments = Storable::retrieve("$file_name-$area_name.storable.segment");
        $osm_ways     = Storable::retrieve("$file_name-$area_name.storable.way");
	if ( $VERBOSE) {
	    printf "Read $file_name-$area_name.storable.* in %.0f sec\n",time()-$start_time;
	}
    } else {
	print STDERR "Parsing file: $file_name\n" if $debug;
	$PARSING_START_TIME=time();
	my $p = XML::Parser->new( Style => 'Subs' ,
				  ErrorContext => 10,
				  );
	
	my $fh = data_open($file_name);
	my $content;
	eval {
	    $content = $p->parse($fh);
	};
	
	if ($@) {
	    die "ERROR: Could not parse osm data $file_name\n".
		"$@\n";
	}
	if (not $p) {
	    die "ERROR: Could not parse osm data $file_name\n";
	}
	if ( $VERBOSE) {
	    printf "Read and parsed $file_name in %.0f sec\n",time()-$start_time;
	}
	if ( $file_name eq "planet.osm" ) {
	    Storable::store($osm_nodes   ,"$file_name-$area_name.storable.node");
		Storable::store($osm_segments,"$file_name-$area_name.storable.segment");
		Storable::store($osm_ways    ,"$file_name-$area_name.storable.way");
		if ( $VERBOSE) {
		    printf "Read and parsed and stored $file_name for area $area_name in %.0f sec\n",time()-$start_time;
		}
	    }
    }

    return;
}

# -----------------------------------------------------------------------------
sub read_osm_dir($) { # read all OSM Files in Diorectory
    my $osm_dir = shift;

    my $osm_auth = " --http-user=$ENV{OSMUSER} --http-passwd=$ENV{OSMPASSWD} ";
    my $osm_base_url="http://www.openstreetmap.org/api/0.3/map?bbox=";
    # http://www.openstreetmap.org/api/0.3/map?bbox=11.0,48.0,12.0,49.0

    for my $abs_filename ( 
			   #glob("$osm_dir/planet.osm"),
			   glob("$osm_dir/Streets*.osm"),
			   glob("$osm_dir/Streets*.xml"),
			   glob("$osm_dir/Streets*.gz"),
			   glob("$osm_dir/Streets*.bz2"),
			   ) {
	$abs_filename .= ".gz"	if  -s "$abs_filename.gz" && ! -s $abs_filename;
	my $size = (-s $abs_filename)||0;
	if ( $size == 538 ) { # Probably Internal Error Message
	    `cat $abs_filename`;
	    next;
	}
	next unless $size >76; # Empty File (Only Header, no nodes)
	print "$abs_filename:	$size Bytes\n";
	read_osm_file($abs_filename);
    }
}

# -----------------------------------------------------------------------------
sub adjust_bounding_box($$$){
    my $bbox = shift;
    my $lat = shift;
    my $lon = shift;

    for my $type ( qw(lat_min lat_max lon_min lon_max lat lon )) {
	next if defined ($bbox->{$type});
	if ( $type =~m/min/ ) {
	    $bbox->{$type} = 1000;
	} else {
	    $bbox->{$type} = -1000;
	}
    }
    # remember lat/lon Min/Max 
    $bbox->{lat_min}= min($bbox->{lat_min},$lat);
    $bbox->{lat_max}= max($bbox->{lat_max},$lat);
    $bbox->{lon_min}= min($bbox->{lon_min},$lon);
    $bbox->{lon_max}= max($bbox->{lon_max},$lon);
    $bbox->{lat}= ($bbox->{lat_min}+$bbox->{lat_max})/2;
    $bbox->{lon}= ($bbox->{lon_min}+$bbox->{lon_max})/2;
}

sub adjust_bounding_box_for_segment($$){
    my $bbox = shift;
    my $segment = shift;
    $segment = $osm_segments->{$segment}
    if $segment =~ m/^\d+$/;
    my $node_from = $segment->{from};
    my $node_to   = $segment->{to};
    my $lat1 = $osm_nodes->{$node_from}->{lat};
    my $lon1 = $osm_nodes->{$node_from}->{lon};
    my $lat2 = $osm_nodes->{$node_to}->{lat};
    my $lon2 = $osm_nodes->{$node_to}->{lon};
    adjust_bounding_box($bbox,$lat1,$lon1);
    adjust_bounding_box($bbox,$lat2,$lon2);
}

# ------------------------------------------------------------------
sub tag_string($){
    my $obj = shift;
    my $tags      = $obj->{tag};
    my $tags_string = join(" ",map{" $_:$tags->{$_} "}keys %$tags);
    my $erg ='';
    $erg = " Tags[$tags_string] " if $tags_string;
    return $erg;
}

# ------------------------------------------------------------------
my $class2type = {
    "barn"		=> "area.building.barn",
    "bridge"		=> "area.bridge",
    "campsite"		=> "accomodation.campground",
    "car park"		=> "transportation.car.parkinglot",
    "caution"		=> "general.caution",
    "church"		=> "church",
    "city"		=> "places.city.small",
    "country park"	=> "recreation.park",
    "farm"		=> "area.area.farm",
    "hamlet"		=> "places.city.hamlet",
    "hill"		=> "area.area.hill",
    "historic-name"	=> "general.point-of-interest",
    "industrial area"	=> "area.area.industial-area",
    "large town"	=> "places.city.large",
    "lift"		=> "transportation.public.station.lift",
    "locality"		=> "general",
    "parking"		=> "transportation.car.parkinglot",
    "point of interest"	=> "general.point-of-interest",
    "pub"		=> "recreation.pub",
    "railway crossing"	=> "area.railway-crossing",
    "railway station"	=> "transport-public.station.railroad",
    "restaurant"	=> "food.restaurant",
    "school"		=> "education.school",
    "small town"	=> "places.city.small",
    "suburb"		=> "places.city.small",
    "tea shop"		=> "shopping.food.tea",
    "town"		=> "places.city.small",
    "trafficlight"	=> "transport-vehicle.traffic-light",
    "tunnel"		=> "area.tunnel",
    "viewpoint"		=> "general.viewpoint",
    "village"		=> "places.city.small",
    "Village"		=> "places.city.small",
    "waypoint"		=> "general.waypoint",
};

sub node2type($){
    my $node = shift;
    $node = $osm_nodes->{$node} if $node =~ m/^\d+$/;

    my $type =0;
    return 0  
	unless defined  $node->{tag};
    my $tag =   $node->{tag};
    my (%tags) = (%{$tag});
    delete $tags{created_by};
    delete $tags{time};
    delete $tags{ele};
    delete $tags{vdop};
    delete $tags{hdop};
    delete $tags{fix};
    delete $tags{car};
    delete $tags{sat};
    delete $tags{'opm:difficulty'};
    return 0 if $tags{course};
    return 0 if $tags{highway};
    return 0 if $tags{lanes};
    return 0 if $tags{lanes};
    return 0 if $tags{speed};
    return 0 unless $tags{class};
    return 0 if $tags{class} =~ m/^(path|lift|motorway|primary|secondary|piste|river|bridge|node)$/i;
    return 0 if $tags{class} =~ m/^(residential|trackpoint|waypoint|unclassified|mast)$/i;
    return 0
	unless keys %tags;
    return 0
	unless defined  $tags{class};
    my $node_class = $tags{class};
    if ( defined( $class2type->{$node_class}) ) {
	if ( ! $class2type->{$node_class} ) {
	    print "unclassified node_class $node_class.\n";
	    print "node2type:".Dumper(\$node);
	}
	my $poi_type_name = $class2type->{$node_class};
	if ( $poi_type_name ) {
	    $type = poi_type_name2id($poi_type_name);
	    print "no known id for  node-class: '$node_class';	poi_type_name: $poi_type_name\n"
		unless $type;
	} 
    }
    return $type;
    return 0;
}

# ------------------------------------------------------------------
sub fill_osm_nodes(){

    delete_existing_entries("node");
    my $source_id= get_source_id("node");
    my $count_inserted=0;
    my $count_nodes=0;
    for my $node_id (  keys %{$osm_nodes} ) {
	$count_nodes++;
	my $node = $osm_nodes->{$node_id};
	my $lat = $osm_nodes->{$node_id}->{lat};
	my $lon = $osm_nodes->{$node_id}->{lon};
	my $poi_type = node2type($node);
	next unless $poi_type > 0;
	my $tag = $node->{tag};
	my $poi_name = ($tag->{ref}||'').($tag->{name}||'unnamed');
	# insert into mysql
	# ....
	my $tags_string = tag_string($node);
	my $comment = $tags_string ;
	$comment .= "node_id=$node_id ";

	my $values;
	$values->{'poi.scale_min'} = 1;
	$values->{'poi.scale_max'} = 50000;
	$values->{'poi.proximity'} = "5000m";
	$values->{'poi.source_id'} = $source_id;
	$values->{'poi.name'}      = $poi_name;
	$values->{'poi.lat'}       = $lat;
	$values->{'poi.lon'}       = $lon;
	$values->{'poi.type'}      = $poi_type;
	$values->{'poi.poi_type_id'} = $poi_type;
	$values->{'poi.comment'}   = $comment;
	printf("--------- fill poi: (type $poi_type,%s) name=$poi_name\n",poi_type_id2name($poi_type))
	    if $debug;
	Geo::Gpsdrive::DBFuncs::add_poi($values);
	printf("inserted %d (%d) OSM Nodes as POI\r",$count_inserted++,$count_nodes);
    }
    print "\n";
};

# ------------------------------------------------------------------
sub obj2street_id($){
    my $obj = shift;

    my $tags  = $obj->{tag}  || {};
    my $class = $tags->{class}  || '';
    my $name  = $obj->{name} || '';

    my $streets_type_id=0;
    my $id_name='';
    if ( $class eq "residential" ) {	
	$id_name = 'Strassen.residential';
    } elsif ( $class eq "minor" ) {	
	$id_name = 'Strassen.minor';
    } elsif ( $class eq "secondary" ) {	
	$id_name = 'Strassen.secondary';
    } elsif ( $class eq "primary" ) {	
	$id_name = 'Strassen.primary';
    } elsif ( ($tags->{highway}||'') ne "no"  ) {
	$id_name = 'Strassen.Autobahn';
    } elsif ( $class eq "motorway" ) {	
	$id_name = 'Strassen.Autobahn';
    } elsif ( $tags->{car} ) {	
	$id_name = 'Strassen.Allgemein';
    } elsif ( $tags->{foot} ) {	
	$id_name = 'Strassen.Fussweg';
    } elsif ( $tags->{bike} ) {	
	$id_name = 'Strassen.Allgemein';
    } elsif ( $tags->{horse} ) {	
	$id_name = 'Strassen.Reitweg';
    } elsif ( ($tags->{abutters} ||'') eq "residential" ) {
	$id_name = 'Strassen.residential';
    } ;
    
    if ( $id_name ) {
	$streets_type_id = streets_type_name2id($id_name);
	if ( ! $streets_type_id ) {
	    print "#---------------------------------\n";
	    print "Proposed id_name '$id_name' not found\n";
	    print "#---------------------------------\n\n\n";
	    return 0;
	}
    } 

    $streets_type_id ||= streets_type_name2id($obj->{name});
    $streets_type_id ||= streets_type_name2id($obj->{ref});
    

    if ( 0 && $debug ) {
	my $tags_string = tag_string($obj);
	print "Street Type ID: $streets_type_id\t";
	print "Street Name: $name\t";
	print "Street Tags: $tags_string\n";
    }
    
    if ( ! $streets_type_id &&( $debug || $VERBOSE ) ) {
	print "#---------------------------------\n";
	print "Unknown Street Type for '$name' (id: $obj->{id}):\n";
	print "class: $class\n";
	print "obj:".Dumper(\$obj);
	print "tags:".Dumper(\$tags);
	print "#---------------------------------\n\n";
    }
    return $streets_type_id;
}

# ------------------------------------------------------------------
# Make some stats on the data in Memory
# - # of segments connected to this node
# - # of way which use this segment
my $node_positions;
sub create_internal_stats_osm(){
    my $start_time=time();

    # Have a look at all duplicate Nodes
    for my $node_id (  keys %{$osm_nodes} ) {
	my $node = $osm_nodes->{$node_id};
	my $lat = $osm_nodes->{$node_id}->{lat};
	my $lon = $osm_nodes->{$node_id}->{lon};
	my $alt = $osm_nodes->{$node_id}->{alt};
	$alt = "-9999" unless defined $alt;
	my $position="$lat,$lon,$alt";
	if ( defined $node_positions->{$position} ) {
	    $node->{master_node} = $node_positions->{$position}->[0];
	}
	push(@{$node_positions->{$position}},$node_id);
    }

    # mark all nodes in segments as connected
    my $seg_id;
    for $seg_id ( keys %{$osm_segments} ) {
	$osm_segments->{$seg_id}->{ways}++;
	my $node_from = $osm_segments->{$seg_id}->{from};
	my $node_to   = $osm_segments->{$seg_id}->{to};
	my $tags      = $osm_segments->{$seg_id}->{tag};
	
	# Count Number of connections to a node
	if ( ! defined( $osm_segments->{$seg_id} ) ) {
	    $osm_nodes->{$node_from}->{referenced_by_segment}++;
	    $osm_nodes->{$node_to}->{referenced_by_segment}++;
	} 

    }

    my $segments_in_way=0;
    for my $way_id ( keys %{$osm_ways} ) {
	my $way = $osm_ways->{$way_id};
	
	for $seg_id ( @{$way->{seg}} ) {
	    $segments_in_way++;
	    $osm_segments->{$seg_id}->{referenced_by_way} ++;
	}
    }

    $osm_stats->{segments}             = scalar keys( %$osm_segments);
    $osm_stats->{ways}                 = scalar keys %{$osm_ways};
    $osm_stats->{nodes}                = scalar keys %{$osm_nodes};
    $osm_stats->{segments_in_ways}     = $segments_in_way;
    $osm_stats->{segments_not_in_ways} = $osm_stats->{segments}-$segments_in_way;

    printf("Indexed Osm-Data in %.0f sec\n",time()-$start_time );
}

# -----------------------------------------------------------------------------
sub fill_osm_ways() { # Insert Streets from osm variables into mysql-db for gpsdrive

    delete_existing_entries("way");
    my $source_id= get_source_id("way");

    my $start_time=time();

    print "Import new OSM way Data\n";
    print("Importing  $osm_stats->{segments_in_ways} Segments\n") if $debug;
    print("Importing  $osm_stats->{ways} osm Ways\n")             if $debug;

    # ---------------------- write Streets to DB
    # Neue Strasse anfangen
    print "Creating Segments\n" if $debug;
    my $multi_segment={};
    my $lat1=1003;
    my $lat2=1003;
    my $lon1=1003;
    my $lon2=1003;
    my $max_allowed_dist; # Distance to warn for more precise Data
    my $max_dist =0;
    $multi_segment={};
    $multi_segment->{'source_id'} = $source_id;
    $multi_segment->{'streets_type_id'} = '';
    $multi_segment->{'streets_type_id'} = streets_type_name2id('Strassen.Allgemein');
    $max_allowed_dist = 0.05;
    
    $multi_segment->{'scale_min'}       = 1;
    $multi_segment->{'scale_max'}       = 50*1000*1000;
    $multi_segment->{'source_id'}       = $source_id;
    $multi_segment->{'segments'}        =[];
    
    my $segment_count=0;
    my $lat_min= 1000;
    my $lat_max=-1000;
    my $lon_min= 1000;
    my $lon_max=-1000;
    
    for my $way_id (  keys %{$osm_ways} ) {
	my $way = $osm_ways->{$way_id};
	my $tags      = $way->{tag};
	
	my $street_name=$tags->{name} || $tags->{ref} || "Way uid=$way_id";
	$multi_segment->{'name'}            = $street_name;

	my $streets_type_id = obj2street_id($way);
        $streets_type_id ||= streets_type_name2id('Strassen.Allgemein');
	
	my $tags_string = tag_string($way);
	
	for my $seg_id ( @{$way->{seg}} ) {
	    next unless $seg_id;
	    next unless defined $osm_segments->{$seg_id};
	    next unless $osm_segments->{$seg_id}->{from};
	    next unless $osm_segments->{$seg_id}->{to};

	    my $segment = $osm_segments->{$seg_id};
	    $segment->{way_element}++;

	    my $node_from = $segment->{from};
	    my $node_to   = $segment->{to};
	    $lat1 = $osm_nodes->{$node_from}->{lat};
	    $lon1 = $osm_nodes->{$node_from}->{lon};
	    $lat2 = $osm_nodes->{$node_to}->{lat};
	    $lon2 = $osm_nodes->{$node_to}->{lon};

	    # remember lat/lon Min/Max 
	    $lat_min= min($lat_min,$lat1);
	    $lat_max= max($lat_max,$lat1);
	    $lon_min= min($lon_min,$lon1);
	    $lon_max= max($lon_max,$lon1);

	    $lat_min= min($lat_min,$lat2);
	    $lat_max= max($lat_max,$lat2);
	    $lon_min= min($lon_min,$lon2);
	    $lon_max= max($lon_max,$lon2);
	    
	    $multi_segment->{'streets_type_id'} = $streets_type_id;

	    my $tags_string_seg = tag_string($segment);

	    my $comment ='';
	    $comment .= "way_id=$way_id ";
	    $comment .= "seg_id=$seg_id ";
	    $comment .= ($tags_string.$tags_string_seg);
	    $comment .= "Seg# : ".$osm_nodes->{$node_from}->{cluster} 
	    if defined ($osm_nodes->{$node_from}->{cluster});
	    push(@{$multi_segment->{'segments'}},
	     {
		 'lat1' => $lat1, 'lon1' => $lon1, 'alt1' => -99,
		 'lat2' => $lat2, 'lon2' => $lon2, 'alt2' => -99,
		 'name' => $street_name,
		 'comment' => $comment,
		 'streets.source_id' => $source_id,
		 'streets_type_id' => $streets_type_id,
	     }
		 );
	    $segment_count++
	    }
	print "Writing Segments\n" if $debug;
	street_segments_add($multi_segment);
	$multi_segment->{'segments'}=[];
	print "lat($lat_min , $lat_max)	lon($lon_min , $lon_max)\n" 
	    if $VERBOSE && $debug;
	
    }; # of for osm_ways

    printf("Imported $segment_count Segments for  ".scalar(keys(%{$osm_ways}))." osm Ways".
	   " in %.0f sec\n",time()-$start_time );
    
}

# ------------------------------------------------------ 
# Segments not in Ways
# Insert Streets from osm variables into mysql-db for gpsdrive
# Take all the segments which are not connected otherwise
sub fill_osm_rest_segments() { 

    delete_existing_entries("segment");
    my $source_id= get_source_id("segment");

    my $rest_segments = $osm_stats->{segments_not_in_ways};
    print "Writing $rest_segments unconnected Segments to DB\n";

    my $start_time=time();

    my $segment_count=0;
    my $new_segment_count=0;
    my $streets_type_id = streets_type_name2id('Strassen.minor');

    my $estimate_timer=time();
    for my $seg_id ( keys %{$osm_segments} ) {
	my $segment = $osm_segments->{$seg_id};
	$segment_count++;
	# For DEBUGGING:
	# last if $segment_count>1000;
	if ( !($segment_count % 1000)) {
	    my $percent = $segment_count/$rest_segments*100;
	    my $time_diff=time()-$estimate_timer;
	    my $segments_missing=$rest_segments-$segment_count;
	    my $rest_estimate = $time_diff/1000*$segments_missing;
	    $estimate_timer=time();
	    printf "Writing Segment $segment_count to DB ( %d%%) %d Segments missing; estimating %4.2f min\r"
		,$percent,$segments_missing,$rest_estimate/60;
	}
	next 
	    if defined($segment->{way_element})
	    && $segment->{way_element} > 0;
	$new_segment_count++;
	my $tags = $segment->{tag};
	my $street_name=$tags->{name} || $tags->{ref} || "Segment=$seg_id";

	my $node_from = $segment->{from}||0;
	my $node_to   = $segment->{to}||0;
	#	print Dumper( $segment);
	unless ( $node_from && $node_to ) {
	    #print "Missing node '$node_from' or '$node_to' for Segment # $seg_id\n";
	    next;
	}
	my $lat1 = $osm_nodes->{$node_from}->{lat};
	my $lon1 = $osm_nodes->{$node_from}->{lon};
	#print Dumper( $osm_nodes->{$node_to});
	my $lat2 = $osm_nodes->{$node_to}->{lat};
	my $lon2 = $osm_nodes->{$node_to}->{lon};

	my $tags_string_seg = tag_string($segment);

	my $comment ='';
	$comment .= "seg_id=$seg_id ";
	$comment .= $tags_string_seg;

	#my $streets_type_id = 0; # obj2street_id($segment);
	#$streets_type_id ||= streets_type_name2id('Strassen.Allgemein');
	my $segment4db = {
	    'streets.lat1' => $lat1, 'streets.lon1' => $lon1, 'streets.alt1' =>-99,
	    'streets.lat2' => $lat2, 'streets.lon2' => $lon2, 'streets.alt2' =>-99,
	    'streets.streets_type_id' => $streets_type_id,
	    'streets.scale_min' => 1,
	    'streets.scale_max' => 40000,
	    'streets.last_modified'  => time(),
	    'streets.source_id' => $source_id,
	    'streets.name'      => $street_name
	    };
	insert_hash("streets",$segment4db);
    }
    if ( $debug) {
	debug("Imported another $new_segment_count Segments not in ways for  ".
	      scalar(keys(%{$osm_segments}))." osm Segments".
	      sprintf(" in %.0f sec\n",time()-$start_time));
    }
}




# ******************************************************************
sub delete_existing_entries($){
    my $type = shift;
    my $source = "OpenStreetMap.org $type";

    unless ( $main::no_delete ) {
	print "Delete old 'OSM $type' Data\n";
	Geo::Gpsdrive::DBFuncs::delete_all_from_source($source);
	print "Deleted old '$source' Data\n" if $VERBOSE || $debug;
    }

}

# ******************************************************************
sub get_source_id($){
    my $type = shift;
    my $source = "OpenStreetMap.org $type";

    my $source_id = Geo::Gpsdrive::DBFuncs::source_name2id($source);

    unless ( $source_id ) {
	my $source_hash = {
	    'source.url'     => "http://openstreetmap.org/",
	    'source.name'    => $source ,
	    'source.comment' => '' ,
	    'source.licence' => ""
	    };
	Geo::Gpsdrive::DBFuncs::insert_hash("source", $source_hash);
	$source_id = Geo::Gpsdrive::DBFuncs::source_name2id($source);
    }
    exit unless $source_id;
    return $source_id;
}


# *****************************************************************************

sub import_Data($@){
    my $areas_todo = shift;
    my @filenames = @_;

    #$areas_todo ||= 'germany';
    $areas_todo ||= 'world';
    $areas_todo=lc($areas_todo);

    print "\nImport OSM Data\n";
	
    my $mirror_dir="$main::MIRROR_DIR/osm";
    
    -d $mirror_dir or mkpath $mirror_dir
	    or die "Cannot create Directory $mirror_dir:$!\n";
    
    for my $filename ( @filenames ) {
	print "Import OSM Data '$filename'\n";
	
	if ( $filename =~ m/^download:(.+)/ ) {
	    my $area = $1;
	    download_osm_streets($mirror_dir,$area);
	    read_osm_dir($mirror_dir);
	} elsif ( -s $filename ) {
	    read_osm_file( $filename);
	} elsif ( $filename eq '' ) {
	    print "Download planet-xxxxxx.osm\n";
	    $filename = mirror_planet();
	    print "Mirror $filename complete\n";
	    read_osm_file($filename);
	} else {
	    die "OSM::import_Data: Cannot find File '$filename'\n";
	    print "Read OSM Data\n";
	};
    }	
	
    # Update Counters and references
    create_internal_stats_osm();
    
	
    print "OSM Nodes:    " . scalar keys( %$osm_nodes)."\n";
    print "OSM Segments: " . scalar keys( %$osm_segments)."\n";
    print "OSM Ways:     " . scalar keys( %$osm_ways)."\n";
    
    disable_keys('poi');
    fill_osm_nodes();
    enable_keys('poi');
    
    disable_keys('streets');
    fill_osm_ways();
    fill_osm_rest_segments();
    enable_keys('streets');


    print "\nDownload and import of OSM Data FINISHED\n";
}

1;
