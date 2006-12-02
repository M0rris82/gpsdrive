#!/usr/bin/perl
# gpsfetchmap
#
# You are allowed to modify the source code in any way you want 
# except you cannot modify this copyright details
# or remove the polite feature.
#
# NO WARRANTY.

BEGIN {
    unshift(@INC,"/opt/gpsdrive");
}

my $VERSION ="gpsfetchmap (c) 2002 Kevin Stephens <gps\@suburbialost.com>
modified (Sep 2002) by Sven Fichtner <sven.fichtner\@flugfunk.de>
modified (Nov 2002) by Magnus Månsson <ganja\@0x63.nu>
modified (Nov 2003) by camel <camel\@insecure.at>
modified (Feb 2004) by Robin Cornelius <robin\@cornelius.demon.co.uk>
modified (Jan 2005) by Joerg Ostertag <gpsdrive\@ostertag.name>
modified (May 2005) by Olli Salonen <olli\@cabbala.net>
modified (Jul 2005) by Jaroslaw Zachwieja <grok\@filippa.org.uk>
modified (Dec 2005) by David Pollard <david dot pollard\@optusnet.com.au>
Version 1.19 (gpsdrive-2.10pre3-work)
";

sub redirect_ok { return 1; }


use IO::Socket;
use Data::Dumper;
use strict;
use warnings;
use LWP::UserAgent;
use LWP::Debug qw(- -conns -trace);
#use LWP::Debug qw(+ +conns +trace);
use HTTP::Request;
use HTTP::Request::Common;
use HTML::Parser;
use Getopt::Long;
use Pod::Usage;
use File::Basename;
use File::Path;
use File::Copy;
use IO::File;
use Switch;
use POSIX qw(ceil floor);
use Image::Magick;

my $long_sleep_time=600;

# For Expedia
my @SCALES = (1000,1500,2000,3000,5000,7500,10000,15000,20000,30000,50000,75000,
              100000,150000,200000,300000,500000,750000,1000000,1500000,2000000,3000000,
              5000000,7500000,10000000,15000000,20000000,30000000,50000000,75000000);

# Translates Scale to zoom factor used by Mapserver
# This Variable also is used to see if the required mapserver
# is a valid Mapserver source
my $Scale2Zoom = { 
    expedia => {
	12000*3950 => 12000,
	 7000*3950 =>  7000,
	 2000*3950 =>  2000,
	  800*3950 =>   800,
	  150*3950 =>   150,
	   50*3950 =>	 50,
	   25*3950 =>	 25,
	   12*3950 =>	 12,
	    6*3950 =>     6,
	    3*3950 =>	  3,
	    1*3950 =>	  1,
    },
    googlesat => {
	5708800 =>  7,  #Modified Scale values
	2854400 =>  8,
	1427200 =>  9,
	713600  => 10,
	356800  => 11,
	178400  => 12,
	89200   => 13,
	44600   => 14,
	22300   => 15,
	11200   => 16,
	5600    => 17,
	2825    => 18,
	1412    => 19,
    },
    eniro => {
	18000000 => 1, # 480     Km
	3200000  => 2, #  80     Km
	400000   => 3, #  10     Km
	200000   => 4, #   4     Km
	100000   => 5, #   1.1   Km
	20000    => 6, #     400 m
	5000     => 7, #     160 m
    },
    gov_au => {
	10000000000 => 1, # 
	5000000000  => 2, # 
	1000000000  => 3, # 
	500000000   => 4, # 
	100000000   => 5, # 
	50000000    => 6, #
	10000000    => 7, # 
	5000000     => 8, # 
	1000000     => 9, # 
	500000      => 10, # 
	100000      => 11, # 
	50000       => 12, #  
	10000       => 13, #  
	5000        => 14, # 
	1000        => 15, # 
    },

    incrementp => {
	3200000 => 0,  # 500     Km
	2400000 => 1,  # 250     Km
	1600000 => 2,  # 100     Km
	800000  => 3,  #  50     Km
	400000  => 4,  #  25     Km
	200000  => 5,  #  10     Km
	100000  => 6,  #   5     Km
	50000   => 7,  #   2.5   Km 
	25000   => 8,  #   1.0   Km
	2500    => 9,  #     500 m
	6250    => 10, #     250 m
	3125    => 11, #     100 m
	1562    => 12, #      50 m
    },
    geoscience => {
   	383000*8 => 8,   # DP Still not correct but closer
   	383000*4 => 4,
   	383000*2 => 2,
   	383000*1 => 1,
    },
    landsat => {
   	  2000 =>    2*6.4,
   	  5000 =>    5*6.4,	
   	 10000 =>   10*6.4,
   	 50000 =>   50*6.4, 
   	100000 =>  100*6.4,
   	500000 =>  500*6.4,   
       1000000 => 1000*6.4,
       5000000 => 5000*6.4,
      10000000 =>10000*6.4,
      50000000 =>50000*6.4,
    }
};

# Set defaults and get options from command line
Getopt::Long::Configure('no_ignore_case');
my ($lat,$lon);
my ($slat,$endlat,$slon,$endlon);
my ($waypoint,$area,$unit,$mapdir);
my ($debug,$force,$version,$man,$help);
my $cover_route;
my $failcount           = 0;
my $newcount            = 0;
my $existcount          = 0;
my $polite              = 1;
my $MIN_MAP_BYTES       = 4000;   # Minimum Bytes for a map to be accepted as downloaded
my $scale;
my $CONFIG_DIR          = "$ENV{'HOME'}/.gpsdrive"; # Should we allow config of this?
my $CONFIG_FILE         = "$CONFIG_DIR/gpsdriverc";
my $WAYPT_FILE          = "$CONFIG_DIR/way.txt";
my $GPSTOOL_MAP_FILE    = "$ENV{'HOME'}/.gpsmap/maps.txt";
my $FILEPREFIX          = 'map_';
my $mapserver           = 'expedia';
my $simulate_only       = 0;
my $check_koord_file    = 0;
my $update_koord        = 0;
my $check_coverage      = 0;
my $TRACK_FILE          = "";
our $MAP_KOORDS         = {};
our $MAP_FILES          = {};
our $GPSTOOL_MAP_KOORDS = {};
our $GPSTOOL_MAP_FILES  = {};
my $PROXY=$ENV{'http_proxy'};

GetOptions ( 'lat=f'     => \$lat,        'lon=f'       => \$lon, 
	     'start-lat=f'    => \$slat,       'end-lat=f'   => \$endlat, 
	     'start-lon=f'    => \$slon,       'end-lon=f'   => \$endlon, 
	     'sla=f'          => \$slat,       'ela=f'       => \$endlat, 
	     'slo=f'          => \$slon,       'elo=f'       => \$endlon, 
	     'scale=s'        => \$scale,      'mapserver=s' => \$mapserver, 
	     'waypoint=s'     => \$waypoint,   'area=s'      => \$area, 
	     'unit=s'         => \$unit,       'mapdir=s'    => \$mapdir, 
	     'polite:i'       => \$polite,
	     'WAYPOINT=s'     => \$WAYPT_FILE, 'CONFIG=s'    => \$CONFIG_FILE, 
	     'PREFIX=s'       => \$FILEPREFIX,
	     'n'              => \$simulate_only,
	     'track=s'        => \$TRACK_FILE,
	     'route'          => \$cover_route,
	     'check-koordfile'=> \$check_koord_file,
	     'check-coverage' => \$check_coverage,
	     'U'              => \$update_koord,
	     'FORCE'          => \$force,     
	     'PROXY=s'        => \$PROXY,
	     'debug+'          => \$debug,      'MAN' => \$man, 
	     'help|x'         => \$help,       'version' => \$version
	     )
    or pod2usage(1);

if ( $mapserver eq 'geoscience' )
{
    $scale ||= join(",",keys %{$Scale2Zoom->{'geoscience'}});
} else {
    $scale ||= '100000-5000000';
}

if ( ! defined ( $Scale2Zoom->{$mapserver} ) ){
    print "Invalid Mapserver: $mapserver\n";
    print "Valid Mapserver: ".join(",",keys %{$Scale2Zoom} )."\n";
    exit;
}

if ( $mapserver eq 'googlesat') {
    $MIN_MAP_BYTES = 1000; # Small Tiles
    $FILEPREFIX          = 'top_';
}

if ( $mapserver eq 'landsat') {
    $FILEPREFIX          = 'top_';
    $FILEPREFIX          = 'map_';
}

@SCALES = sort  {$a <=> $b} keys %{$Scale2Zoom->{$mapserver}};

# for google stitch
my %font_description = (font=> "Arial-Bold", pointsize=>12, fill=>"yellow");
my @quadChars = qw/t q s r/;
my $PI = 3.14159265358979323846;
my $MERCATOR_TOP = 85.051125;
my $MERCATOR_BOTTOM = -85.051125;


pod2usage(1) if $help;
pod2usage(-verbose=>2) if $man;

sub append_koords($$$$);   # {}
sub check_coverage($);     # {}
sub check_koord_file($);   # {}
sub debug($);              # {}
sub expedia_url($$$);      # {}
sub geoscience_url($$$);   # {}
sub landsat_url($$$);   # {}
sub resize($$); #{}
sub file_count($);         # {}
sub get_coords_for_route;  # {}
sub get_coords_for_track($); # {}
sub get_waypoint($);       # {}
sub is_map_file($);        # {}
sub read_gpstool_map_file(); # {}
sub read_koord_file($);    # {}
sub update_gpsdrive_map_koord_file(); # {}
sub wget_map($$$);         # {}
sub get_gpsd_position();       # {}

STDERR->autoflush(1);
STDOUT->autoflush(1);

# Setup up some constants
my $DIFF          = 0.0000028;
my $RADIUS_KM     = 6371.01;
my $LAT_DIST_KM   = 110.87;
my $KM2NAUTICAL   = 0.54;
my $KM2MILES      = 0.62137119;


my $CFG = read_config();
# Get unit from config file, unless they override with command line
$unit ||= $CFG->{units};

# Get mapdir from config file, unless they override with command line
$mapdir ||= $CFG->{mapdir};
$mapdir ||= "$CONFIG_DIR/maps";

my $KOORD_FILE  = "$mapdir/map_koord.txt";


debug( $VERSION);
#############################################################################
# LPW::UserAgent initialisieren
my $ua = LWP::UserAgent->new;
if ( $PROXY ) {
    $PROXY =~ s,^(http|ftp)://,,;
    $PROXY =~ s,/$,,;
    $PROXY = "http://$PROXY/";
    debug("Using Proxy '$PROXY'");
    $ua->proxy(['http','ftp'],$PROXY);
}
#$ua->level("+trace") if $debug;


# Print version
if ($version) {
    print $VERSION, "\n";
    exit();
}


# Verify that we have the options that we need 
pod2usage(1) if (&error_check);

# Change into the gpsdrive maps directory 
chdir($mapdir);

############################################
if ( $check_koord_file ) {
    check_koord_file($KOORD_FILE); # This also memoizes the filenames
    update_gpsdrive_map_koord_file();
    exit();
}

if ( $check_coverage ) {
    check_coverage($KOORD_FILE); 
    exit();
}

#############################################################################
# Get the list of scales we need
my $SCALES_TO_GET_ref = get_scales(\$scale);
debug( "Scale to download: ". join(",",sort {$a <=> $b} @{$SCALES_TO_GET_ref}));

# Get the center waypoint if they want one
if ( $waypoint ) {
    if ( $waypoint eq "gpsd" ) {
	($lat,$lon) = get_gpsd_position();
    } else {
	($lat,$lon) = get_waypoint($waypoint);
    }
    debug("Centerpoint: $lat,$lon");
}


my $desired_locations = {};

if ($TRACK_FILE) { # download maps along a saved track
    my ($number, $i);
    print "Downloading maps along a saved track $TRACK_FILE.\n" if $debug;
    my ($latsref, $lonsref) = get_coords_for_track($TRACK_FILE);
    my ($sla, $slo, $ela, $elo);
    $area ||= 1;
    $number = @{$latsref};
    print "Inserting $number points to the list.\n" if $debug;
    for ($i = 0; $i < $number; $i++) {
	($sla,$slo,$ela,$elo) = get_coords($latsref->[$i],$lonsref->[$i],$area,$unit);
	debug("");
	desired_locations($desired_locations,$sla,$slo,$ela,$elo);
    }
} elsif ($cover_route) { # download maps between a set of waypoints
    print "Downloading maps along the following route:\n" if $debug;
    my ($latsref, $lonsref) = get_coords_for_route;
    my ($sla, $slo, $ela, $elo, $number, $i);
    $area ||= 1;
    $number = @{$latsref};
    print "Inserting $number points to the list.\n" if $debug;
    for ($i = 0; $i < $number; $i++) {
	($sla,$slo,$ela,$elo) = get_coords($latsref->[$i],$lonsref->[$i],$area,$unit);
	desired_locations($desired_locations,$sla,$slo,$ela,$elo);
    }
} else { # we are not downloading maps a saved track or between waypoints
    # Now get the start and end coordinates
    unless ($slat && $slon && $endlat && $endlon) {
        ($slat,$slon,$endlat,$endlon) = get_coords($lat,$lon,$area,$unit); 
    }
    print "Upper left:  $slat, $slon\n" if $debug;
    print "Lower right: $endlat, $endlon\n" if ($debug);

    desired_locations($desired_locations,$slat,$slon,$endlat,$endlon);
}

my ($existing,$wanted) = file_count($desired_locations);
print "You are about to download $wanted (".($existing+$wanted).") file(s).\n";

if ( $mapserver eq 'geoscience' ){
    print "+-----------------------------------------------------------+\n";
    print "| Geoscience Maps are Copyright, Commonwealth of Australia  |\n";
    print "| They are free for non commercial use.                     |\n";
    print "| Full Copyright information can be found at                |\n";
    print "| http://www.ga.gov.au/about/copyright.jsp                  |\n";
    print "+-----------------------------------------------------------+\n";
}elsif ( $mapserver eq 'landsat' ){
    print "+-----------------------------------------------------------+\n";
    print "| Landsat Maps are Copyright, .....   |\n";
    print "| They are free for non commercial use.                     |\n";
} elsif ( ! $force) {
    print "You are violating the map servers copyright!\n";
    print "Are you sure you want to continue? [y|n] ";
    my $answer = <STDIN>;
    exit if ($answer !~ /^[yY]/);
}

if ( $update_koord  ) {
    read_gpstool_map_file();
    read_koord_file($KOORD_FILE);
}

print "\nDownloading files:\n";

# Ok start getting the maps
for my $scale ( sort {$b <=> $a} keys %{$desired_locations} ) {
    print "Scale: $scale\n";
    for my $lati ( sort {$a <=> $b} keys %{$desired_locations->{$scale}} ) {
	printf "   %5.2f: ",$lati;
	my @longs = sort  {$a <=> $b} keys %{$desired_locations->{$scale}->{$lati}};
	print "(". scalar( @longs ) . ")\t";
	#print ":". join(" ", @longs ) . "\t";
	for my $long ( @longs ) {
	    print wget_map($scale,$lati,$long);
	}
	print "\n";
    }
#print "\n";
}


print "\n";

print "Fail:  $failcount\n";
print "Exist: $existcount\n";
print "New:   $newcount\n";

################################################################################
#
# Subroutines
#
################################################################################

#############################################################################
sub slurp($){
    my $fh=IO::File->new(shift);
    return "" unless $fh;
    return join("",$fh->getlines());
}

#############################################################################
sub get_gpsd_position(){
    # Connect to local gpsd
    my $GPSD = "localhost:2947";
    my $gpsd = IO::Socket::INET->new($GPSD);
    my ($lat,$lon) =(0,0);
    if ( !$gpsd ) {
	warn "No local gpsd found at $GPSD: $!\n";
    } else {
	# Tell gpsd we want position, altitude, date, and status.
	$gpsd->print("pads\n");
	$gpsd->print("p\n");
	while ( my $line = <$gpsd> ) {
	    debug("GPSD Line: $line");
	    if ( $line =~ m/P=(-?\d+\.\d+)\s+(-?\d+\.\d+)/){
		($lat,$lon) =($1,$2);
		debug("gpsd reports: lat/lon ($lat,$lon)");
		return ($lat,$lon);
	    } else {
		print "GPSD Line: $line";
		$gpsd->print("pads\n");
		sleep 1;
	    }
	}
    }
    $gpsd->close();
}

#############################################################################
# get File with lwp-mirror
#############################################################################
sub mirror_file($$){
    my $url            = shift;
    my $local_filename = shift;

    my $file_existed = -s $local_filename;

    my $exist_string = $file_existed?"exists":"getting";
    debug("mirror_file($url --> $local_filename) $exist_string");

    my $ok=1;

    my $dir =  dirname($local_filename);
    unless ( -s $dir || -l $dir ) {
	debug("Create Directory '$dir'");
	mkpath($dir)
	    or warn "Could not create '$dir':$!\n";;
    }

    my $response = $ua->mirror($url,$local_filename);
#    debug(sprintf("success = %d <%s>",$response->is_success,$response->status_line));
    
    if ( ! $response->is_success ) {
	if ( $response->status_line =~ /^304/ ) {
	    print "$url --> $local_filename\tNOT MOD" if $debug ;
	} else {
	    print sprintf("ERROR: %s\n",$response->message)
		if $debug;
	    print sprintf("STATUS LINE: %s\n",$response->status_line)
		if $debug;
	    print "COULD NOT GET\t$url\n\t\tfor --> $local_filename\n";
	    unlink $local_filename unless $file_existed;
	    $ok=0;
	}
    }    
    debug("mirror_file($url --> $local_filename) error ")
	unless $ok;
    return $ok;
}

######################################################################
# Check if $filename is a valid map image
# for now we only check the size and existance
######################################################################
sub is_map_file($){
    my $filename = shift;
    my $full_filename = $filename;
    $full_filename = "$mapdir/$filename" unless $filename =~ m,^/,;
    if (  $debug ) {
#	printf "is_map_file($full_filename) -> %d Bytes\n",-s $full_filename;
    }

    return 1 if ( -s $full_filename || 0  ) > $MIN_MAP_BYTES ;

    return 0 if -s $full_filename;

    # Search in System 
    $full_filename = "/usr/local/share/gpsdrive/maps/$filename" unless $filename =~ m,^/,;
    return 1 if ( -s $full_filename || 0  ) > $MIN_MAP_BYTES ;

    $full_filename = "/usr/share/gpsdrive/maps/$filename" unless $filename =~ m,^/,;
    return 1 if ( -s $full_filename || 0  ) > $MIN_MAP_BYTES ;
    
    return 0;
}


######################################################################
# get a single map
# Args:
#     $url
#     $local_filename
# Returns:
#     1 : Success
#     0 : Failure
######################################################################
sub mirror_map($$){
    my $url = shift;
    my $filename = shift;

    my $content = slurp($filename);
    if ( $content =~ m,<title>403 Forbidden</title>,){
	unlink "$filename";
	sleep 10; # Yes this is on purpose. SInce you cannot download for the next hours anyway
	debug($content);
	die ("Access to the Site $url is denied\n");
    };
    my $ok = mirror_file($url,$filename);
    if ( ! is_map_file($filename) ) {
	# unlink($filename) if -s $filename;
	#warn "no map downloaded ($filename)\n";
	$ok=0;
    }

    $content = slurp($filename);
    if ( $content =~ m,<title>(403 .*)</title>,){
	my $message = $1;
	$content =~ s,.*<blockquote>,,s;
	$content =~ s,</blockquote>.*,,s;
	warn "Found $filename $message Message\n";
	warn $content;
	warn "Sleeping very long $long_sleep_time seconds\n";
	sleep $long_sleep_time;
	$long_sleep_time*=2;
    };


    # sleep if polite is turned on to be nice to the webserver of the mapserver
    sleep($polite) if ($polite =~ /\d+/);

    return $ok;
}

######################################################################
# get a single map (Old Version with wget)
# Args:
#     $url
#     $local_filename
# Returns:
#     1 : Success
#     0 : Failure
######################################################################
sub mirror2_map($$){
    my $url = shift;
    my $filename = shift;

    # sleep if polite is turned on to be nice to the webserver of the mapserver
    sleep($polite) if ($polite =~ /\d+/);
    sleep(1) if (!$polite);

    unlink('tmpmap.gif') if -f 'tmpmap.gif';
    `wget -nd -q -O tmpmap.gif "$url"`;
    if ( is_map_file('tmpmap.gif') ) {
	rename('tmpmap.gif',$filename);
	return 1;
    }
    return 0;
}


######################################################################
# returns the map filename for given $scale,$lati,$long
######################################################################
sub map_filename($$$){
    my ($scale,$lati,$long) = @_;
    
    my $filename = "$mapserver/$scale"
	."/".int($lati)
#	."/".sprintf("%3.1f",$lati)
	."/".int($long)
	."/$FILEPREFIX$scale-$lati-$long.gif";
    printf("Filename(%.0f,%.5f,%.5f): $filename\n",$scale,$lati,$long)
	if $debug;
    return $filename;
}

######################################################################
# get a single map at defined position
######################################################################
sub wget_map($$$){
    my ($scale,$lati,$long) = @_;

    my ($url,$mapscale);
    my $result="?";

#    my $plain_filename = "$FILEPREFIX$scale-$lati-$long.gif";
    # mapblast/1000/047/047.0232/9/map_1000-047.0232-0009.8140.gif 47.02320 9.81400 1000
    #my $filename = "$mapserver/$scale/".int($lati)."/".sprintf("%3.1f",$lati).
    #"/".int($long)."/$FILEPREFIX$scale-$lati-$long.gif";
    my $filename = map_filename($scale,$lati,$long);
    
    if ( $mapserver eq 'expedia') 
    {
	($url,$mapscale)=expedia_url($lati,$long,$scale);
    } 
    elsif ( $mapserver =~ m/^eniro_(se|dk|no|fi)$/) 
    {
	($url,$mapscale)=eniro_url($lati,$long,$scale);
    } 
    elsif ( $mapserver eq 'incrementp') 
    {
	($url,$mapscale)=incrementp_url($lati,$long,$scale);
    } 
    elsif ( $mapserver eq 'gov_au') 
    {
	($url,$mapscale)=gov_au_url($lati,$long,$scale);
    } elsif ( $mapserver eq 'googlesat') 
    {
	$mapscale=$scale;
	$url = "google-sat-maps";
    }
    elsif ( $mapserver eq 'geoscience') 
    {
	($url,$mapscale)=geoscience_url($lati,$long,$scale);
    } 
    elsif ( $mapserver eq 'landsat') 
    {
	($url,$mapscale)=landsat_url($lati,$long,$scale);
    } 
    else 
    {
	print "Unknown map sever :", $mapserver, "\n"; 
	return "E";
    }
    
    return "E" unless $url;

    my $mirror_filename=$filename;
    if ( $mapserver eq 'geoscience'){
	$mirror_filename="MIRROR/$filename";
	debug("mirror_filename=$mirror_filename");
    };
    
    # create directories if not existent
    for my $file ( ($filename , $mirror_filename ) ) {
	my $dir = dirname("$mapdir$file");
	unless ( -s $dir || -l $dir ) {
	    debug("Create Directory '$dir'");
	    mkpath($dir)
		or warn "Could not create $dir:$!\n";;
	}
    }

    
    if ( is_map_file( $filename ) ) {
	$result= "_";
	if ( $update_koord
	     && !defined $MAP_FILES->{$filename} ) {
	    append_koords($filename, $lati, $long, $mapscale);
	    $result= "u";
	};
	$existcount++;
    } else {
	if ( $simulate_only ) {
	    $result="S";
	    $newcount++;
	} else {
	    debug("mirror $url");	    
	    if ( $mapserver eq 'googlesat') 
	    {
		$result = google_stitch($lati,$long,$scale,5,4,"$mapdir$filename");
		if ($result eq '+') {
		  $newcount++;
		} else {
		  $failcount++;
		}
	    } elsif ( $mapserver eq 'gov_au') {
		if ( mirror_map($url,$filename) ) {
		    $result = gov_au_resize($lati,$long,$scale,5,4,"$mapdir$filename");
		    append_koords($filename, $lati, $long, $mapscale);
		    $result= "+";
		    print "\nWrote $filename\n" if $debug;
		    $newcount++;
		}
	    } elsif ( mirror_map($url,$mirror_filename) ) {
		if ( $mapserver eq 'geoscience'){
		    $result = resize($mapdir.$mirror_filename,$mapdir.$filename);
		} 
		append_koords($filename, $lati, $long, $mapscale);
		$result= "+";
		print "\nWrote $filename\n" if $debug;
		$newcount++;
	    }
	    else 
	    {
		$failcount++;
		$result= "E";
	    }
	}
   }
    
   return $result;
}

######################################################################
sub error_check {
    my $status;

    return 0 if $check_koord_file;
    return 0 if $check_coverage;
    return 0 if $TRACK_FILE;
    if ($cover_route) {
	my $num = @ARGV;
	if ($num < 2) {
	    print "ERROR: You must supply at least two waypoints\n\n";
	    return 1;
	} else {
	    return 0;
	}
    }

    # Check for a centerpoint
    unless (($waypoint) || ($lat && $lon) || ($slat && $endlat && $slon && $endlon)) {
	print "ERROR: You must supply a waypoint, latitude and longitude coordinates or starting and ending coordinates for both latitude and longitude\n\n";
	$status++;
    }
    
    # Check for area
    unless ($area || ($slat && $endlat && $slon && $endlon)) {
	print "ERROR: You must define an area to cover or starting and ending coordinates for both latitude and longitude\n\n";
	$status++;
    }
    
    return $status;
}

######################################################################
sub get_scales {
    my ($scale_ref) = @_;
    # OK lets figure out what scales they want
    #
    # '####'  - just that scale
    # '>####' - scales above and including the number given
    # '<####' - scales below and including the number given
    # '####,####,####' - a list of scales to download
    # '####-####' - scales from first to last
    # 
    my @scales_to_get;
    for my $temp_scale (split /,/, $$scale_ref) {
	if ($temp_scale =~ /^\d+$/) {
	    unless (grep (/$temp_scale/, @SCALES)){
		print STDERR "\n";
		print STDERR "Wrong scale $temp_scale\n";
		print STDERR "Allowed scales: ".join(", ",@SCALES)."\n";
		print STDERR "\n";
		pod2usage(1);
	    }
	    push(@scales_to_get,$temp_scale);
	} elsif ($temp_scale =~ /^>\d+$/) {
	    $temp_scale =~ s/>//;
	    push(@scales_to_get, grep ($_ >= $temp_scale, @SCALES)); 
	} elsif ($temp_scale =~ /^<\d+$/) {
	    $temp_scale =~ s/<//;
	    push(@scales_to_get, grep ($_ <= $temp_scale, @SCALES)); 
	} elsif ($temp_scale =~  /-/) {
	    my(@NUMS) = split(/-/,$temp_scale);
	    @NUMS = sort {$a <=> $b} @NUMS;
	    push(@scales_to_get, grep (($_ >= $NUMS[0]) && ($_ <= $NUMS[1]), @SCALES));
	} else {
	    pod2usage(1);
	}
    }
    return \@scales_to_get;
} #End get_scales

#############################################################################
sub expedia_url($$$){
    my $lati = shift;
    my $long = shift;
    my $scale = shift;

    # get altimeter setting for given scale
    my $mapscale = $scale;
    my $alti = undef;
    for my $s ( sort keys %{$Scale2Zoom->{expedia}} ) {
	next unless $s == $scale;
	$alti = $Scale2Zoom->{expedia}->{$s};
	$mapscale = $s;
	last;
    }

    # error if not found
    unless ( $alti ) {
	print "Error calculating Zoomlevel for Scale: $scale\n";
	return (undef,undef);
    }

    # debug output
    if ($debug) {
	print "\n";
	print "Using expedia altitude ", $alti, " for requested scale ", $scale, ":1 actual scale ", $mapscale, ":1\n";
	print "lat: $lati\n";
	print "lon: $long\n";
    }

    # generate url
    my $where;
    if ($long < -30) {
	$where = 'USA0409';
    } else {
	$where = 'EUR0809';
    }
    my $url = "http://www.expedia.com/pub/agent.dll?qscr=mrdt&ID=3XNsF.\&"
	."CenP=$lati,$long\&Lang=$where\&Alti=$alti\&Size=1280,1024\&Offs=0.000000,0.000000";
    return ($url,$mapscale);
}


#############################################################################
sub eniro_url($$$){
    my $lati = shift;
    my $long = shift;
    my $scale = shift;

    my $mapscale = $scale;

    
    my $zoom = undef;
    my $url='';

    my %factor = ( 1 => 18000000, # 480     Km
		   2 => 3200000,  #  80     Km
		   3 => 400000,   #  10     Km
		   4 => 200000,   #   4     Km
		   5 => 100000,   #   1.1   Km
		   6 => 20000,    #     400 m
		   7 => 5000,     #     160 m
		   );
    for my $f ( sort keys %factor ) {
	my $test_scale =  $factor{$f};
	if ( $debug) {
#	    print "testing $f: against	$test_scale\n";
	}
	if ( $test_scale <= $scale ) {
	    $zoom = $f;
	    $mapscale = $test_scale;
	    last;
	}
    }
    unless ( $zoom ) {
	print "Error calculating Zommlevel for Scale: $scale\n";
	return (undef,undef);
    }

    if ($debug) {
	printf "Eniro : lat: %.4f lon: %.4f\t",$lati,$long;
	print "using zoom ", $zoom, " for requested scale ", $scale, ":1 actual scale ", $mapscale, ":1\n";
    }
    
    my ( $eniro_country ) = ($mapserver =~ m/^eniro_(se|dk|no|fi)$/);


    if ( $eniro_country eq "dk"
	 || # Add real restriction for Denmark
	  ( $lati > 54.67694 ) && ( $lati < 56.71874 ) &&
	  ( $long >  7.77864 ) && ( $long < 13.40111 )
	  ) {
	print "Daenmark ($lati,$long)\n" if $debug;
	$url  = "http://maps.eniro.$eniro_country/servlets/${eniro_country}_MapImageLocator?profile=Main";
	$url .= sprintf("&center=%10.4f;%10.4f",($lati*10000),($long*10000));
	$url .= "&zoomlevel=$zoom";
	$url .= "&size=1280x1024";
	$url .= "&symbols=";
    } else { # Add here restriction for Norway
	print "Norway ($lati,$long)\n" if $debug;
	$url  = "http://maps.eniro.$eniro_country/servlets/${eniro_country}_MapImageLocator?profile=Main";
	$url .= sprintf("&center=%.4f;%.4f",$long,$lati);
	$url .= "&zoomlevel=$zoom";
	$url .= "&size=1280x1024";
	$url .= "&symbols";
    } 
    
    return ($url,$mapscale);
}

#############################################################################
sub incrementp_url($$$){
    my $lati = shift;
    my $long = shift;
    my $scale = shift;

    my $mapscale = $scale;

    my $zoom = undef;
    my $url='';
    for my $s ( sort keys %{$Scale2Zoom->{incrementp}} ) {
	next unless $s > $scale;
	$zoom = $Scale2Zoom->{incrementp}->{$s};
	$mapscale = $s;
	last;
    }

    unless ( $zoom ) {
	print "Error calculating Zoomlevel for Scale: $scale\n";
	return (undef,undef);
    }

    if ($debug) {
	printf "Incrementp : lat: %.4f lon: %.4f\t",$lati,$long;
	print "using zoom ", $zoom, " for requested scale ", $scale, ":1 actual scale ", $mapscale, ":1\n";
    }
    
    $url  = "http://mapserv.incrementp.co.jp/cgi-bin/map/mapserv.cgi?";

{
    my $lat1 = ($lati>0 ? "E" :"W" );
    $lat1 .= int($lati);
    $lat1 .= sprintf(".%.0f",($lati-int($lati))/60);
    
    my $lon1 = ($lati>0 ? "N" :"S" );
    $lon1 .= int($long);
    $lon1 .= sprintf(".%.0f",($long-int($long))/60);
    #                    MAP=E127.48.26.8N26.19.49.3
    $url .= sprintf("MAP=%s%s",$lat1,$lon1);
}

$url .= "&OPT=e0000011";
$url .= "&ZM=$zoom";
$url .= "&SZ=1200,1200";
$url .= "&COL=1";

return ($url,$mapscale);
}

#############################################################################
sub gov_au_url($$$){
    my $lati = shift;
    my $long = shift;
    my $scale = shift;

    my $mapscale = $scale;
    my $zoom     = undef;
    my $url      = '';
    for my $s ( sort keys %{$Scale2Zoom->{incrementp}} ) {
	next unless $s > $scale;
	$zoom = $Scale2Zoom->{incrementp}->{$s};
	$mapscale = $s;
	last;
    }
    unless ( $zoom ) {
	print "Error calculating Zommlevel for Scale: $scale\n";
	return (undef,undef);
    }

    $url = "http://www.ga.gov.au/bin/mapserv36?".
	"map=/public/http/www/docs/map/globalmap/global_36.map".
	"\&mode=map".
	"\&layer=outline".
	"\&layer=waterbodies".
	"\&layer=capitals".
	"\&layer=permanentriverslargescale".
	"\&layer=fluctuatingrivers".
	"\&layer=dualcarriage".
	"\&layer=PrimaryPavedRoad".
	"\&layer=roadsecpav".
	"\&layer=roadotherpav".
	"\&mapsize=1000+800";

    # OK I'm too lazy to calculate the real start and end
    my $fac = 1000000000;
    my $scale_fac =  ($scale/$fac);
    my $lat1= $lati - $scale_fac;
    my $lat2= $lati + $scale_fac;
    my $lon1= $long - $scale_fac;
    my $lon2= $long + $scale_fac;
    $url .= "\&mapext=$lon1+$lat2+$lon2+$lat1";
	
	if ($debug) {
	    printf "Gov_Au : lat: %.4f lon: %.4f\t",$lati,$long;
	    print "using zoom $zoom for requested scale $scale\n";
	    print "actual scale $mapscale with scalefac: $scale_fac\n";
	    print "URL: \n$url\n";
	};
    
    return ($url,$mapscale);
}

#############################################################################
sub landsat_url($$$){
    my $lat   = shift;
    my $lon   = shift;
    my $scale = shift;
 
    my $factor = $Scale2Zoom->{'landsat'}->{$scale};

    my $deltalat = 0.0005;
    my $deltalon = 0.001;   # Gives ratio 1.2 in meter

    my $lon1 = $lon-$deltalon*$factor/2;
    my $lat1 = $lat-$deltalat*$factor/2;
    my $lon2 = $lon+$deltalon*$factor/2;
    my $lat2 = $lat+$deltalat*$factor/2;

    debug( "landsat_url(LAT=$lat,LON=$lon,SCALE=$scale,FACTOR=$factor)");

    debug( "Calculated Lat1  $lat1");
    debug( "Calculated Lat2  $lat2");
    debug( "Calculated Lon1 $lon1");
    debug( "Calculated Lon2 $lon2");

    # Build the URL
    my $url='';
    $url .= "http://onearth.jpl.nasa.gov/wms.cgi";
    $url .= '?request=GetMap';
    $url .= '&width=1280';
    $url .= '&height=1024';
    $url .= '&layers=global_mosaic';
    $url .= '&styles=';
    $url .= '&srs=EPSG:4326';
#    $url .= '&format=image/gif';
    $url .= '&format=image/jpeg';

    # Add the 4 Coordinates.
    $url .= '&bbox='; # '15.3,58.35,15.5,58.45';
    $url .= "$lon1,$lat1,$lon2,$lat2";
#    $url .= sprintf("%.5f",$lon1).",";
#    $url .= sprintf("%.5f",$lat1).",";
#    $url .= sprintf("%.5f",$lon2).",";
#    $url .= sprintf("%.5f",$lat2);

    return ($url,$scale);
}


#############################################################################
sub geoscience_url($$$){
    my $gs_lati = shift;
    my $gs_long = shift;
    my $gs_scale = shift;
 
    debug( "geoscience_url(LATI=$gs_lati,LONG=$gs_long,SCALE=$gs_scale)");

    my $url='';
    my $factor = $Scale2Zoom->{'geoscience'}->{$gs_scale};
    my $latioffset = 0.6*$factor;
    my $longoffset = 0.8*$factor;
    my $lati1 = $gs_lati + $latioffset;
    my $lati2 = $gs_lati - $latioffset;
    my $long1 = $gs_long - $longoffset;
    my $long2 = $gs_long + $longoffset;
    debug( "Calculated Lat1  $lati1");
    debug( "Calculated Lat2  $lati2");
    debug( "Calculated Long1 $long1");
    debug( "Calculated Long2 $long2");

    # Build the URL
    $url = "http://www.ga.gov.au/bin/mapserv36"
	."?map=/public/http/www/docs/map/globalmap/global_36.map";
    $url .= '&mode=map';
    # Add the 4 Coordinates.
    $url .= '&mapext=';
    $url .= sprintf("%.5f",$long1)."+";
    $url .= sprintf("%.5f",$lati1)."+";
    $url .= sprintf("%.5f",$long2)."+";
    $url .= sprintf("%.5f",$lati2);

    # Add all the different layers
    $url .= '&layer=outline';
    $url .= '&layer=waterbodies';
    $url .= '&layer=capitals';
    $url .= '&layer=statenames';
    $url .= '&layer=permanentriverslargescale';
    $url .= '&layer=fluctuatingrivers';
    $url .= '&layer=streams';
    $url .= '&layer=builtup';
    $url .= '&layer=dualcarriage';
    $url .= '&layer=PrimaryPavedRoad';
    $url .= '&layer=PrimaryUnpavedRoad' unless $scale >665924;
    $url .= '&layer=roadsecpav'         unless $scale >665924;
    $url .= '&layer=roadsecnotpav'      unless $scale >665924;
    $url .= '&layer=roadotherpav'       unless $scale >665924;
    $url .= '&layer=roadothernotpav'    unless $scale >665924;
    $url .= '&layer=bridge'             unless $scale >665924;
    $url .= '&layer=railways';
    $url .= '&layer=railyard'           unless $scale >665924;
    $url .= '&layer=airports';
    $url .= '&layer=localities';
    $url .= '&layer=index250k'          if $debug;       # 250K MapSheet Outlines
    # Add the map size
    $url .= "\&mapsize=1000+800";	
#    $url .= "\&mapsize=1024+820";	

    return ($url,$gs_scale);
}
#############################################################################
# Note: The resize will only work if the image you want to resize has
# the same aspect ratio as 1280x1024 
# i.e. 1280/1024=1.25
#      1000/800=1.25
#############################################################################
sub resize($$){
    my $src_filename = shift;
    my $dst_filename = shift;

    my $x='';
    my $image='';
    
    debug( "resize( $src_filename --> $dst_filename)" );

    $image = Image::Magick->new;
    $x = $image->Read($src_filename);
    if( $x ) {
        undef $image;
        warn "\npic_resize($src_filename,$dst_filename) open: $x\n" ;
        return 0;
    }
    $x = $image->Sample(geometry => "1280x1024+10+10");
    if( $x ) {
        undef $image;
        warn "\npic_resize($src_filename,$dst_filename) _resize: $x\n" ;
        return 0;
    }
    $x = $image->Write($dst_filename);
    if( $x ) {
        undef $image;
        warn "\npic_resize($src_filename,$dst_filename)_write: $x\n" ;
        return 0;
  }
  undef $image;
}

#############################################################################
# This function only once calculates
# which lat/lon/scale Combinations are desired to download
# Args:
#    $slat,$slon : Start latitude /longitude
#    $elat,$elon : End   latitude /longitude
# Returns:
#     $desired_locations->{$scale}->{$lati}->{$long}='?';
#############################################################################
sub desired_locations {
    my $desired_locations = shift;
    my ($slat,$slon,$elat,$elon) = @_;
    my $count;   

    my $local_debug = 0 && $debug;

    my $min_lat=-90;
    my $max_lat= 90;
    my $min_lon=-180;
    my $max_lon= 180;
    if ( $slat < $min_lat ) { warn ("Starting Latitude  ($slat) set to $min_lat\n"); $slat=$min_lat; };
    if ( $elat > $max_lat ) { warn ("End      Latitude  ($elat) set to $max_lat\n"); $elat=$max_lat; };
    if ( $slon < $min_lon ) { warn ("Starting Longitude ($slon) set to $min_lon\n"); $slon=$min_lon; };
    if ( $elon > $max_lon ) { warn ("End      Longitude ($elon) set to $max_lon\n"); $elon=$max_lon; };

    foreach my $scale ( sort  {$b <=> $a} @{$SCALES_TO_GET_ref} ) {
	# Setup k
	my $k = $DIFF * $scale;
	my $delta_lat = $k - ($k / 2); ### FIX BY CAMEL
	my $delta_lon = $k - ($k / 6); ### FIX BY CAMEL
	#TODO: $delta_lon sollte von lat abhaengen

	# make the starting points for the loop $slat and $slon 
	# snap into a grid with a Size depending on the scale.
	# The result is $snapped_start_lat and $snapped_start_lon
	# The grid allows maps in each direction to 
	# overlapp by 1/$overlap of the size of one map
	# With snap to grid we would have to download the exact same maps
	# for slightly different starting points. This way we can 
	# circumvent downloads of almost completely overlaping maps
	my $overlap = 1;
	my $flat =  $delta_lat / $overlap;
	my $snapped_start_lat = int ( $slat / $flat  ) * $flat;
	my $flon = $delta_lon / $overlap;
	my $snapped_start_lon = int ( $slon / $flon ) * $flon;

	print "Scale: $scale\t";
	printf "  lati: %6.5f(%6.5f) +=%5.5f ... %6.5f\n",
	$snapped_start_lat,$slat,$delta_lat,$elat;

	my $lati = $snapped_start_lat;

	while (($lati <= $elat) || (!$count)) {
	    my $long = $snapped_start_lon;
	    if ( $local_debug ) {
		printf "        %5.5f:",$lati;
		printf "\tlong: %6.4f(%6.4f) +=%5.4f ... %6.4f"
		    ,$snapped_start_lon,$slon,$delta_lon,$elon;
		printf "\t\t";	
	    }
	    while (($long <= $elon) || (!$count)) {
		$desired_locations->{$scale}->{$lati}->{$long} ||='?';

		if ( $local_debug ) {
		    my $filename = map_filename($scale,$lati,$long);
		    my $exist = ( is_map_file($filename) ) ? " ":"+";

		    printf " %6.3f %1s",$long,$exist;
		}
		$long += $delta_lon; ### FIX BY CAMEL
		$count++;
	    }
	    print "\n" if $local_debug;
	    $lati += $delta_lat; ### FIX BY CAMEL
	    $long = $slon; ### FIX BY CAMEL
	}
    }
}

######################################################################
# Count the Number of Files to be retrieved
# Returns: ($existing,$wanted)
#    $existing : Number of existing maps
#    $wanted   : Number of maps to be retrieved
######################################################################
sub file_count($){
    my $desired_locations = shift;
    my $existing=0;
    my $wanted=0;
    for my $scale ( keys %{$desired_locations} ) {
	for my $lat ( keys %{$desired_locations->{$scale}} ) {
	    for my $lon ( keys %{$desired_locations->{$scale}->{$lat}} ) {
		my $filename = map_filename($scale,$lat,$lon);
		if ( is_map_file($filename) ) {
		    $existing++;
		} else{
		    $wanted++;
		}
	    }
	}
    }
    return($existing,$wanted);
} #end file_count

######################################################################
sub get_waypoint($) {
    my $waypoint = shift;
    
    # If they give just a filename, we should assume they meant the CONFIG_DIR
    $WAYPT_FILE = "$CONFIG_DIR/$WAYPT_FILE" unless ($WAYPT_FILE =~ /\//);
    
    open(WAYPT,"$WAYPT_FILE") || die "ERROR: get_waypoint Can't open: $WAYPT_FILE: $!\n";
    my ($name,$lat,$lon);
    while (<WAYPT>) {
	chomp;
	next unless (/$waypoint/);
	($name,$lat,$lon) = split(/\s+/);
    }
    close(WAYPT);
    unless (($lat) && ($lon)) {
	print "Unable to find waypoint '$waypoint' in '$WAYPT_FILE'\n";
	exit;
    }
    return($lat,$lon);
} #End get_waypoint


######################################################################
# Read the config File (~/.gpsdrive/gpsdriverc) 
# and return all Config Keys as a Hash reference
sub read_config {
    # If they give just a filename, we should assume they meant the CONFIG_DIR
    $CONFIG_FILE = "$CONFIG_DIR/$CONFIG_FILE" unless ($CONFIG_FILE =~ /\//);
    
    # If not specified on the command line, we read from the config file
    open(CONFIG,"$CONFIG_FILE") || die "ERROR: get_unit Can't open $CONFIG_FILE: $!\n";

    my $cfg =  {};
    while (my $line = <CONFIG>) {
	chomp $line;
	my ($key,$val) = split(/\s*=\s*/,$line);
	$cfg->{lc($key)} = $val
	    if $key;
    }   
    close(CONFIG);
    return $cfg;
} #End read_config

######################################################################
sub get_coords {
    my ($lat,$lon,$area,$unit) = @_;
    
    # Figure out if we are doing square area or a rectangle
    my ($lat_dist,$lon_dist);
    if ($area =~ /x/i) {
	($lat_dist,$lon_dist) = split(/x/i,$area);
    } else {
	$lat_dist = $area;
	$lon_dist = $area;
    }
    print "Latitude distance: $lat_dist, Longitude distance: $lon_dist\n" if ($debug); 
    
    my $lon_dist_km = calc_lon_dist($lat);
    my $lat_offset  = calc_offset($unit,$lat_dist,\$LAT_DIST_KM);
    my $lon_offset  = calc_offset($unit,$lon_dist,\$lon_dist_km);   
    
#    print "LAT_OFFSET = $$lat_offset LON_OFFSET = $$lon_offset \n" if ($debug);
    
    # Ok subtract the offset for the start point
    my $slat = $lat - $lat_offset;
    my $slon = $lon - $lon_offset;

    # Ok add the offset for the start point
    my $elat = $lat + $lat_offset;   
    my $elon = $lon + $lon_offset;   
    
    return ($slat,$slon,$elat,$elon);
} #End get_coords

######################################################################
sub calc_offset {
    my($unit,$area,$dist_per_degree) = @_;
    # Adjust the dist_per_degree for the unit chosen by the user
    if ($unit =~ /miles/) {
	$$dist_per_degree *= $KM2MILES;   
    } elsif ($unit =~ /nautic/) {
	$$dist_per_degree *= $KM2NAUTICAL;
    }
    
    # The offset for the coordinate is the distance to travel divided by 
    # the dist per degree   
    my $offset = sprintf("%.7f", ($area / 2) / $$dist_per_degree);
    
    #print "-\n".Dumper($area,\$dist_per_degree,$offset);
    return($offset);
} #End calc_offset

######################################################################
# Opens a track file and returns two identically sized arrays,
# one that contains the latitudes and other that contains the
# longitudes.
######################################################################
sub get_coords_for_track($) {
    my $filename = shift;

    my @lats;
    my @lons;
    print "Opening Track file: $filename\n" if $debug;
    my $fh = IO::File->new("<$filename");
    $fh or die("Error: $!");

    my ($la, $lo, $oldla, $oldlo, $line, $delta_la, $delta_lo, $rest);
    my ($step_la, $step_lo, $ste_la, $ste_lo);

    my $max_lat= 180;
    my $max_lon= 90;

    $oldla = 0;
    $oldlo = 0;
    $step_la = ($DIFF * $scale) - ($DIFF * $scale) / 2;
    $step_lo = ($DIFF * $scale) - ($DIFF * $scale) / 1.5;

    # loop through each line of the track file
    while ($line = $fh->getline()) {
        $line =~ s/^\s+//;
	chomp $line;
        ($la,$lo,$rest) = split(/\s+/, $line, 3);

		# Disallow blank lines, as gpsreplay uses them. Also disallow
		# blank (undefined) fields in case of other possible
		# wierdnesses.

		next if (!defined($lo));
		next if (!defined($la));

		debug("pre-regex:  ($la|$lo|$rest)");
		# Now a regex (applied to both lat & long) to drop cruft like
		# leading text or trailing commas. Skip over anything that is
		# not a digit, minus sign or decimal point. Then extract
		# either an optional minus sign, one or more digits, a decimal
		# point, 0 or more digits, or an optional minus sign, a
		# decimal point and one or more digits. Ignore the rest.
		$la =~ s/([^\d\-\.]*)(\-?\d+\.?\d*|-?\.\d+)(.*)/$2/o;
		$lo =~ s/([^\d\-\.]*)(\-?\d+\.?\d*|-?\.\d+)(.*)/$2/o;
		debug("post-regex: ($la|$lo|$rest)");
		next if ( $la == 1001 ) && ( $lo == 1001) ;
		if ((($la != $oldla) || ($lo != $oldlo)) && ($la < $max_lat) && ($lo < $max_lon)) {
			$delta_la = abs($la - $oldla);
			$delta_lo = abs($lo - $oldlo);
			if (($delta_la > $step_la) || ($delta_lo > $step_lo)) {
				push(@lats, $la);
				push(@lons, $lo);
				$oldla = $la;
				$oldlo = $lo;
			}
		}
	}

    # Close the file and release lock or die.
    $fh->close() or die("Error: $!");    
    return (\@lats, \@lons);
} # end get_coords_for_track

######################################################################
# Creates a track between given waypoints. Returns two identically 
# sized arrays, one that contains the latitudes and other that 
# contains the longitudes.
######################################################################
sub get_coords_for_route {
    my @lats;
    my @lons;
    my @waypoints = @ARGV;
    foreach $waypoint (@waypoints) {
        # check if all given waypoints exist
        &get_waypoint($waypoint);
    }    

    my (@start, @end, @delta, @steps);
    my ($la, $lo, $i, $j);

    my $step_la = ($DIFF * $scale) - ($DIFF * $scale) / 2;
    my $step_lo = ($DIFF * $scale) - ($DIFF * $scale) / 1.5;

    while (@waypoints > 1) {
        # download maps for each etap
        print("$waypoints[0] - $waypoints[1]\n");
        @start = &get_waypoint($waypoints[0]);
        @end = &get_waypoint($waypoints[1]);
        shift(@waypoints);

        $delta[0] = $start[0] - $end[0];
        $delta[1] = $start[1] - $end[1];
        $steps[0] = $delta[0] / $step_la;
        $steps[1] = $delta[1] / $step_lo;
        if ($steps[0] > $steps[1]) {
            $i = abs(ceil($steps[0]));
        } else {
            $i = abs(ceil($steps[1]));
        }
        for ($j=1; $j<=$i; $j++) {
            $la = $start[0] - ($delta[0] / $i)*$j;
            $lo = $start[1] - ($delta[1] / $i)*$j;
	    push(@lats, $la);
	    push(@lons, $lo);
        }
    }
    return (\@lats, \@lons);
} #End get_coords_for_track


######################################################################
# Return KM/degree for this latitude
######################################################################
sub calc_lon_dist {
    my $lat = shift;

    my $PI  = 3.141592654;
    my $dr = $PI / 180;
    
    # calculate the circumference of the small circle at latitude 
    my $cos = cos($lat * $dr); # convert degrees to radians
    my $circ_km = sprintf("%.2f",($PI * 2 * $RADIUS_KM * $cos));
    
    # divide that by 360 and you have kilometers per degree
    my $km_deg = sprintf("%.2f",($circ_km / 360));
    
    return ($km_deg);
} #End calc_longitude_dist

######################################################################
# Check if Map-image is valid and append one Entry to the map_koord File
# Returns:
#  A for Appended
#  E for Error
######################################################################
sub append_koords($$$$) {
    my $filename = shift;
    my $lati     = shift;
    my $long     = shift;
    my $mapscale = shift;

    # eliminate map_Directory prefix
    $filename =~ s,^$mapdir,,;

    die "Missing Params to append_koords($filename,$lati,$long,$mapscale)\n"
	unless $filename && defined($lati) && defined($long) && $mapscale;


    if ( is_map_file($filename) ) {
	# print "$filename $lati $long $mapscale\n";
    } elsif ( ! -s "$mapdir/$filename" ) {
	print "Fehler $filename leer/existiert nicht\n";
	return 'E';
    } else {
	print "Fehler $filename ist kein mapfile\n";
	return 'E';
    }

    if ( ! defined $MAP_FILES->{$filename} ) {
	debug("Appending $filename,$lati, $long, $mapscale to $KOORD_FILE");
	open(KOORD,">>$KOORD_FILE") || die "ERROR: append_koords can't open: $KOORD_FILE: $!\n"; 
	printf KOORD "$filename %17.13f %17.13f %17d\n",$lati, $long, $mapscale;
	close KOORD;
	$MAP_FILES->{$filename} = "$lati, $long, $mapscale";
	$MAP_KOORDS->{$mapscale}->{$lati}->{$long} = 1;
    }

    if ( -s $GPSTOOL_MAP_FILE ) {
	if ( ! defined $GPSTOOL_MAP_FILES->{$filename} ) {
	    open(KOORD,">>$GPSTOOL_MAP_FILE") || 
		die "ERROR: append_koords can't open: $GPSTOOL_MAP_FILE: $!\n"; 
	    printf KOORD "$mapdir/$filename %17.13f %17.13f %4d 1280 1024\n",$lati, $long, $mapscale;
	    close KOORD;
	    $GPSTOOL_MAP_FILES->{$filename} = "$lati, $long, $mapscale";
	    $GPSTOOL_MAP_KOORDS->{$mapscale}->{$lati}->{$long} = 1;
	}
    }
    
    return 'A';
} # End  append_koords


#############################################################################
# Read actual Koordinate File and memorize in $MAP_KOORDS and $MAP_FILES
sub read_koord_file($) {
    my $koord_file = shift;

    $MAP_FILES={};

    my $s_time=time();
    unless ( -s $koord_file ) {
	warn "ERROR: read_koord_file sees empty file '$koord_file'\n";
	return;
    };
    print "reading $koord_file\n";
    open(KOORD,"<$koord_file") || die "ERROR: read_kooord_file can't open: $koord_file: $!\n"; 
    my $anz_files = 0;
    while ( my $line = <KOORD> ) {
	my ($filename ,$lati, $long, $mapscale);
	($filename ,$lati, $long, $mapscale) = split( /\s+/ , $line );
	if ( is_map_file( $filename ) ) {
	    $MAP_KOORDS->{$mapscale}->{$lati}->{$long} = 1;
	    $MAP_FILES->{$filename} = "$lati, $long, $mapscale";
	    $anz_files ++;
	} else {
	    warn "ERROR: read_koord_file is missing File $filename\n";
	};
    }
    close KOORD;
    my $r_time = time()-$s_time;
    print "$koord_file read $anz_files in $r_time sec.\n";
    
}

#############################################################################
# Read actual Koordinate File and memorize in $GPSTOOL_MAP_KOORDS and $GPSTOOL_MAP_FILES
sub read_gpstool_map_file() {
    my $koord_file = $GPSTOOL_MAP_FILE;
    my $s_time=time();
    return unless -s $koord_file;
    print "reading $koord_file\n";
    open(KOORD,"<$koord_file") || die "ERROR: read_gpstool_map_file can't open: $koord_file: $!\n"; 
    my $anz_files = 0;
    while ( my $line = <KOORD> ) {
	my ($filename ,$lati, $long, $mapscale);
	($filename ,$lati, $long, $mapscale) = split( /\s+/ , $line );
	if ( is_map_file( $filename) ) {
	    $GPSTOOL_MAP_KOORDS->{$mapscale}->{$lati}->{$long} = 1;
	    $GPSTOOL_MAP_FILES->{$filename} = "$lati, $long, $mapscale";
	    $anz_files ++;
	}
    }
    close KOORD;
    my $r_time = time()-$s_time;
    print "$koord_file read $anz_files in $r_time sec.\n";
    
}

#############################################################################
# Read actual Koordinate File (gpstool)
# and check if all Files it references are existing
sub check_koord_file($) {
    my $koord_file = shift;
    # Change into the gpsdrive maps directory 

    print "Checking all entries in $koord_file\n" if $debug;
    $MAP_FILES={};
    my $anz_files = 0;
    my $missing_files =0;
    if ( -s $koord_file ) {
	open(KOORD,"<$koord_file") || die "ERROR: check_koord_file can't open: $koord_file: $!\n";
	while ( my $line = <KOORD> ) {
	    my ($filename ,$lati, $long, $mapscale);
	    ($filename ,$lati, $long, $mapscale) = split( /\s+/ , $line );
	    my $full_filename = "$CONFIG_DIR/maps/$filename";

#	debug("Checking ($filename ,$lati, $long, $mapscale)");

	    if ( !is_map_file( $full_filename ) ) {
		print "ERROR: File $full_filename not found\n";
		$missing_files ++;
	    } else {
		debug("OK:    File $full_filename found");
		$MAP_KOORDS->{$mapscale}->{$lati}->{$long} = 1;
		if ( $MAP_FILES->{$filename} ) {
		    print "ERROR: Duplicate File $full_filename found\n";
		};
		$MAP_FILES->{$filename} = "$lati, $long, $mapscale";
		$anz_files ++;
	    }
	}
	close KOORD;
	print "Good files: $anz_files\n";
    }
    if ( $missing_files ) {
	print "Missing Files: $missing_files\n";
	open(KOORD,">$koord_file.new") || die "Can't open: $koord_file.new: $!\n"; 
	foreach my $map_filename ( keys  %$MAP_FILES )  {
	    printf KOORD "$map_filename	%s\n", $MAP_FILES->{$map_filename};
	}
	close KOORD;
	print "wrote $koord_file.new\n";
    }
}


###########################################################################
# Update Maps found in Filesystem which cannot be found in map_koords.txt
###########################################################################
use File::Find;
sub update_file_in_map_koords(); # {} 
sub update_gpsdrive_map_koord_file(){
    print "\n";
    print "\n";
    print "Check, if Files in Filesystem can be added to map_koord.txt\n";

    debug("Searching for Files in '$mapdir'");
    find(
     { wanted       => \&update_file_in_map_koords,
       follow_skip  => 2,
       follow       => 1 
       },
	 $mapdir, );
}

sub update_file_in_map_koords(){
    my $filename = $File::Find::name;
    return if -d $filename;
    return unless  $filename =~ m/\.gif$/;
    my $short_filename = $filename;
    $short_filename =~ s,$mapdir,,;
#    debug("Check File: $short_filename");
    if ( $MAP_FILES->{$filename} ||
	 $MAP_FILES->{$short_filename} ) {
#	print "OK       $filename\n";
    } else {
	#mapblast/1000/047/047.0232/9/map_1000-047.0232-0009.8140.gif 47.02320 9.81400 1000
	if ( $filename =~ m/map_(\d+)-(\d+\.\d+)-(\d+\.\d+)\.gif/ ) {
	    my $scale=$1;
	    my $lati=$2;
	    my $long=$3;
	    my ($url,$mapscale)=expedia_url($lati,$long,$scale);
	    
#	    print "Appending File: $filename\n";
	    append_koords($short_filename, $lati, $long, $mapscale);
	    debug("File:$filename lat:$lati lon:$long");
	}

    }
}


######################################################################
# See which area the maps cover
######################################################################
sub check_coverage($){
    my $koord_file = shift;
    read_koord_file($koord_file);
    my @scales= sort  {$b <=> $a} keys %{$MAP_KOORDS};

    for my $scale ( @scales ) {
	print "$scale:\n";;
	
	my @all_lons;
	my %all_lons;
	my @lats = sort {$a <=> $b} keys %{$MAP_KOORDS->{$scale}};
	for my $lat ( @lats ) {
	    my @lons = keys %{$MAP_KOORDS->{$scale}->{$lat}};
	    for my $lon ( @lons ) {
		push (@all_lons, $lon)
		    unless $all_lons{$lon}++;
	    }
	}

	my $first_lon = $all_lons[0];
	my $last_lon = $all_lons[-1];

	print "lon: ($first_lon - $last_lon))\n";
	print "lat: (desiredDistance, minDistance - maxDistance) Km\n";

	for my $lat ( @lats ) {
	    printf "   %7.4f: ", $lat;
	    my @lons = sort {$a <=> $b} keys %{$MAP_KOORDS->{$scale}->{$lat}};
	    my %lons;
	    map { $lons{$_}++ }  @lons;


	    # Find out which is the desired Distance
	    my $lon_dist_km = calc_lon_dist($lat);
	    my $k = $DIFF * $scale;    
	    my $delta_lat = $k - ($k / 2); my $dlat = $delta_lat * $LAT_DIST_KM;
	    my $delta_lon = $k - ($k / 6); my $dlon = $delta_lon * $lon_dist_km;
	    
	    # Find Min and Max Distance between 2 Maps
	    my $min_dist= $RADIUS_KM;
	    my $max_dist=0;
	    my $count_overlaps=0;
	    my $prev_lon;
	    my %gap;
	    my %overlap;
	    for my $lon (  sort {$a <=> $b} @all_lons ) {
		if ( defined $prev_lon ) {
		    my $dist = abs($lon - $prev_lon) * $lon_dist_km;
		    if(  $dist < ($dlon * 0.9)) {
			$overlap{$lon}++;
			$count_overlaps++;
		    }

		    $gap{$lon}=$dist /$dlon if $dist > ($dlon * 1.2);
		    $min_dist = $dist if $dist < $min_dist;
		    $max_dist = $dist if $dist > $max_dist;
		}
		#print "Dist: ( $min_dist - $max_dist) Km ($prev_lon => $lon = ".($prev_lon -$lon).")\n";
		$prev_lon = $lon;
	    };
	    
	    printf " (%.2f,%.2f - %.2f) Km\t",$dlon,$min_dist,$max_dist;
	    printf " %d Overlaps\t",$count_overlaps if $count_overlaps;
	    #*= $LAT_DIST_KM;
	    


	    # Print +/- for existing/non-existing Map
	    for my $lon ( @all_lons ) {
#		printf "%7.4f ", $lon;
		# Print Number of Tiles missing(gap) between last(left) map and this one
		printf " -%.0f ",$gap{$lon} if $gap{$lon};

		if ( $lons{$lon} ) {
		    if ( $overlap{$lon} ) {
			print "o";
		    } else {
			print "+";
		    }
		} else {
		    print "-";
		}
	    }
	    print "\n";
	}
	print "\n";
    }
    print "\n";
}

# =============================================================================
# This part downloads a bunch of Google satellite images 
# for coordinates given and stitches them together
# ver 0.99
#
# changes: v. 0.92 - 0.99 now using PerlMagic directly
#
# Copyright (c) 2005 Piter Pen <code\@piterpen.net>
# http://www.piterpen.net
#
# I was too lazy to write a function converts number of the map to 'tqrs'-like string 
# so I took one (xy2goog) from http://web.media.mit.edu/~nvawter/projects/googlemaps/sa01.pl 
# without permission - I assume it's ok but I promise to remove if not.
#
# SYNOPSIS:
# google_stitch.pl 39.346970 72.879140 15 3 4
# Stitch the satellite image with zoom level 15, 3 kh.google.com images width,
# 4 height and centered in 39.346970N and 72.879140E
sub lat2y1($){
    my $lat = shift;

    my $sinLat = sin(($lat*$PI)/180);
    my $res = log(
		  (1 + $sinLat) /
		  (1 - $sinLat)
		  )/2;
#    debug("lat2y1($lat) --> $res");
    return $res;
}




sub xy2goog($$$){
    my ($x, $y, $zoom) = @_;

    $y = 2**($zoom-1) - 1 - $y;
    
    my $format = '%0'.$zoom.'b';

    my @xBits = split(//, sprintf($format, $x));
    my @yBits = split(//, sprintf($format, $y));

    #debug("xy2goog($x,$y,$zoom)-> $format '".sprintf($format, $x)."'");
    #debug("xy2goog($x,$y,$zoom)-> $format '".sprintf($format, $y)."'");

    my $res = '';
    for(my $i = 0; $i < $zoom; $i++) {
	my $qc = ($xBits[$i] << 1) + $yBits[$i];
        $res .= $quadChars[$qc];
    }
    #debug("xy2goog($x,$y,$zoom)-> $res");

    return $res;
}

sub xy2latlon($$$$){
    my ($x, $y, $zoom, $format) = @_;

    my $width = 2**($zoom-1);
    my $height = 2**($zoom-1);
    
    my $tileWidth = 360 / $width;

    my $lon = $x*$tileWidth - 180;
    
    my $yTop = lat2y1($MERCATOR_TOP);
    my $yBottom = lat2y1($MERCATOR_BOTTOM);

    $y = (($yBottom - $yTop)*$y)/$height + $yTop;
    
    my $lat = (360*atan2(exp($y), 1))/$PI - 90;

    my $res = sprintf($format, $lat, $lon);

    #debug("xy2latlon($x,$y,$zoom,$format)-> $res");
    return $res;
}

sub latlon2xy($$$){
    my ($lat, $lon, $zoom) = @_;

    my $width = 2**($zoom-1);
    my $height = 2**($zoom-1);

    my $tileWidth = 360 / $width;

    my $x = ($lon + 180)/$tileWidth;
    
    my $y = lat2y1($lat);

    my $yTop = lat2y1($MERCATOR_TOP);
    my $yBottom = lat2y1($MERCATOR_BOTTOM);

    $y = ($height*($y - $yTop))/($yBottom - $yTop);

    #debug("latlon2xy($lat,$lon,$zoom) = $x,$y");
    return int($x), int($y);
}


######################################################################
# Check if $filename is a valid map image
# Check if there ist a result in it or only Blank
######################################################################
sub is_usefull_map_file($){
    my $filename = shift;
    $filename = "$mapdir/$filename" unless $filename =~ m,^/,;
    my $size = (-s $filename)||0;
    return 0 if $size < 1000;
    if ( ( $size == 2085 ) ||
	 ( $size == 1755 ) ) {
	# For GoogleSat Maps
	my $content = slurp($filename);
	return 0 if $content =~ m,<title>403 Forbidden</title>,;
	my ($checksum) = split(/\s+/,`md5sum $filename`);
	if ( $checksum eq "c2b3a15d665ba8d2c5aed77025c41a6e" ) {
#	    print "checksumm sees no content \n";
	    return 0;
	};
    }
    return 1;
}

sub write_wld($$$$){
    my $filename = shift;
    my $lat      = shift;
    my $lon      = shift;
    my $zoom     = shift;

    my $wld_filename = $filename;
    die "wrong Filename $filename for making wld file\n"
	unless $wld_filename =~ s/\.(jpg|gif)$/.wld/;
#    print "$wld_filename,$lat,$lon,$zoom\n";
    my $fw =IO::File->new(">$wld_filename");
    print $fw "$zoom\n";
    print $fw "0.0\n";
    print $fw "0.0\n";
    print $fw "$zoom\n";
    print $fw "$lat\n";
    print $fw "$lon\n";
    $fw->close();
}

sub google_stitch($$$$$$) {
    my ($lat, $lon, $scale, $width, $height , $destination_file )= @_;
    my $gpsdrivehome = "$ENV{'HOME'}/.gpsdrive";
    

    my $zoom  = $Scale2Zoom->{$mapserver}->{$scale};

    # Directory for mirroring Google tiles
    my $google_mirror_dir = "${mapdir}google_mirror/$zoom";

    my $hwidth  = $width/2;
    my $hheight = $height/2;

    my ($hval, $vval) = latlon2xy ($lat, $lon, $zoom);
    $hval -= int($hwidth);
    $vval -= int($hheight);

    debug("google_stitch($lat($hval), $lon($vval), $scale, $width, $height )");

    my $image=Image::Magick->new();    
    my $anz_found=0;
    my $anz_new=0;
    for(my $yd = 0; $yd < $height; $yd++) {
	my $images=Image::Magick->new();    

	for(my $xd = 0; $xd < $width; $xd++) {
	    my $hpos = $hval + $xd;
	    my $vpos = $vval + $yd;
#	    print "google_stitch: $lat($hpos), $lon($vpos), $scale\n" if $debug;
	    my $googlename = xy2goog($hpos, $vpos, $zoom);
	    my $filename = "$google_mirror_dir/$googlename.jpg";
	    my $url = "http://kh.google.com/kh?v=3&t=$googlename";
	    
	    if ( ! is_map_file($filename) ) {
		if ( mirror_map($url,$filename) ) {
		    if ( is_usefull_map_file($filename) ) {
			$anz_new++;
			write_wld($filename,$lat,$lon,$zoom)
		    }
		}
	    }
	    write_wld($filename,$lat,$lon,$zoom);

	    my $model=Image::Magick->new();
	    if ( is_usefull_map_file($filename) ) {
		$anz_found++;
		$model->ReadImage($filename);
		if ($debug) { # Annotate position and coordinate
		    $model->Draw(fill => 'blue',
				 primitive => 'line',   
				 points => '0,0 0,150');
		    $model->Draw(fill => 'green',
				 primitive => 'line',   
				 points => '0,0 150,0');
		    my $gwp = IO::File->new(">>way_google_stitch.txt");
		    my ($lat,$lon)=split(",",xy2latlon($hpos, $vpos, $zoom, "%.4f,%.4f"));
		    printf $gwp "%d_%d_%d %f %f Map\n",$hpos,$vpos,$zoom,$lat,$lon;
		    $gwp->close();

		    $model->Annotate( %font_description, 
				      text => sprintf("%s\n%d, %d, %d",
						      xy2latlon($hpos, $vpos, $zoom, "(%.3f,%.3f)"),
						      $hpos,$vpos,$zoom),
				      geometry => '+1+12' );
		}

	    }
	    push @$images, $model;
	}
	#print "Stitching stripe N$yd..."  if $debug;
	my $montage=$images->Montage(geometry=>'256x256', tile=>$width."x1");
	#print "done.\n" if $debug;
	push @$image, $montage;
    }

    my $ulh = $hval;
    my $ulv = $vval;

    my $lrh  = $hval+$width;
    my $lrv  = $vval+$height;


    my $fname =  
	xy2latlon($ulh, $ulv, $zoom, "(%.6f)(%.6f)").
	xy2latlon($lrh, $lrv, $zoom, "(%.6f)(%.6f)").
	".jpeg";

    my $include_empty_tiles=0;
    if (  (!$include_empty_tiles) && $anz_found  < ($width* $height) ) {
	return "O";
    } elsif ( $anz_found ) {
	print " Stitching result image... (w:$width * h:$height found:$anz_found) " if $debug;
	$image = $image->Montage(geometry=>(256*$width)."x256", tile=>" 1x$height");
	print "done.\n" if $debug;
	
	if ( $debug ) {
	    my $ulm   =  xy2latlon($ulh, $ulv, $zoom, "%.6f\n%.6f");
	    my $lrm   =  xy2latlon($lrh, $lrv, $zoom, "%.6f\n%.6f");
	    my $mmm   =  xy2latlon($hval+$hwidth, $vval+$hheight, $zoom, "%.6f\n%.6f");
	    print "Annotating result image...\n";
	    $image->Draw(fill=>'red',    primitive=>'rectangle',   points=>'635,507 645,517');
	    $image->Draw(fill=>'yellow', primitive=>'rectangle',   points=>'638,510 642,514');
	    $image->Annotate(%font_description, text=>"$mmm", geometry=>'+640+512' );
	    if (0){
		my $way_txt = IO::File->new(">>/home/gpsdrive/way-test.txt");
		#Muenchen                48.129281   11.573221 area.city
		my $mm1 = $mmm;
		$mm1 =~ s/\s+/_/g;
		$mmm =~ s/\s+/ /g;
		$way_txt->print("$mm1 $mmm\n");
		$way_txt->close();
	    }
	    $image->Annotate(%font_description, text=>"$ulm", geometry=>'+2+54' );
	    $image->Annotate(%font_description, text=>"$lrm", geometry=>'+2-54',gravity=>"SouthEast" );
	    for my $x ([$ulh,$lrh]){
		for my $y ([$ulv,$lrv]){
		    my $pos_txt   =  xy2latlon($x, $y, $zoom, "%.6f\n%.6f");
		    #$image->Draw(fill=>'red',    primitive=>'rectangle',   points=>'635,507 645,517');
		    #$image->Annotate(%font_description, text=>"$lrm", geometry=>'+22+24',gravity=>"SouthEast" );
		}
	    }
	    print "done.\n";
	}

	print "Writing result image ($destination_file)...\n" if $debug;
	$image->Write($destination_file);

	# TODO: $lat,$lon is not the real map center --> This has to be fixed!!!!
	my ($la,$lo) = split(' ',xy2latlon($hval+$hwidth, $vval+$hheight, $zoom, "%.6f %.6f"));
	append_koords($destination_file, $la, $lo, $scale);

	($la,$lo) = split(' ',xy2latlon($hval, $vval, $zoom, "%.6f %.6f"));
	write_wld($destination_file,$la,$lo,$scale);

	if ( $anz_new ) {
	    return ( $anz_found < ($width* $height) ) ? "x":"+"; # incomplete ?
	} else {
	    return ( $anz_found < ($width* $height) ) ? "c":"C"; # incomplete ?
	}
    } else {
	return "O";
    }

}


#############################################################################
# Debug
#############################################################################
sub debug($){
    my $msg = shift;
    return unless $debug;
    print STDERR "DEBUG: $msg\n";
}

#################################################################################

__END__

=head1 NAME

B<gpsfetchmap> Version 1.04

=head1 DESCRIPTION

B<gpsfetchmap> is a program to download maps from a mapserver for use with gpsdrive. 

=head1 SYNOPSIS

B<Common usages:>

gpsfetchmap -w <WAYPOINT NAME> -sc <SCALE> -a <#> -p

gpsfetchmap -la <latitude MM.DDDD> -lo <latitude MM.DDDD> -sc <SCALE> -a <#> -p

gpsfetchmap -sla <start latitude MM.DDDD> -endla <end latitude MM.DDDD> -slo <start longitude MM.DDDD> -endlo <end longitude MM.DDDD> -sc <SCALE> -a <#> -p

gpsfetchmap -sc <SCALE> -a <#> -r <WAYPOINT 1> <WAYPOINT 2> ... <WAYPOINT n> -p

B<All options:>

gpsfetchmap [-w <WAYPOINT NAME>]
            [-la <latitude DD.MMMM>] [-lo <longitude DD.MMMM>] 
            [-sla <start latitude DD.MMMM>] [-endla <end latitude DD.MMMM>]
            [-slo <start longitude DD.MMMM>] [-endlo <end longitude DD.MMMM>]
            [-sc <SCALE>] [-a <#>] [-p] [-m <MAPSERVER>]
            [-u <UNIT>] [-md <DIR>] [-W <FILE>] [-t <FILE>] [-r]
            [-C <FILE>] [-P <PREFIX>] [-F] [-d] [-v] [-h] [-M] [-n] [-U] [-c]

=head1 OPTIONS

=over 8

=item B<-w, --waypoint <WAYPOINT NAME>>
   
Takes a waypoint name and uses the latitude and longitude for that waypoint as
the centerpoint of the area to be covered. Waypoints are read from 'way.txt', 
or file defined by '-W'. 
the special name gpsd asks your local gpsd where your gps thinks you are and uses 
this point as center.
This, '-la' and '-lo', '-sla', '-ela', '-slo' and '-elo' or '-a' is required. 
A special name is gpsd this waypoint asks your gps where you currently are.

=item B<-la,  --lat <latitude DD.MMMM>>

Takes a latitude in format DD.MMMM and uses that as the latitude for the centerpoint of the area
to be covered. Will be overriden by the latitude of waypoint if '-w' is used. This and '-lo', '-w' or '-sla', '-ela', '-slo', '-elo' is required.

=item B<-lo, --lon <longitude DD.MMMM>>

Takes a longitude in format DD.MMMM and uses that as the longitude for the centerpoint of the area
to be covered. Will be overriden by the longitude of waypoint if '-w' is used. This and '-la', '-w' or '-sla', '-ela', '-slo', '-elo' is required.

=item B<-sla --start-lat <start latitude DD.MMMM>>

Takes a latitude in format DD.MMMM and uses that as the start latitude for the area to be covered. Will override '-la' and '-lo' but will be overriden by '-w'. This, '-ela', '-slo' and '-elo' or '-w' or '-la' and '-lo' is required.

=item B<-ela --end-lat <end latitude DD.MMMM>>

Takes a latitude in format DD.MMMM and uses that as the end latitude for the area to be covered. Will override '-la' and '-lo' but will be overriden by '-w'. 
This, '-sla', '-slo' and '-elo' or '-w' or '-la' and '-lo' is required.

=item B<-slo --start-lon <start longitude DD.MMMM>>

Takes a longitude in format DD.MMMM and uses that as the start longitude for the area to be covered. Will override '-la' and '-lo' but will be overriden by '-w'. This, '-sla', '-ela' and '-elo' or '-w' or '-la' and '-lo' is required.

=item B<-elo --end-lon <end longitude DD.MMMM>>

Takes a longitude in format DD.MMMM and uses that as the end longitude for the area to be covered. Will override '-la' and '-lo' but will be overriden by '-w'. This, '-sla', '-ela' and '-slo' or '-w' or '-la' and '-lo' is required.

=item B<-sc, --scale <SCALE>>

Scales of map(s) to download. Default: 50000.

Formats:

   '####'
   - Just this scale.   

   '####,####,####'  
   - All scales in the list. May be combined with other formats.

   '>####'          
   - All scales above and including the number given.

   '<####'           
   - All scales below and including the number given.

   '####-####'       
   - All scales from first to last number given.

=item B<-a, --area <#>>

Area to cover. # of 'units' size square around the centerpoint. You can use a single number
for square area. Or you can use '#x#' to do a rectangle, where the first number is distance
latitude and the second number is distance of longitude. 'units' is read from the configuration 
file (-C) or as defined by (-u).

=item B<-p, --polite>

This causes the program to sleep one second between downloads to be polite to the mapserver. 
Takes an optional value of number of seconds to sleep.

=item B<--mapserver <MAPSERVER>>

Mapserver to download from. Default: 'expedia'.
Currently can use: landsat or expedia.

geoscience, gov_au, incrementp, googlesat and eniro have download stubs, 
but they are !!!NOT!!!! in the right scale.


geoscience

landsat covers the whole world with satelite Photos

gov_au is for Australia

incrementp for japanese Maps

googlesat: Google Satelite Maps

expedia

eniro covers:
 eniro_se Sweden
 eniro_dk Denmark
 eniro_no Norway 
 eniro_fi Finnland

Overview of Area covered by eniro_fi:
 http://maps.eniro.com/servlets/fi_MapImageLocator?profile=Main&center=26.;62.&zoomlevel=1&size=800x600


=item B<-u, --unit <UNIT>>

The measurement system to use. Default is read from configuration file <-C>. Possibles are: 
miles, nautical, kilometers.

=item B<--mapdir <DIR>>

Override the configfiles mapdir with this value.

=item B<-W, --WAYPOINT <FILE>>

File to read waypoints from. Default: '~/.gpsdrive/way.txt'. 

=item B<-t, --track <FILE>>

Download maps that are along a saved track. File is a standard track filed saved from GpsDrive.

=item B<-r, --route>

Download maps that are along a route defined by waypoints. You must give a list of waypoints as parameters separated with space.

=item B<-C, --CONFIG>

File to read for GPSDrive configuration information. Default: '~/.gpsdrive/gpsdriverc'.

=item B<-P, --PREFIX <PREFIX>>

Takes a prefix string to be used as the start of all saved map files. Default: "map_".

=item B<-F, --FORCE>

Force program to download maps without asking you to confirm the download.

=item B<-n>

Dont download anything only tell which maps are missing

=item B<-U>

read map_koord.txt file at Start. Then also check for not downloaded map_*.gif Files 
if they need to be appended to map_koords.txt. 

=item B<--check-koordfile>

Update map_koord.txt: search map Tree if map_*.gif file exist, but cannot
be found in map_koords.txt file. This option first reads the 
map_koord.txt file and checks every Map in the filesystem if it also is 
found in the map_koord.txt file.
If not found it is appended into the map_koord.txt file.

Check map_koord.txt File. This option checks, if every Map also exist
If any Map-File is missing, a file map_koord.txt.new will be created. 
This file can be copied to the original file if checked.

=item B<--check-coverage>

See which areas the maps cover.
Output is simple ASCII Art

=item B<--PROXY>

Set proxy for mirroring image Files

=item B<-d, --debug>

Prints debugging information.

=item B<-v, --version>

Prints version information and exits.

=item B<--help -h -x>

Prints the usage page and exits.

=item B<--MAN -M>

Prints the manual page and exits.

=item B<Download>

When downloading Maps the output reads as folows:


 _ Map already exists in Filesystem
 E Error while downloading Map
 + Map got downloaded 
 C googlestich map from Cache
 c incomplete googlestich map from Cache
 x Downloaded maps for googlestich but incomplete image
 O Not all tiles where found for stitching
 u updated map_koords.txt File
 S Simulate only 

=back

=cut

