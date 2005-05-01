# Einlesen der WDB Daten und schreiben in die geodb Datenbank von 
# gpsdrive
#
# $Log$
# Revision 1.9  2005/05/01 13:49:36  tweety
# Added more Icons
# Moved filling with defaults to DB_Defaults.pm
# Added some more default POI Types
# Added icons.html to see which icons are used
# Added more Comments
# Reformating Makefiles
# Added new options for importing from way*.txt and adding defaults
# Added more source_id and type_id
#
# Revision 1.8  2005/04/13 19:58:30  tweety
# renew indentation to 4 spaces + tabstop=8
#
# Revision 1.7  2005/04/10 00:15:58  tweety
# changed primary language for poi-type generation to english
# added translation for POI-types
# added some icons classifications to poi-types
# added LOG: Entry for CVS to some *.pm Files
#

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
    my $base_filename = basename($full_filename);
    my $fh = IO::File->new("<$full_filename");
    my $segment=0;
    my $rank =0;
    my $points =0;
    my ($lat1,$lon1) = (0,0);
    my ($lat2,$lon2) = (0,0);

    my ( $sub_source ) = ( $base_filename =~ m/(.*).txt/ );
    my ( $country,$type_string) = ( $base_filename =~ m/(.*)-(.*).txt/);

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
    


    my $area_limit=0;
    if ( $main::lat_min ||
	 $main::lat_max ||
	 $main::lon_min ||
	 $main::lon_max ) {
	$area_limit=1;
    }




    my $streets_type_id=0;
    my $scale_max;
    my @segments;
    while ( my $line = $fh->getline() ) {
	chomp $line;
	#print "line: $line\n";
	if ( $line =~ m/^\s*$/)  {
	} elsif ( $line =~ m/^segment\s+(\d+)\s+rank\s+(\d+)\s+points\s+(\d+)/ ) {
	    $scale_max = 10000;
	    $scale_max = 100000	    if  $rank >1;
	    $scale_max = 1000000    if  $rank >2;
	    $scale_max = 10000000   if  $rank >3;
	    $scale_max = 100000000  if  $rank >4;
	    POI::DBFuncs::segments_add(
				   { scale_min       => 1,
				     scale_max       => $scale_max,
				     streets_type_id => $streets_type_id, 
				     source_id       => $source_id,
				     segments        => \@segments
				     }
				       );
	    @segments=();
	    # Segment: segment 27  rank 1  points 1131
	    ($segment,$rank,$points) = ( $1,$2,$3) ;
	    print "Segment: $segment, rank: $rank  points:$points\r";
	    print "\n" if $verbose;
	    ( $lat1,$lon1 ) = ( $lat2 , $lon2 ) = (0,0);

	    # ---------------------- Type    
	    my $type_name = "WDB $type_string rank $rank";
	    $streets_type_id = streets_type_name2id($type_name);
	    unless ( $streets_type_id ) {
		my $type_hash= {
		    'streets_type.name' => $type_name
		    };
		POI::DBFuncs::insert_hash("streets_type",$type_hash);
		$streets_type_id = streets_type_name2id($type_name);
	    }	
	} elsif ( $line =~ m/^\s*([\d\.\-]+)\s+([\d\.\-]+)\s*$/ ) {
	    ( $lat1,$lon1 ) = ( $lat2 , $lon2 );
	    ( $lat2,$lon2 ) = ($1,$2);
	    # 31.646111 25.148056
	    if ( $area_limit  && 
		 ( $lat2 < $main::lat_min || $lat2 > $main::lat_max ||
		   $lon2 < $main::lon_min || $lon2 > $main::lon_max 
		   )
		 ){
		#print "Skipping $lat2,$lon2\n";
		next;
	    } 


	    push(@segments,{
		lat=> $lat2, lon=>$lon2,
		name => "$rank : $segment : $points"
		});
	} else {
	    print "Unrecognized Line '$line'\n";
	}
    }
    POI::DBFuncs::segments_add( {
	scale_min       => 1,
	scale_max       => $scale_max,
	streets_type_id => $streets_type_id, 
	source_id       => $source_id,
	segments        => \@segments
	} );
}




# *****************************************************************************
sub import_Data(){
    my $mirror_dir="$main::MIRROR_DIR/wdb";
    my $unpack_dir="$main::UNPACK_DIR/wdb";

    -d $mirror_dir or mkpath $mirror_dir
	or die "Cannot create Directory $mirror_dir:$!\n";
    
    -d $unpack_dir or mkpath $unpack_dir
	or die "Cannot create Directory $unpack_dir:$!\n";
    
    my $url = "http://www.evl.uic.edu/pape/data/WDB/WDB-text.tar.gz";
    my $tar_file = "$mirror_dir/WDB-text.tar.gz";
    my $mirror = mirror_file($url,$tar_file);

    my $dst_file="$unpack_dir/WDB/europe-bdy.txt";
    if ( (!-s $dst_file) ||
	 file_newer($tar_file,$unpack_dir ) ) {
	print "Unpacking $tar_file\n";
	`(cd $unpack_dir/; tar -xvzf $tar_file)`;
    } else {
	print "unpack: $dst_file up to date\n" unless $verbose;
    }
    
    POI::DBFuncs::disable_keys('streets');
    debug("$unpack_dir/WDB/*.txt");
    foreach  my $full_filename ( glob("$unpack_dir/WDB/euro*.txt") ) {
	# print "Mirror: $mirror\n";
	
	import_wdb($full_filename);
    }
    POI::DBFuncs::enable_keys('streets');
}

1;
