#############################################################################
# Einlesen der von jiggle gecacheten Daten und
# schreiben in die POI Datenbank
#
# $Log$
# Revision 1.1  2005/08/15 13:54:22  tweety
# move scripts/POI --> scripts/Geo/Gpsdrive to reflect final Structure and make debugging easier
#
# Revision 1.5  2005/08/09 01:08:30  tweety
# Twist and bend in the Makefiles to install the DataDirectory more apropriate
# move the perl Functions to Geo::Gpsdrive in /usr/share/perl5/Geo/Gpsdrive/POI
# adapt icons.txt loading according to these directories
#
# Revision 1.4  2005/05/10 05:28:49  tweety
# type in disable_keys
#
# Revision 1.3  2005/04/10 00:15:58  tweety
# changed primary language for poi-type generation to english
# added translation for POI-types
# added some icons classifications to poi-types
# added LOG: Entry for CVS to some *.pm Files
#

package Geo::Gpsdrive::JiGLE;

use strict;
use warnings;

use IO::File;
use File::Basename;
use File::Path;
use Data::Dumper;

use Geo::Gpsdrive::DBFuncs;
use Geo::Gpsdrive::Utils;
use Geo::Gpsdrive::Gps;

use Date::Manip;
use Time::Local;

$|=1;


my $LINES_COUNT_FILE =0;
my $LINES_COUNT_UNPARSED =0;

# poi_type_id's
our $wlan_wep  =0;
our $wlan_open   = 0;
our $wlan_public = 0;

sub import_Jigle_file($$){
    my $full_filename = shift;
    my $source_id = shift;
    
    print "Reading $full_filename                   \n";

    my $fh = IO::File->new("<$full_filename");
    my $filename = basename($full_filename);

    $LINES_COUNT_FILE =0;
    while ( my $line = $fh->getline() ) {
	$LINES_COUNT_FILE ++;
	
	next if $line =~ m/trilat~trilong~ssid~netid~discoverer/;

#	print "line: $line\n";
	if ( $line =~ m/^\s*$/)  {
	} elsif ( $line =~ m/^trilat/ ) {
	} else {
#	    print "$line\n";
	    # trilat~trilong~ssid~netid~discoverer~channel~type~freenet~firsttime~wep~comment~qos~lastupdt~paynet~userfound
	    # 48.035843~10.747351~ ~00:01:E3:00:DD:71~ ~0~????~N~2004-02-20 03:27:45~N~ ~0~20040404125535~N~Y
	    # 48.029793~10.51143~ ~00:02:2D:39:9A:79~ ~0~????~N~2004-02-20 03:21:39~N~ ~0~20040404125620~N~Y

	    # trilat  ~trilong  ~ssid~netid            ~discoverer~channel~type~freenet~firsttime          ~wep~comment~qos~lastupdt      ~paynet~userfound
	    # 48.43025~10.281776~    ~00:02:2d:9c:77:66~          ~8192   ~BBS ~N      ~2004-03-07 10:32:13~Y  ~       ~0  ~20040511173722~N     ~N
	    # 0        1              2                 3          4       5    6       7                   8   9       0   1              2      3
	    my @line = split(/\~/,$line);

	    my $point;
	    $point->{'poi.lat'}        = $line[0];
	    $point->{'poi.lon'}        = $line[1];
	    $point->{'poi.ssid'}       = $line[2];
	    $point->{'poi.bssid'}      = $line[3];
	    $point->{'poi.name'}      = $point->{'poi.ssid'}."\n".$point->{'poi.bssid'};;
	    $point->{'poi.discoverer'} = $line[4];
	    $point->{'poi.channel'}    = $line[5];
	    $point->{'poi.type'}       = $line[6];
	    $point->{'poi.freenet'}    = $line[7];
	    $point->{'poi.last_modified'}      = $line[8];
	    $point->{'poi.wep'}        = $line[8];
	    $point->{'poi.comment'}      = $point->{'poi.bssid'};
	    $point->{'poi.date2'}      = $line[12];
	    $point->{'poi.scale_min'}  = 1;
	    $point->{'poi.scale_max'}  = 100000;
	    $point->{'poi.proximity'}  = 100;
	    
	    # Kismet: bssid time-sec time-usec lat lon alt spd heading fix signal quality noise

	    if ( $point->{'poi.wep'} ) {
		$point->{'poi.poi_type_id'} = $wlan_wep;
	    } else {
		$point->{'poi.poi_type_id'} = $wlan_open;
	    }

	    $point->{source_id} = $source_id;
	    correct_lat_lon($point);
	    Geo::Gpsdrive::DBFuncs::add_poi($point);
	    #print "Point:".Dumper(\$point);
	    
	}	    

    }

}

# *****************************************************************************
sub import_Data($){
    my $dir = shift;
    my $jigle_dir = $dir || "~/JiGLE/WiGLEnet/data";
    my $source = "JiGLE WLAN";


    my $source_id = Geo::Gpsdrive::DBFuncs::source_name2id($source);

    unless ( $source_id ) {
	my $source_hash = {
	    'source.url'     => "",
	    'source.name'    => $source ,
	    'source.comment' => 'JiGLE Wlan' ,
	    'source.licence' => "JiGLE"
	    };
	Geo::Gpsdrive::DBFuncs::insert_hash("source", $source_hash);
	$source_id = Geo::Gpsdrive::DBFuncs::source_name2id($source);
    }

    $wlan_wep    = poi_type_name2id("W-LAN.WEP");
    $wlan_open   = poi_type_name2id("W-LAN.Offen");
    $wlan_public = poi_type_name2id("W-LAN.Oeffentlich");


    Geo::Gpsdrive::DBFuncs::disable_keys('poi');

    delete_all_from_source($source);

    debug("$jigle_dir/*.autocache");
    foreach  my $full_filename ( glob("$jigle_dir/*.autocache") ) {
	import_Jigle_file($full_filename,$source_id)
	    if ( -s $full_filename > 110 );
    }
    Geo::Gpsdrive::DBFuncs::enable_keys('poi');
}

1;
