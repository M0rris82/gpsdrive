# Import Data from http://openstreetmap.org/

use strict;
use warnings;

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
use Data::Dumper;
use XML::Parser;

my $osm_nodes;
my $osm_segments;
my $osm_ways;

# ------------------------------------------------------------------
# Guess the Street Type if we got a Streetname
sub street_name_2_id($) {
    my $street_name = shift;

    my $streets_type_id =0;
    if ( $street_name =~ m/^A\s*\d+/ ) {
	$streets_type_id = streets_type_name2id('Strassen.Autobahn');
    } elsif ( $street_name =~ m/^ST\s*\d+/ ) {
	$streets_type_id = streets_type_name2id('Strassen.Bundesstrasse');
    } elsif ( $street_name =~ m/^B\s*\d+/ ) {
	$streets_type_id = streets_type_name2id('Strassen.Bundesstrasse');
    }   
    return $streets_type_id;
}

# -----------------------------------------------------------------------------
# Open Data File in predefined Directories
sub data_open($){
    my $file_name = shift;

    my $file_with_path="$file_name";
    if ( -s $file_with_path ) {
	debug("Opening $file_with_path");
	my $fh;
	if ( $file_with_path =~ m/\.gz$/ ) {
	    $fh = IO::File->new("gzip -dc $file_with_path|")
		or die("cannot open $file_with_path: $!");
	} elsif ( $file_with_path =~ m/\.bz2$/ ) {
	    $fh = IO::File->new("bzip2 -dc $file_with_path|")
		or die("cannot open $file_with_path: $!");
	} else {
	    $fh = IO::File->new("<$file_with_path")
		or die("cannot open $file_with_path: $!");
	}
	return $fh;
    }
    die "cannot Find $file_name";
}

###########################################
my $node_id=0;
sub node_ {
    $node_id=0;
}
sub node {
    my($p, $tag, %attrs) = @_;  
    $node_id=$attrs{id};
    $osm_nodes->{$node_id}->{lat} = $attrs{lat};
    $osm_nodes->{$node_id}->{lon} = $attrs{lon};
    $osm_nodes->{$node_id}->{id} = $node_id;
}

# --------------------------------------------
my $way_id=0;
sub way_ {
    $way_id=0;
}
sub way {
    my($p, $tag, %attrs) = @_;  
    #print "Way - $tag: ".Dumper(\%attrs);
    $way_id=$attrs{id};
    $osm_ways->{$way_id}->{id}=$way_id;
}
# --------------------------------------------
my $segment_id=0;
sub segment_ {
    $segment_id=0;
}
sub segment {
    my($p, $tag, %attrs) = @_;  
    #print "Segment - $tag: ".Dumper(\%attrs);
    $segment_id=$attrs{id};
    $osm_segments->{$segment_id}->{from}=$attrs{from};
    $osm_segments->{$segment_id}->{to}=$attrs{to};
    $osm_segments->{$segment_id}->{id}=$attrs{id};
}
# --------------------------------------------
sub seg {
    my($p, $tag, %attrs) = @_;  
    my $id = $attrs{id};
    #print "Seg $id for way($way_id)\n";
    push(@{$osm_ways->{$way_id}->{seg}},$id);
}
# --------------------------------------------
sub tag {
    my($p, $tag, %attrs) = @_;  
    #print "Tag - $tag: ".Dumper(\%attrs);
    my $k = $attrs{k};
    my $v = $attrs{v};

    #return if $k eq "created_by";

    if ($way_id) {
	if ( defined( $osm_ways->{$way_id}->{tag}->{$k} ) ) {
	    warn "Tag $k already exists for way($way_id)\n";
	} else {
	    $osm_ways->{$way_id}->{tag}->{$k} = $v;
	}
    } elsif ($segment_id) {
	$osm_segments->{$segment_id}->{tag}->{$k} = $v;
    } elsif ($node_id) {
	$osm_nodes->{$node_id}->{tag}->{$k} = $v;
    } else {
	print "Unknown Tag $tag,".Dumper(\%attrs);
    }
}

###########################################

# -----------------------------------------------------------------------------
sub read_osm_streets_area($) { # Insert Streets from osm File
    my $file_name = shift;

    my $start_time=time();

    print("Reading $file_name\n") if $verbose || $debug;
    print "$file_name:	".(-s $file_name)." Bytes\n" if $debug;

    print STDERR "Parsing file: $file_name\n" if $debug;
    my $p = XML::Parser->new( Style => 'Subs' ,
			      );

    my $fh = data_open($file_name);
    my $content = $p->parse($fh);
    if (not $p) {
	print STDERR "WARNING: Could not parse osm data\n";
	return;
    }

    if ( $debug) {
	printf "Read and parsed $file_name in %.0f sec\n",time()-$start_time;
    }
    print "OSM Nodes:    " . scalar keys( %$osm_nodes)."\n";
    print "OSM Segments: " . scalar keys( %$osm_segments)."\n";
    print "OSM Ways:     " . scalar keys( %$osm_ways)."\n";
    return;
}

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
    "amenity"	=> "",
    "barn" => "area.building.barn",
    "bridge" => "area.bridge",
    "campsite" => "accomodation.campground",
    "canal"	=> "",
    "car park" => "transport.car.parkinglot",
    "caution" => "general.caution",
    "church" => "church",
    "city" => "area.city",
    "country park" => "recreation.park",
    "farm" => "area.area.farm",
    "hamlet"	=> "area.city.hamlet",
    "hill" => "area.area.hill",
    "historic-name"	=> "",
    "industrial area"	=> "area.area.industial_area",
    "large town" => "area.city.large",
    "Ledi Drive"	=> "",
    "lift" => "",
    "locality" => "",
    "mast" => "",
    "minor" => "",
    "motorway"	=> "",
    "Motorway"	=> "",
    "National Route"	=> "",
    "offPiste"	=> "",
    "parking" => "transport.car.parkinglot",
    "path"	=> "",
    "piste"	=> "",
    "point of interest"	=> "general.point_of_interest",
    "primary"	=> "",
    "Primary"	=> "",
    "pub" => "recreation.pub",
    "railway"	=> "",
    "railway crossing"	=> "",
    "railway station" => "transport.public.station.railroad",
    "Regional Route"	=> "",
    "residential"	=> "",
    "restaurant" => "food.restaurant",
    "river"	=> "",
    "school" => "education.school",
    "secondary"	=> "",
    "small town" => "area.city.small",
    "street"	=> "",
    "suburb" => "area.city",
    "tea shop" => "shopping.food.tea",
    "town"	=> "",
    "trackpoint"	=> "",
    "trafficlight" => "transport.traffic.signs.trafficlight",
    "train"	=> "",
    "tram"	=> "",
    "tunnel"	=> "",
    "unclassified"	=> "",
    "unsurfaced"	=> "",
    "viewpoint"	=> "",
    "village" => "area.city",
    "Village" => "area.city",
    "waypoint"	=> "",
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
    return 0 if $tags{class} =~ m/^(residential|trackpoint|waypoint)$/i;
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
	    print "no known id for  node-class:$node_class; poi_type_name: $poi_type_name\n"
		unless $type;
	} 
    }
    return $type;
    return 0;
}

# ------------------------------------------------------------------
sub fill_osm_nodes(){

    my $source_id= delete_existing_entries("node");

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
	$values->{'poi.scale_min'} = 0;
	$values->{'poi.scale_max'} = 5000000;
	$values->{'poi.proximity'} = "5000m";
	$values->{'poi.source_id'} = $source_id;
	$values->{'poi.name'}      = $poi_name;
	$values->{'poi.lat'}       = $lat;
	$values->{'poi.lon'}       = $lon;
	$values->{'poi.type'}      = $poi_type;
	$values->{'poi.poi_type_id'}      = $poi_type;
	$values->{'poi.comment'}   = $comment;
	print "--------- fill poi: ($poi_type): $poi_name\n" 
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
	my $tags_string = join(" ",map{" $_:$tags->{$_} "}keys %$tags);
	print "Street Type ID: $streets_type_id\t";
	print "Street Name: $name\t";
	print "Street Tags: $tags_string\n";
    }
    
    if ( ! $streets_type_id &&( $debug || $verbose ) ) {
	print "#---------------------------------\n";
	print "Unknown Street Type for '$name' (id: $obj->{id}):\n";
	print "class: $class\n";
	print "obj:".Dumper(\$obj);
	print "tags:".Dumper(\$tags);
	print "#---------------------------------\n\n";
    }
    return $streets_type_id;
}

# -----------------------------------------------------------------------------
sub fill_osm_ways() { # Insert Streets from osm variables into mysql-db for gpsdrive
    my $source_id= delete_existing_entries("way");

    print "Import new OSM way Data\n";
    for my $seg_id ( keys %{$osm_segments} ) {
	$osm_segments->{$seg_id}->{ways}++;
	my $node_from = $osm_segments->{$seg_id}->{from};
	my $node_to   = $osm_segments->{$seg_id}->{to};
	my $tags      = $osm_segments->{$seg_id}->{tag};
	
	# Count Number of connections to a node
	if ( ! defined( $osm_segments->{$seg_id} ) ) {
	    $osm_nodes->{$node_from}->{connections}++;
	    $osm_nodes->{$node_to}->{connections}++;
	} 

    }

    #print("Importing ".scalar(keys(%{$osm_segments}))." osm Segments\n") if $debug;
    #print("Importing  ".scalar(keys(%{$osm_ways}))." osm Ways\n") if $debug;

    # ---------------------- write Streets to DB
    # Neue Strasse anfangen
    # $street_name = $line;
    print "Creating Segments\n" if $debug;
    my $street_name='';
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
    
    $multi_segment->{'name'}            = $street_name;
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
	
	my $street_name=$tags->{name} || "Way uid=$way_id";
	my $streets_type_id = obj2street_id($way);
        $streets_type_id ||= streets_type_name2id('Strassen.Allgemein');
	
	my $tags_string = join(" ",map{" $_:$tags->{$_} "}keys %$tags);
	
	for my $seg_id ( @{$way->{seg}} ) {
	    next unless $seg_id;
	    next unless defined $osm_segments->{$seg_id};
	    next unless $osm_segments->{$seg_id}->{from};
	    next unless $osm_segments->{$seg_id}->{to};

	    my $node_from = $osm_segments->{$seg_id}->{from};
	    my $node_to   = $osm_segments->{$seg_id}->{to};
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

	    my $comment = "tags=tags_string ";
	    $comment .= "seg_id=$seg_id ";
	    $comment .= "Seg# : ".$osm_nodes->{$node_from}->{cluster} 
	    if defined ($osm_nodes->{$node_from}->{cluster});
	    push(@{$multi_segment->{'segments'}},
	     {
		 'lat1' => $lat1, 'lon1' => $lon1, 'alt1' => -99,
		 'lat2' => $lat2, 'lon2' => $lon2, 'alt2' => -99,
		 'name' => $street_name." tags_string",
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
	    if $verbose && $debug;
	
    }; # of for osm_ways

    debug("Imported $segment_count Segments for  ".scalar(keys(%{$osm_ways}))." osm Ways")
	if $debug;

    # ------------------------------------------------------ Segments not in Ways
    my $rest_segments =  (scalar keys( %$osm_segments) ) - $segment_count;
    debug("Importing  $rest_segments unconnected Segments")
	if $debug;

    $segment_count=0;
    for my $seg_id ( keys %{$osm_segments} ) {
	my $segment = $osm_segments->{$seg_id};
	next 
	    if defined($segment->{connections})
	    && $segment->{connections} > 0;
	$segment_count++;
	if ( $verbose && !($segment_count %1000)) {
	    my $percent = $segment_count/$rest_segments*100;
	    printf "importing Segment $segment_count ( %d%%)\r",$percent;
	}

	my $node_from = $segment->{from}||0;
	my $node_to   = $segment->{to}||0;
	#	print Dumper( $segment);
	unless ( $node_from && $node_to ) {
	    print "Missing node '$node_from' or '$node_to' for Segment # $seg_id\n";
		next;
	}
	$lat1 = $osm_nodes->{$node_from}->{lat};
	$lon1 = $osm_nodes->{$node_from}->{lon};
	#print Dumper( $osm_nodes->{$node_to});
	$lat2 = $osm_nodes->{$node_to}->{lat};
	$lon2 = $osm_nodes->{$node_to}->{lon};
	my $streets_type_id = obj2street_id($segment);
	$streets_type_id ||= streets_type_name2id('Strassen.Allgemein');
	my $segment4db = {
	    'streets.lat1' => $lat1, 'streets.lon1' => $lon1, 'streets.alt1' =>-99,
	    'streets.lat2' => $lat2, 'streets.lon2' => $lon2, 'streets.alt2' =>-99,
	    'streets.streets_type_id' => $streets_type_id,
	    'streets.scale_min' => 1,
	    'streets.scale_max' => 75000,
	    'streets.last_modified'  => time(),
	    'streets.source_id' => $source_id,
	    'streets.name'      => $street_name
	    };
	insert_hash("streets",$segment4db);
    }
    debug("Imported another $segment_count Segments non way for  ".scalar(keys(%{$osm_segments}))." osm Segments")
	if $debug;
}



# -----------------------------------------------------------------------------
sub read_osm_streets($) { # Insert Streets from osm File  
    my $osm_dir = shift;

    my $osm_auth = " --http-user=$ENV{OSMUSER} --http-passwd=$ENV{OSMPASSWD} ";
    my $osm_base_url="http://www.openstreetmap.org/api/0.3/map?bbox=";
    #$osm_base_url="http://www.openstreetmap.org/api/0.3/map?bbox=11.0,48.0,12.0,49.0

    for my $abs_filename ( glob("$osm_dir/*.xml") ) {
	my $size = (-s $abs_filename)||0;
	if ( $size == 538 ) {
	    `cat $abs_filename`;
	    next;
	}
	next unless $size >76;
	read_osm_streets_area($abs_filename);
    }
}

# ******************************************************************
sub delete_existing_entries($){
    my $type = shift;
    print "Delete old OSM $type Data\n";

    my $source = "OpenStreetMap.org $type";

    Geo::Gpsdrive::DBFuncs::delete_all_from_source($source);
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
    print "Deleted old $source($source_id) Data\n" if $verbose || $debug;
    exit unless $source_id;
    return $source_id;
}


# *****************************************************************************
sub import_Data(){

    print "\nDownload and import OSM Data\n";

    my $mirror_dir="$main::MIRROR_DIR/osm";

    -d $mirror_dir or mkpath $mirror_dir
	or die "Cannot create Directory $mirror_dir:$!\n";
    
    my $url = "http://www.ostertag.name/osm/planet.osm.bz2";
    my $tar_file = "$mirror_dir/planet.osm.bz2";

    print "Mirror $url\n";
    my $mirror = mirror_file($url,$tar_file);

    print "Read OSM Data\n";
    #read_osm_streets($mirror_dir);
    read_osm_streets_area($tar_file);

    #read_osm_streets_area( "/home/tweety/.gpsdrive/MIRROR/osm-0.3/Streets_osm_040,0010,050,0020.xml");
    #read_osm_streets_area( "/home/tweety/.gpsdrive/MIRROR/osm-0.3/Streets_osm_040,0010,050,0020-test.xml");
    #read_osm_streets_area("/home/tweety/.gpsdrive/MIRROR/osm-0.3/Streets_osm_00,60,10,70.xml");
    #read_osm_streets_area("/home/tweety/.gpsdrive/MIRROR/osm-0.3/Streets_osm_040,0010,050,0020.xml");

    disable_keys('poi');
    fill_osm_nodes();
    enable_keys('poi');

    disable_keys('streets');
    fill_osm_ways();
    enable_keys('streets');


    print "Download and import of OSM Data FINISHED\n";
}

1;
