package POI::OpenGeoDB;

use strict;
use warnings;

use IO::File;
use POI::DBFuncs;
use POI::Utils;
use File::Path;

#############################################################################
# Args: 
#    $filename : Filename to read 
# returns:
#    $waypoints : Hash of read Waypoints
#############################################################################
sub read_open_geo_db($){
    my $full_filename = shift;

    my $waypoints={};
    print "Reading open geo db: $full_filename\n";
    my $fh = IO::File->new("<$full_filename");
    $fh or die ("read_open_geo_db: Cannot open $full_filename:$!\n");

    my $source = "open geo db";
    delete_all_from_source($source);
    my $source_id = POI::DBFuncs::source_name2id($source);

    unless ( $source_id ) {
	my $source_hash = {
	    'source.url'     => "http://ovh.dl.sourceforge.net/".
		"sourceforge/geoclassphp/opengeodb-0.1.3-txt.tar.gz",
	    'source.name'    => $source ,
	    'source.comment' => '' ,
	    'source.licence' => ""
	    };
	POI::DBFuncs::insert_hash("source", $source_hash);
	$source_id = POI::DBFuncs::source_name2id($source);
    }

    my @columns;
    @columns = qw( primarykey 
		   address.staate address.bundesland address.regierungsbezirk 
		   address.landkreis address.verwaltungszusammenschluss
		   address.ort   address.ortsteil address.gemeindeteil
		   poi.lat poi.lon
		   poi.autokennzeichen
		   address.plz);
    my $lines_count_file =0;
    while ( my $line = $fh->getline() ) {
	$lines_count_file ++;
	$line =~ s/[\t\r\n\s]*$//g;;
#	 print "line: '$line'\n";
	if ( $line =~ m/^$/ ) {
	} elsif ( $line =~ m/^\#/ ) {
	} else {
	    die "Spalten nicht definiert" unless @columns;
	    
#	    print "WP: $line\n";
	    my @values = split(/\s*\;\s*/,$line);
	    #print Dumper(\@values);
	    my $values;
	    for my $i ( 0 .. scalar @columns -1 ) {
		$values->{$columns[$i]} = $values[$i];
		$values->{'poi.comment'} .= ",$values[$i]";
	    }

	    ############################################
	    # Set Default Proximity 
	    $values->{'poi.proximity'} ||= "1000 m";

	    $values->{'poi.symbol'} ||= "City";


	    for my $plz ( split(',',$values->{'address.plz'})) {
		#	    print Dumper($values);
		my $wp_name = $plz;
		#	    $wp_name .= "_$values->{'poi.primarykey'}";
		#	    $wp_name .= "_$values->{'address.staat'}";
		$wp_name .= "_$values->{'address.bundesland'}";
		#	    $wp_name .= "_$values->{'address.regierungsbezirk'}";
		#	    $wp_name .= "_$values->{'address.landkreis'}";
		#	    $wp_name .= "_$values->{'address.verwaltungszusammenschluss'}";
		$wp_name .= "_$values->{'address.ort'}";
		$wp_name .= "_$values->{'address.ortsteil'}";
		$wp_name .= "_$values->{'address.gemeindeteil'}";
		$values->{'poi.name'}=$wp_name;
		if (  $plz =~ m/000$/ ) {
		    print "$plz $values->{'address.ort'} $values->{'address.ortsteil'} $values->{'address.gemeindeteil'}\n";
		    $values->{'poi.level_min'}=0;
		} elsif (  $plz =~ m/00$/ ) {
		    $values->{'poi.level_min'}=22;
		} elsif (  $plz =~ m/0$/ ) {
		    $values->{'poi.level_min'}=44;
		} else {
		    $values->{'poi.level_min'}=55;
		}
		$values->{'poi.level_max'}=99;
		
		unless ( defined($values->{'poi.lat'}) ) {
		    print "Undefined lat".Dumper(\$values);
		}
		unless ( defined($values->{'poi.lon'}) ) {
		    print "Undefined lon".Dumper(\$values);
		}

		$values->{'poi.source_id'}=$source_id;
		
		correct_lat_lon($values);
		$waypoints->{$wp_name} = $values;
		#print "Values:".Dumper(\$values);
	    }
	}
    }
    return $waypoints;
}

########################################################################################
# Get and Unpack opengeodb
# http://www.opengeodb.de/download/
########################################################################################
sub import_Data() {
    my $mirror_dir="$main::MIRROR_DIR/opengeodb";
    my $unpack_dir="$main::UNPACK_DIR/opengeodb";

    -d $mirror_dir or mkpath $mirror_dir
	or die "Cannot create Directory $mirror_dir:$!\n";
    
    -d $unpack_dir or mkpath $unpack_dir
	or die "Cannot create Directory $unpack_dir:$!\n";
    
    # download
    my $mirror = mirror_file("http://ovh.dl.sourceforge.net/".
			     "sourceforge/geoclassphp/opengeodb-0.1.3-txt.tar.gz",
			     "$mirror_dir/opengeodb-0.1.3-txt.tar.gz");

    print "Mirror: $mirror\n";

    # Unpack it 
    `(cd $unpack_dir/; tar -xvzf $mirror_dir/opengeodb-0.1.3-txt.tar.gz)`;

    for my $file_name ( glob("$unpack_dir/opengeodb*.txt") ) {
	my $out_file_name = "$main::CONFIG_DIR/way_opengeodb.txt";
	my $waypoints = read_open_geo_db($file_name);
#	write_gpsdrive_waypoints($waypoints,$out_file_name);
	POI::DBFuncs::add_poi_multi($waypoints);
    }

}

1;
