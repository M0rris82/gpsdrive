# Einlesen der WDB Daten und schreiben in die geodb Datenbank von 
# gpsdrive

package POI::WDB;

use strict;
use warnings;

use IO::File;
use File::Basename;
use File::Path;
use Data::Dumper;

use POI::DBFuncs;
use POI::Utils;

$|=1;

my $LINES_COUNT_FILE =0;
my $LINES_COUNT_UNPARSED =0;

##################################################################
# Alle Punkte rausschreiben
# Args:
#    $fo : Filedescriptor to write t
#    @{$points_in_segment} : List of points
sub write_points($$){
    my $fo = shift;
    my $points_in_segment = shift;
    if ( @{$points_in_segment} ) {
	for my $point ( @{$points_in_segment} , $points_in_segment->[0]  ) {
	    #$fo->point($point);
	    #$fo{$rank}->point($point) if $rank;
	    print $fo "$point->{lat} $point->{lon}\n";
	}
	print $fo "1001.0 1001.0\n";
    }
}

##########################################################################

sub import_wdb($){
    my  $full_filename = shift;
    print "Reading $full_filename                   \n";
    my $out_filename = basename($full_filename);
    $out_filename =~ s/\.txt/.sav/g;
    my ($country) = ($out_filename =~ m/(.*)-/);

    my $fh = IO::File->new("<$full_filename");
    my $segment=0;
    my $rank =0;
    my ($lat1,$lon1) = (0,0);
    my ($lat2,$lon2) = (0,0);
    my $type = 0;

    $type = 1 if ( $full_filename =~ m/-bdy/ );
    $type = 2 if ( $full_filename =~ m/-cil/ );
    $type = 3 if ( $full_filename =~ m/-riv/ );
    my ( $sub_source ) = ( $full_filename =~ m/([^\/]+).txt/ );

    my $source = "WDB $sub_source";
    delete_all_from_source($source);
    my $source_id = POI::DBFuncs::source_name2id($source);

    unless ( $source_id ) {
	my $source_hash = {
	    'source.url'     => "http://www.evl.uic.edu/pape/data/WDB/WDB-text.tar.gz",
	    'source.name'    => $source ,
	    'source.comment' => '' ,
	    'source.licence' => ""
	    };
	POI::DBFuncs::insert_hash("source", $source_hash);
	$source_id = POI::DBFuncs::source_name2id($source);
    }
    


    while ( my $line = $fh->getline() ) {
	chomp $line;
	#print "line: $line\n";
	if ( $line =~ m/^\s*$/)  {
	} elsif ( $line =~ m/^segment\s+(\d+)\s+rank\s+(\d+)/ ) {
	    # Segment: segment 27  rank 1  points 1131
	    ($segment,$rank) = ( $1,$2) ;
	    print "Segment: $line, rank: $rank  \r";
	    ( $lat1,$lon1 ) = ( $lat2 , $lon2 ) = (0,0);
	} elsif ( $line =~ m/^\s*([\d\.\-]+)\s+([\d\.\-]+)\s*$/ ) {
	    # 31.646111 25.148056

	    ( $lat1,$lon1 ) = ( $lat2 , $lon2 );
	    ( $lat2,$lon2 ) = ($1,$2);


	    if ( $lat1 && $lon1 ) {
		POI::DBFuncs::streets_add(
					  { lat1 => $lat1, lon1 => $lon1,
					    lat2 => $lat2, lon2 => $lon2,
					    level_min => 0, level_max => 99,
					    type_id => $type, 
					    source_id => $source_id
					    }
					  );
	      }
		  
	} else {
	    print "Unrecognized Line '$line'\n";
	}
    }
}




# *****************************************************************************
sub import_Data(){

    my $wdb_dir="$main::CONFIG_DIR/MIRROR/wdb";
    
    unless ( -d $wdb_dir ) {
	print "Creating Directory $wdb_dir\n";
	mkpath $wdb_dir
	    or die "Cannot create Directory $wdb_dir:$!\n";
    }
    

    my $url = "http://www.evl.uic.edu/pape/data/WDB/WDB-text.tar.gz";
    my $tar_file = "$wdb_dir//WDB-text.tar.gz";
    my $mirror = mirror_file($url,$tar_file);

    my $dst_file="$wdb_dir/WDB/*.txt";
    if ( (!-s $dst_file) ||
	 file_newer($tar_file,$dst_file ) ) {
	print "Unpacking $tar_file\n";
	`(cd $wdb_dir/; tar -xvzf $tar_file)`;
    } else {
	print "unpack: $dst_file up to date\n" unless $verbose;
    }
    
    debug("$wdb_dir/WDB/*.txt");
    foreach  my $full_filename ( glob("$wdb_dir/WDB/euro*.txt") ) {
	# print "Mirror: $mirror\n";
	
	import_wdb($full_filename);
    }
}

1;
