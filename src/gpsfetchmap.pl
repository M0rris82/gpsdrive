#!/usr/bin/perl

# gpsfetchmap
#
# Kevin Stephens
# Based on shell script by: Fritz Ganter and Manfred Carus
#
# Nov 29, 2003 <camel\@insecure.at>:
# ive "fixed" the following issues ...
#
# .) previous authors forgot to reset "$long = $slon;" after each "$lat" loop
#    resulting in a non rectangular area of map pieces becaus after each row
#    the column doesnt get reset to the starting-value $slon and so the $lon
#    variable is only increased but never reset ...
#  .) russian style fix so the map pieces of an area do actually overlap vertically.
#     this fix results in ugly after-comma numbers but hey ... at least i do get
#     a real area and not "interlaced" maps with gaps ...
#
#  do a /CAMEL in vi to see all the changes ...
#
#  regards, camel
# 
# Feb 27, 2004 Sorted out expedia downloading (Robin Cornelius)

my $VERSION ="gpsfetchmap (c) 2002 Kevin Stephens <gps\@suburbialost.com>
modified (Sept 06, 2002) by Sven Fichtner <sven.fichtner\@flugfunk.de>
modified (Sept 18, 2002) by Sven Fichtner <sven.fichtner\@flugfunk.de>
modified (Nov 21, 2002) by Magnus Månsson <ganja\@0x63.nu>
modified (Nov 29, 2003) by camel <camel\@insecure.at>
modified (Feb 27,2004) by Robin Cornelius <robin\@cornelius.demon.co.uk>
Version 1.05
";

use Data::Dumper;
use strict;
use warnings;
use Getopt::Long;
use Pod::Usage;
use File::Basename;
use File::Path;
use File::Copy;

# Setup possible scales
my @SCALES = (1000,1500,2000,3000,5000,7500,10000,15000,20000,30000,50000,75000,
              100000,150000,200000,300000,500000,750000,1000000,1500000,2000000,3000000,
              5000000,7500000,10000000,15000000,20000000,30000000,50000000,75000000);

my @EXPEDIAALTS = ( 1, 3, 6, 12, 25, 50, 150, 800, 2000, 7000, 12000);

# Set defaults and get options from command line
Getopt::Long::Configure('no_ignore_case');
my ($lat,$lon,$slat,$endlat,$slon,$endlon,$waypoint,$area,$unit,$mapdir,$debug,$force,$version,$man,$help);
my $failcount  = 0;
my $newcount   = 0;
my $existcount = 0;
my $polite = 'yes';
my $MIN_MAP_BYTES = 4000;   # Minimum Bytes for a map to be accepted as downloaded
my $scale = '100000';
my $CONFIG_DIR    = "$ENV{'HOME'}/.gpsdrive"; # Should we allow config of this?
my $CONFIG_FILE   = "$CONFIG_DIR/gpsdriverc";
my $WAYPT_FILE    = "$CONFIG_DIR/way.txt";
my $KOORD_FILE    = 'map_koord.txt'; # Should we allow config of this?
my $GPSTOOL_MAP_FILE    = "$ENV{'HOME'}/.gpsmap/maps.txt";
my $FILEPREFIX    = 'map_';
my $mapserver     = 'expedia';
my $check_koord_file = 0;
our $MAP_KOORDS={};
our $MAP_FILES={};

GetOptions ( 'lat=f' => \$lat, 'lon=f' => \$lon, 
	     'start-lat=f' => \$slat, 'end-lat=f' => \$endlat, 
	     'start-lon=f' => \$slon, 'end-lon=f' => \$endlon, 
	     'sla=f' => \$slat, 'ela=f' => \$endlat, 
	     'slo=f' => \$slon, 'elo=f' => \$endlon, 
	     'scale=s' => \$scale, 'mapserver=s' => \$mapserver, 
	     'waypoint=s' =>, \$waypoint, 'area=s' => \$area, 
	     'unit=s' => \$unit, 'mapdir=s' => \$mapdir, 'polite:i' => \$polite,
	     'WAYPOINT=s' => \$WAYPT_FILE, 'CONFIG=s' => \$CONFIG_FILE, 
	     'PREFIX=s' => \$FILEPREFIX,
	     'c' => \$check_koord_file,
	     'FORCE' => \$force, 'debug' => \$debug, 'MAN' => \$man, 
	     'help|x' => \$help, 'version' => \$version)
    or pod2usage(1);

pod2usage(1) if $help;
pod2usage(-verbose=>2) if $man;

sub is_map_file($);      # {}
sub expedia_url($$$);    # {}
sub check_koord_file($); # {}
sub read_koord_file($);  # {}
sub append_koords($$$$); # {}
sub expedia_url($$$);    # {}

# Print version
if ($version) {
    print $VERSION, "\n";
    exit();
}

if ( $check_koord_file ) {
    check_koord_file("$CONFIG_DIR/$KOORD_FILE");
    exit();
}

# Verify that we have the options that we need 
pod2usage(1) if (&error_check);


# Setup up some constants
my $EXPEDIAFACT   = 3950;
my $DIFF          = 0.0000028;
my $RADIUS_KM     = 6371.01;
my $LAT_DIST_KM   = 110.87;
my $KM2NAUTICAL   = 0.54;
my $KM2MILES      = 0.62137119;


# Get the list of scales we need
my $SCALES_TO_GET_ref = get_scales(\$scale);
print "Scale to download: ", join(",",sort {$a <=> $b} @{$SCALES_TO_GET_ref}), "\n" if ($debug);

# Get the center waypoint if they want one
if ($waypoint) {
    ($lat,$lon) = get_waypoint(\$waypoint);
}
print "Centerpoint: $lat,$lon\n" if ($debug);

# Get unit from config file, unless they override with command line
$unit = &get_unit unless ($unit);

# Get mapdir from config file, unless they override with command line
$mapdir = &get_mapdir unless ($mapdir);

# Now get the start and end coordinates
unless ($slat && $slon && $endlat && $endlon) {
    ($slat,$slon,$endlat,$endlon) = get_coords(\$lat,\$lon,\$area,\$unit); 
}
print "Upper left: $slat $slon, Lower Right: $endlat, $endlon\n" if ($debug);

my $count = file_count(\($slat,$slon,$endlat,$endlon));
print "You are about to download $count file(s).\n";

unless ($force) {
    print "You are violating the map servers copyright!\nAre you sure you want to continue? [y|n] ";
    my $answer = <STDIN>;
    exit if ($answer !~ /^[yY]/);    
}

# Change into the gpsdrive maps directory 
chdir($CONFIG_DIR);
chdir($mapdir);

print "\nDownloading files:\n";

# Ok start getting the maps
foreach my $scale (@{$SCALES_TO_GET_ref}) {
   # Setup k
   my $k = $DIFF * $scale;
   my $klat = $k - ($k / 2); ### FIX BY CAMEL
   my $klon = $k - ($k / 6); ### FIX BY CAMEL
   my $lati = $slat;   
   my $mapscale=0;
   while ($lati < $endlat) {
       my $long = $slon;
       while ($long < $endlon) {
	   my $filename = "$FILEPREFIX$scale-$lati-$long.gif";
	   print "File: $filename\n";
	   if ( is_map_file($filename)) {
	       $existcount++;
	   } else {
	       my $url;
	       if ($mapserver eq 'expedia') {
		   ($url,$mapscale)=expedia_url($lati,$long,$scale);
	       } else { # of if expedia
		   print "Unknown map sever :", $mapserver, "\n"; 
	       }

	       print "$url\n" if $debug;
	       `wget -nd -q -O tmpmap.gif "$url"`;
	       
	       if (is_map_file('tmpmap.gif')) {
		   rename('tmpmap.gif',$filename);
		   append_koords($filename, $lati, $long, $mapscale);
		   print ".";
		   print "\nWrote $filename\n" if ($debug);
		   $newcount++;
	       } else {
		   $failcount++;
		   print ",";
	       }
	       # sleep if polite is turned on to be nice to the webserver of the mapserver
	       sleep($polite) if ($polite =~ /\d+/);
	       sleep(1) if (!$polite);
	   }
	   $long += $klon; ### FIX BY CAMEL
       }
       $lati += $klat; ### FIX BY CAMEL
       $long = $slon; ### FIX BY CAMEL
   }
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

######################################################################
# Check if $filename is a valis map image
# for now we only check the size and existance
######################################################################
sub is_map_file($){
    my $filename = shift;
    return 1 if ( -s $filename || 0  ) > $MIN_MAP_BYTES ;
    return 0;
}

######################################################################
sub error_check {
    my $status;
    
    # Check for a centerpoint
    unless (($waypoint) || ($lat && $lon) || ($slat && $endlat && $slon && $endlon)) {
	print "ERROR: You must supply a waypoint, latitude and longitude coordinates or starting and ending coordinates for both latitude and longiture\n\n";
	$status++;
    }
    
    # Check for area
    unless ($area || ($slat && $endlat && $slon && $endlon)) {
	print "ERROR: You must define an area to cover or starting and ending coordinates for both latitude and longiture\n\n";
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
    my @SCALES_TO_GET;
    for my $temp_scale (split /,/, $$scale_ref) {
	if ($temp_scale =~ /^\d+$/) {
	    pod2usage(1) unless (grep (/$temp_scale/, @SCALES));
	    push(@SCALES_TO_GET,$temp_scale);
	} elsif ($temp_scale =~ /^>\d+$/) {
	    $temp_scale =~ s/>//;
	    push(@SCALES_TO_GET, grep ($_ >= $temp_scale, @SCALES)); 
	} elsif ($temp_scale =~ /^<\d+$/) {
	    $temp_scale =~ s/<//;
	    push(@SCALES_TO_GET, grep ($_ <= $temp_scale, @SCALES)); 
	} elsif ($temp_scale =~  /-/) {
	    my(@NUMS) = split(/-/,$temp_scale);
	    @NUMS = sort {$a <=> $b} @NUMS;
	    push(@SCALES_TO_GET, grep (($_ >= $NUMS[0]) && ($_ <= $NUMS[1]), @SCALES));
	} else {
	    pod2usage(1);
	}
    }
    return \@SCALES_TO_GET;
} #End get_scales

#############################################################################
sub expedia_url($$$){
    my $lati = shift;
    my $long = shift;
    my $scale = shift;

    my $url='';
    my $ns = $scale/$EXPEDIAFACT;
    my $di = 999999;
    my $found=0;			
    my $i=0;

    for($i=0;$i<11;$i=$i+1){              
	if(abs($ns-$EXPEDIAALTS[$i]) < $di) {
	    $di=$ns-$EXPEDIAALTS[$i];
	    $found=$i;
	}
    }
    my $alti=0;
    $alti=$EXPEDIAALTS[$found];
    my $mapscale= sprintf("%d",$alti * $EXPEDIAFACT);		  		  
    print "Using expedia altitude ", $alti, " for requested scale ", $scale, ":1 actual scale ", $mapscale, ":1\n"
	if ($debug);
    
    my $where;
    if ($long < -30) {
	$where = 'USA0409';
    } else {
	$where = 'EUR0809';
    }
    $url = "http://www.expedia.com/pub/agent.dll?qscr=mrdt&ID=3XNsF.\&"
	."CenP=$lati,$long\&Lang=$where\&Alti=$alti\&Size=1280,1024\&Offs=0.000000,0.000000";
    return ($url,$mapscale);
}


######################################################################
sub file_count {
    my ($slat,$slon,$endlat,$endlon) = @_;
    my $count;
    foreach my $scale (@{$SCALES_TO_GET_ref}) {
	my $k = $DIFF * $scale;
	my $klat = $k - ($k / 2); ### FIX BY CAMEL
	my $klon = $k - ($k / 6); ### FIX BY CAMEL
	my $lati = $$slat;   
	while ($lati < $$endlat) {
	    my $long = $$slon;
	    while ($long < $$endlon) {
		$long += $klon; ### FIX BY CAMEL
		$count++;
	    }
	    $lati += $klat; ### FIX BY CAMEL
	    $long = $slon; ### FIX BY CAMEL
	}
    }
    return($count);
} #end file_count

######################################################################
sub get_waypoint {
    my ($waypoint_ref) = @_;
    
    # If they give just a filename, we should assume they meant the CONFIG_DIR
    $WAYPT_FILE = "$CONFIG_DIR/$WAYPT_FILE" unless ($WAYPT_FILE =~ /\//);
    
    open(WAYPT,"$WAYPT_FILE") || die "ERROR: Can't open: $WAYPT_FILE: $!\n";
    my ($name,$lat,$lon);
    while (<WAYPT>) {
	chomp;
	next unless (/$$waypoint_ref/);
	($name,$lat,$lon) = split(/\s+/);
    }
    close(WAYPT);
    unless (($lat) && ($lon)) {
	print "Unable to find waypoint '$$waypoint_ref' in '$WAYPT_FILE'\n";
	exit;
    }
    return($lat,$lon);
} #End get_waypoint

######################################################################
sub get_unit {
   # If they give just a filename, we should assume they meant the CONFIG_DIR
   $CONFIG_FILE = "$CONFIG_DIR/$CONFIG_FILE" unless ($CONFIG_FILE =~ /\//);
   
   # If not specified on the command line, we read from the config file
   open(CONFIG,"$CONFIG_FILE") || die "Can't open $CONFIG_FILE: $!\n";
   my $unit;
   while (<CONFIG>) {
      next unless (/units\s=/);
      chomp;
      $unit = $_;
      $unit =~ s/units\s=\s//;
   }   
   close(CONFIG);
   return $unit;
} #End get_unit

######################################################################
sub get_mapdir {
   # If they give just a filename, we should assume they meant the CONFIG_DIR  
   $CONFIG_FILE = "$CONFIG_DIR/$CONFIG_FILE" unless ($CONFIG_FILE =~ /\//);

   # If not specified on the command line, we read from the config file
   open(CONFIG,"$CONFIG_FILE") || die "Can't open $CONFIG_FILE: $!\n";
   my $mapdir;
   while (<CONFIG>) {
      next unless (/mapdir\s=/);
      chomp;
      $mapdir = $_;
      $mapdir =~ s/mapdir\s=\s//;
   }
   close(CONFIG);
   return $mapdir;

} #End get_mapdir

######################################################################
sub get_coords {
    my ($lat_ref,$lon_ref,$area_ref,$unit_ref) = @_;
    
    # Figure out if we are doing square area or a rectangle
    my ($lat_dist,$lon_dist);
    if ($$area_ref =~ /x/i) {
	($lat_dist,$lon_dist) = split(/x/i,$$area_ref);
    } else {
	$lat_dist = $$area_ref;
	$lon_dist = $$area_ref;
    }
    print "Latitude distance: $lat_dist, Longitude distance: $lon_dist\n" if ($debug); 
    
    my $lon_dist_km = calc_lon_dist($lat_ref);
    my $lat_offset  = calc_offset($unit_ref,\($lat_dist,$LAT_DIST_KM));
    my $lon_offset  = calc_offset($unit_ref,\($lon_dist,$lon_dist_km));   

    print "LAT_OFFSET = $$lat_offset LON_OFFSET = $$lon_offset \n" if ($debug);
   
    # Ok subtract the offset for the start point
    my $slat = $$lat_ref - $$lat_offset;
    my $slon = $$lon_ref - $$lon_offset;
   
    # Ok add the offset for the start point
    my $elat = $$lat_ref + $$lat_offset;   
    my $elon = $$lon_ref + $$lon_offset;   
    
    return ($slat,$slon,$elat,$elon);
} #End get_coords

######################################################################
sub calc_offset {
    my($unit_ref,$area_ref,$dist_per_degree) = @_;
    
    # Adjust the dist_per_degree for the unit chosen by the user
    if ($$unit_ref =~ /miles/) {
	$$dist_per_degree *= $KM2MILES;   
    } elsif ($$unit_ref =~ /nautic/) {
	$$dist_per_degree *= $KM2NAUTICAL;
    }
    
    # The offset for the coordinate is the distance to travel divided by 
    # the dist per degree   
    my $offset = sprintf("%.7f", ($$area_ref / 2) / $$dist_per_degree);
    
    return(\$offset);
} #End calc_offset

######################################################################
sub calc_lon_dist {
    my ($lat) = @_;
    my $PI  = 3.141592654;
    my $dr = $PI / 180;
    
    # calculate the circumference of the small circle at latitude 
    my $cos = cos($$lat * $dr); # convert degrees to radians
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

    if ( is_map_file($filename) ) {
	# print "$filename $lati $long $mapscale\n";
    } else {
	print "Fehler $filename exitiert nicht\n";
	return 'E';
    }

    if ( ! defined $MAP_FILES->{$filename} ) {
	my $koord_filename = "$mapdir/$KOORD_FILE";
	open(KOORD,">>$koord_filename") || die "Can't open: $koord_filename: $!\n"; 
	printf KOORD "$filename %17.13f %17.13f %17d\n",$lati, $long, $mapscale;
	close KOORD;
	$MAP_FILES->{$filename} = "$lati, $long, $mapscale";
	$MAP_KOORDS->{$mapscale}->{$lati}->{$long} = 1;
    }

    if ( -s $GPSTOOL_MAP_FILE ) {
	if ( ! defined $GPSTOOL_MAP_FILES->{$filename} ) {
	    open(KOORD,">>$GPSTOOL_MAP_FILE") || die "Can't open: $koord_filename: $!\n"; 
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
    print "opening $koord_file\n";
    open(KOORD,"<$koord_file") || die "Can't open: $koord_file: $!\n"; 
    my $anz_files = 0;
    while ( my $line = <KOORD> ) {
	my ($filename ,$lati, $long, $mapscale);
	($filename ,$lati, $long, $mapscale) = split( /\s+/ , $line );
	if (is_map_file( $filename) ) {
	    $MAP_KOORDS->{$mapscale}->{$lati}->{$long} = 1;
	    $MAP_FILES->{$filename} = "$lati, $long, $mapscale";
	    $anz_files ++;
	}
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
    print "opening  $koord_file\n";
    open(KOORD,"<$koord_file") || die "Can't open: $koord_file: $!\n"; 
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
# and check if all FIles it references are existing
sub check_koord_file($) {
    my $koord_file = shift;
    # Change into the gpsdrive maps directory 

    print "Checking all entries in $koord_file\n" if $debug;
    $MAP_FILES={};
    open(KOORD,"<$koord_file") || die "Can't open: $koord_file: $!\n";
    my $anz_files = 0;
    my $missing_files =0;
    while ( my $line = <KOORD> ) {
	my ($filename ,$lati, $long, $mapscale);
	($filename ,$lati, $long, $mapscale) = split( /\s+/ , $line );
	my $full_filename = "$CONFIG_DIR/$filename";
	if ( !is_map_file( $full_filename ) ) {
	    print "ERROR: File $full_filename not found\n";
	    $missing_files ++;
	} else {
	    print "OK:    File $full_filename found\n" if $debug;
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
    if ( $missing_files ) {
	print "Missing Files: $missing_files\n";
	open(KOORD,">$koord_file.new") || die "Can't open: $koord_file.new: $!\n"; 
	foreach my $filename ( keys  %$MAP_FILES )  {
	    printf KOORD "$filename	%s\n", $MAP_FILES->{$filename};
	}
	close KOORD;
	print "wrote $koord_file.new"; ;
    }
}


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

B<All options:>

gpsfetchmap [-w <WAYPOINT NAME>]
            [-la <latitude DD.MMMM>] [-lo <longitude DD.MMMM>] 
            [-sla <start latitude DD.MMMM>] [-endla <end latitude DD.MMMM>]
            [-slo <start longitude DD.MMMM>] [-endlo <end longitude DD.MMMM>]
            [-sc <SCALE>] [-a <#>] [-p] [-m <MAPSERVER>]
            [-u <UNIT>] [-md <DIR>] [-W <FILE>]
            [-C <FILE>] [-P <PREFIX>] [-F] [-d] [-v] [-h] [-M]

=head1 OPTIONS

=over 8

=item B<-w, --waypoint <WAYPOINT NAME>>
   
Takes a waypoint name and uses the latitude and longitude for that waypoint as the centerpoint
of the area to be covered. Waypoints are read from 'way.txt', or file defined by '-W'. This, '-la' and '-lo' or '-sla', '-ela', '-slo', '-elo' is required. 

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

=item B<-m, --mapserver <MAPSERVER>>

Mapserver to download from. Currently can use: 'mapblast' or 'expedia'.Default: 'mapblast'. 

=item B<-u, --unit <UNIT>>

The measurement system to use. Default is read from configuration file <-C>. Possibles are: 
miles, nautical, kilometers.

=item B<-md --mapdir <DIR>>

Override the configfiles mapdir with this value.

=item B<-W, --WAYPOINT <FILE>>

File to read waypoints from. Default: '~/.gpsdrive/way.txt'. 

=item B<-C, --CONFIG>

File to read for GPSDrive configuration information. Default: '~/.gpsdrive/gpsdriverc'.

=item B<-P, --PREFIX <PREFIX>>

Takes a prefix string to be used as the start of all saved map files. Default: "map_".

=item B<-F, --FORCE>

Force program to download maps without asking you to confirm the download.

=item B<-c>

Update map_koord.txt: search map Tree if map_*.gif file exist, but cannot
be found in map_koords.txt file. This option first reads the 
map_koord.txt file and checks every Map in the filesystem if it also is 
found in the map_koord.txt file.
If not found it is appended into the map_koord.txt file.

Check map_koord.txt File. This option checks, if every Map also exist
If any Map-File is missing, a file map_koord.txt.new will be created. 
This file can be copied to the original file if checked.

=item B<-d, --debug>

Prints debugging information.

=item B<-v, --version>

Prints version information and exits.

=item B<--help -h -x>

Prints the usage page and exits.

=item B<--MAN -M>

Prints the manual page and exits.

=back

=cut

