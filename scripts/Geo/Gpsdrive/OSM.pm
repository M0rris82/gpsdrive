# Import Data from http://openstreetmap.org/

package Geo::Gpsdrive::OSM;

sub min($$){
    my $a=shift;
    my $b=shift;
    return $a<$b?$a:$b;
}
sub max($$){
    my $a=shift;
    my $b=shift;
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

my $osm_source_id;

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
    }   else {
	print "Unknown Street Type for $street_name\n" if $debug || $verbose;
	$streets_type_id = streets_type_name2id('Strassen.Allgemein');
    };
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
sub node {
    my($p, $tag, %attrs) = @_;  
    $osm_nodes->{$attrs{id}}->{lat} = $attrs{lat};
    $osm_nodes->{$attrs{id}}->{lon} = $attrs{lon};
}

# --------------------------------------------
my $way_id=0;
sub way {
    my($p, $tag, %attrs) = @_;  
    #print "Way - $tag: ".Dumper(\%attrs);
    $way_id=$attrs{id};
#    $osm_ways->{$way_id}->{id}=$way_id;
}
sub way_ {
    my($p, $tag, %attrs) = @_;  
    #print "_Way - $way_id: ".Dumper($osm_ways->{$way_id}->{id});
    $way_id=0;
}
# --------------------------------------------
sub segment {
    my($p, $tag, %attrs) = @_;  
    #print "Segment - $tag: ".Dumper(\%attrs);
    $osm_segments->{$attrs{id}}->{from}=$attrs{from};
    $osm_segments->{$attrs{id}}->{to}=$attrs{to};
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

    return if $k eq "created_by";

    if ($way_id) {
	if ( defined( $osm_ways->{$way_id}->{tag}->{$k} ) ) {
	    warn "Tag $k already exists for way($way_id)\n";
	} else {
	    $osm_ways->{$way_id}->{tag}->{$k} = $v;
	}
    }
}

# -----------------------------------------------------------------------------
sub read_osm_streets_area($) { # Insert Streets from osm File
    my $file_name = shift;

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
	print "OSM Nodes:    " . scalar keys( %$osm_nodes)."\n";
	print "OSM Segments: " . scalar keys( %$osm_segments)."\n";
	print "OSM Ways:     " . scalar keys( %$osm_ways)."\n";
    }
    return;
}

# -----------------------------------------------------------------------------
sub fill_osm_streets() { # Insert Streets from osm variables into mysql-db for gpsdrive
    print "Import new OSM Data\n";
    for my $seg_id ( sort keys %{$osm_segments} ) {
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
    $multi_segment->{'source_id'} = $osm_source_id;
    $multi_segment->{'streets_type_id'} = '';
    $multi_segment->{'streets_type_id'} = streets_type_name2id('Strassen.Allgemein');
    $max_allowed_dist = 0.05;
    
    $multi_segment->{'name'}            = $street_name;
    $multi_segment->{'scale_min'}       = 1;
    $multi_segment->{'scale_max'}       = 50*1000*1000;
    $multi_segment->{'segments'} =[];
    
    my $segment_count=0;
    my $lat_min= 1000;
    my $lat_max=-1000;
    my $lon_min= 1000;
    my $lon_max=-1000;
    
    for my $way_id ( sort keys %{$osm_ways} ) {
	my $way = $osm_ways->{$way_id};
	my $tags      = $way->{tag};
	#print Dumper(\$way);

	for my $seg_id ( @{$way->{seg}} ) {
	    $osm_segments->{$seg_id}->{connections} ++;
	    my $node_from = $osm_segments->{$seg_id}->{from};
	    my $node_to   = $osm_segments->{$seg_id}->{to};
	    #	print Dumper( $osm_segments->{$seg_id});
	    $lat1 = $osm_nodes->{$node_from}->{lat};
	    $lon1 = $osm_nodes->{$node_from}->{lon};
	    #print Dumper( $osm_nodes->{$node_to});
	    $lat2 = $osm_nodes->{$node_to}->{lat};
	    $lon2 = $osm_nodes->{$node_to}->{lon};
	    #print "Segment: $node_from:($lat1,$lon1) $node_to:($lat2,$lon2) \n";

	    # remember lat/lon Min/Max 
	    $lat_min= min($lat_min,$lat1);
	    $lat_max= max($lat_max,$lat1);
	    $lon_min= min($lon_min,$lon1);
	    $lon_max= max($lon_max,$lon1);

	    $lat_min= min($lat_min,$lat2);
	    $lat_max= max($lat_max,$lat2);
	    $lon_min= min($lon_min,$lon2);
	    $lon_max= max($lon_max,$lon2);
	    

	    # tags="class=motorway;name=A99;car=yes;"/>
	    my $street_name=$tags->{name} || "Way uid=$way_id";
	    my $streets_type_id;
	    if ( $tags->{class} && $tags->{class} eq "primary" ) {
		$streets_type_id = streets_type_name2id('Strassen.Bundesstrasse');
	    } elsif ( $tags->{highway}  ) {
		$streets_type_id = streets_type_name2id('Strassen.Autobahn');
	    } elsif ( $tags->{class} && $tags->{class} eq "motorway" ) {
		$streets_type_id = streets_type_name2id('Strassen.Autobahn');
	    } else {
		$streets_type_id = street_name_2_id($street_name);
	    }
	    die "unknown $streets_type_id" 
		unless $streets_type_id;
	    my $tags_string = join(" ",map{" $_:$tags->{$_} "}keys %$tags);
	    if ( 0 && $debug ) {
		print "Street Type ID: $streets_type_id\t";
		print "Street Name: $street_name\t";
		print "Street Tags: $tags_string\n";
	    }

	    $multi_segment->{'streets_type_id'} = $streets_type_id;

	    my $d_lat=abs($lat1-$lat2);
	    my $d_lon=abs($lon1-$lon2);
	    my $dist = $d_lat+$d_lon;
	    $max_dist = $dist if $dist>$max_dist;
	    if ( 0 &&
		 $dist > $max_allowed_dist 
		 ) {
		printf( "Dist: %.4f	($lat1,$lon1) -> ($lat2,lon2) ".
			"$street_name\n",$dist);
		#print "Splitting Track $street_name\n";
		#street_segments_add($multi_segment);
		#$multi_segment->{'segments'} =[];
	    }
	    
	    my $comment = "tags=tags_string ";
	    $comment .= "seg_id=$seg_id ";
	    $comment .= "Seg# : ".$osm_nodes->{$node_from}->{cluster} if defined ($osm_nodes->{$node_from}->{cluster});;
	    push(@{$multi_segment->{'segments'}},
	     {
		 'lat1' => $lat1, 'lon1' => $lon1, 'alt1' => -99,
		 'lat2' => $lat2, 'lon2' => $lon2, 'alt2' => -99,
		 'name' => $street_name." tags_string",
		 'comment' => $comment,
		 'streets_type_id' => $streets_type_id,
	     }
		 );
	    $segment_count++
	    }
	print "Writing Segments\n" if $debug;
	street_segments_add($multi_segment);
	$multi_segment->{'segments'}=[];
	print "lat($lat_min , $lat_max)	lon($lon_min , $lon_max)\n" 
	    if $verbose;
	
    }; # of fill_example_streets()
    debug("Imported $segment_count Segments for  ".scalar(keys(%{$osm_ways}))." osm Ways")
	if $debug;

    # ------------------------------------------------------------------
    my $rest_segments =  (scalar keys( %$osm_segments) ) - $segment_count;
    debug("Importing  $rest_segments unconnected Segments")
	if $debug;

    $segment_count=0;
    for my $seg_id ( keys %{$osm_segments} ) {
	next 
	    if defined($osm_segments->{$seg_id}->{connections})
	    && $osm_segments->{$seg_id}->{connections} > 0;
	$segment_count++;
	my $percent = $segment_count/$rest_segments*100;
	print "importing Segment $segment_count ( $percent %)\r" 
	    if $verbose && !($segment_count %100);

	my $node_from = $osm_segments->{$seg_id}->{from};
	my $node_to   = $osm_segments->{$seg_id}->{to};
	#	print Dumper( $osm_segments->{$seg_id});
	$lat1 = $osm_nodes->{$node_from}->{lat};
	$lon1 = $osm_nodes->{$node_from}->{lon};
	#print Dumper( $osm_nodes->{$node_to});
	$lat2 = $osm_nodes->{$node_to}->{lat};
	$lon2 = $osm_nodes->{$node_to}->{lon};
	my $segment4db = {
	    'streets.lat1' => $lat1, 'streets.lon1' => $lon1, 'streets.alt1' =>-99,
	    'streets.lat2' => $lat2, 'streets.lon2' => $lon2, 'streets.alt2' =>-99,
	    'streets.streets_type_id' => $multi_segment->{'streets_type_id'},
	    'streets.scale_min' => 1,
	    'streets.scale_max' => 1000000,
	    'streets.last_modified'  => time(),
	    'streets.source_id' => $osm_source_id,
	    'streets.name' => $street_name
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
sub delete_existing_entries(){
    print "Delete old OSM Data\n";

    my $source = "OpenStreetMap.org";

    Geo::Gpsdrive::DBFuncs::delete_all_from_source($source);
    $osm_source_id = Geo::Gpsdrive::DBFuncs::source_name2id($source);

    unless ( $osm_source_id ) {
	my $source_hash = {
	    'source.url'     => "http://openstreetmap.org/",
	    'source.name'    => $source ,
	    'source.comment' => '' ,
	    'source.licence' => ""
	    };
	Geo::Gpsdrive::DBFuncs::insert_hash("source", $source_hash);
	$osm_source_id = Geo::Gpsdrive::DBFuncs::source_name2id($source);
    }
    print "Deleted old OSM Data\n" if $verbose || $debug;
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


    #download_osm_streets($mirror_dir);

    print "Read OSM Data\n";
    #read_osm_streets($mirror_dir);
    read_osm_streets_area($tar_file);

    #read_osm_streets_area( "/home/tweety/.gpsdrive/MIRROR/osm-0.3/Streets_osm_040,0010,050,0020.xml");
    #read_osm_streets_area( "/home/tweety/.gpsdrive/MIRROR/osm-0.3/Streets_osm_040,0010,050,0020-test.xml");
    #read_osm_streets_area("/home/tweety/.gpsdrive/MIRROR/osm-0.3/Streets_osm_00,60,10,70.xml");
    #read_osm_streets_area("/home/tweety/.gpsdrive/MIRROR/osm-0.3/Streets_osm_040,0010,050,0020.xml");

    delete_existing_entries();

    fill_osm_streets();


    print "Download and import of OSM Data FINISHED\n";
}

1;
