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
#############################################################################
sub read_open_geo_db($){
    my $full_filename = shift;

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
		   address.state address.bundesland address.regierungsbezirk 
		   address.landkreis address.verwaltungszusammenschluss
		   address.ort   address.ortsteil address.gemeindeteil
		   poi.lat poi.lon
		   poi.autokennzeichen
		   address.plz);
    my $lines_count_file =0;
    print "  ". join("\t",@columns)."\n";
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
		$values->{'poi.comment'} .= "$values[$i]\n" if $i>2;
	    }

	    ############################################
	    # Set Default Proximity 
	    $values->{'poi.proximity'} ||= "1000 m";

	    $values->{'poi.symbol'} ||= "City";


	    my $first_in_a_row=1;
	    for my $plz ( split(',',$values->{'address.plz'})) {
		#	    print Dumper($values);
		my $wp_name = '';
		$wp_name .= "$values->{'address.state'}-";
		$wp_name .= $plz;
		#	    $wp_name .= "_$values->{'poi.primarykey'}";
		#	    $wp_name .= "_$values->{'address.regierungsbezirk'}";
		#	    $wp_name .= "_$values->{'address.landkreis'}";
		#	    $wp_name .= "_$values->{'address.verwaltungszusammenschluss'}";
		$wp_name .= " $values->{'address.ort'}\n";
		$wp_name .= " $values->{'address.bundesland'}";
		$wp_name .= " $values->{'address.ortsteil'}";
		$wp_name .= " $values->{'address.gemeindeteil'}";
		$values->{'poi.name'}=$wp_name;
		if (  $plz =~ m/000$/ ) {
		    print "$values->{'address.state'}-$plz $values->{'address.ort'}\n";
		    $values->{'poi.scale_max'}=1000000000;
		    $values->{'poi.proximity'} = "10000m";
		} elsif ( $values->{'address.ortsteil'}         eq "-" && 
			  $values->{'address.gemeindeteil'}     eq "-" &&
			  $values->{'address.regierungsbezirk'} eq "-" &&
			  $values->{'address.verwaltungszusammenschluss'} eq "-" &&
			  $first_in_a_row
			  ) {
		    print "$values->{'address.state'}-$plz :". join("\t",@values)."\n";
		    $values->{'poi.scale_max'}=100000000;
		    $values->{'poi.proximity'} = "5000m";
		    $first_in_a_row=0;
		} elsif ( $plz =~ m/00$/ ) {
		    $values->{'poi.scale_max'}=5000000;
		    $values->{'poi.proximity'} = "5000m";
		} elsif (  $plz =~ m/0$/ ) {
		    $values->{'poi.scale_max'}=1000000;
		    $values->{'poi.proximity'} = "1000m";
		} else {
		    $values->{'poi.scale_max'}=100000;
		    $values->{'poi.proximity'} = "300m";
		}
		$values->{'poi.scale_min'}=0;
		
		$values->{'poi.name'}.=$values->{'poi.scale_max'};
		unless ( defined($values->{'poi.lat'}) ) {
		    print "Undefined lat".Dumper(\$values);
		}
		unless ( defined($values->{'poi.lon'}) ) {
		    print "Undefined lon".Dumper(\$values);
		}

		$values->{'poi.source_id'}=$source_id;
		
		correct_lat_lon($values);
		POI::DBFuncs::add_poi($values);
		#print "Values:".Dumper(\$values);
	    }
	}
    }
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

    POI::DBFuncs::disble_keys('poi');

    for my $file_name ( glob("$unpack_dir/opengeodb*.txt") ) {
	my $out_file_name = "$main::CONFIG_DIR/way_opengeodb.txt";
	read_open_geo_db($file_name);
    }

    POI::DBFuncs::enable_keys('poi');

}

1;
