package POI::Utils;

use strict;
use warnings;

use IO::File;
use LWP::Debug qw(- -conns -trace);
use LWP::UserAgent;

$|= 1;                          # Autoflush

BEGIN {
    use Exporter   ();
    our ($VERSION, @ISA, @EXPORT, @EXPORT_OK, %EXPORT_TAGS);

    # set the version for version checking
    $VERSION     = 1.00;
    # if using RCS/CVS, this may be preferred
    $VERSION = sprintf "%d.%03d", q$Revision$ =~ /(\d+)/g;

    @ISA     = qw(Exporter);
    @EXPORT  = qw( &debug $debug $debug_range 
		   &stacktrace &mirror_file &file_newer
		   &correct_lat_lon
		   $PROXY $no_mirror $verbose);
    %EXPORT_TAGS = ( );     # eg: TAG => [ qw!name1 name2! ],
    # your exported package globals go here,
    # as well as any optionally exported functions
    @EXPORT_OK   = qw();

}
our @EXPORT_OK;


our $debug;
our $debug_range =''; # Ein Ausdruck ala !Funktion,Funktion
our $PROXY='';
our $no_mirror    = 0;
our $verbose=0;

#############################################################################
# Debug
#############################################################################
sub debug($){
    my $msg = shift;
    return unless $debug || $debug_range;

    my $function = (caller(1))[3];

    if ( $debug_range ) {
	my @debug_ranges = split(/(\,|\s+)\s*/,$debug_range); # Stings separated by ',' or blank
	print STDERR "range: $debug_range\n";
	my $do_debug = 0;
	# Check positive
	foreach my $check ( @debug_ranges) {
	    my $i=0;
	    my ($p,$f,$l,$s) = (1,0,0,0);
	    while ( $p ){
		($p,$f,$l,$s) = caller($i++);
		last unless $p;
		#print STDERR "DEBUG: $i ($p,$f,$l,$s)\n";
		if ( $check =~ s/^\!// ){
		    return
			if $s =~ m{$check};
		} else { # Negativ !...
		    if ($s =~ m{$check}) {
			$do_debug = 1;
		    } else {
			$do_debug = 2
			    unless $do_debug;
		    }
		}
		#print STDERR "check: $check,$s,$do_debug\n";
		
	    }
	}
	return unless $do_debug == 1;
	#stacktrace("Debug");
    }
    print STDERR "$function: " if $verbose;
    print STDERR "$msg\n";
}

#############################################################################
# Stacktrace
#############################################################################
sub stacktrace {
    my $msg = shift || "unnamed";
    my $i = shift || 0;

    print STDERR "STACKTRACE: $msg";
    my $n = 20;
    while ( $n-- >= 0  ) {
	my ($p,$f,$l,$s) = caller($i++);
	last unless $p;
	printf STDERR "STACK($i): File=%-20s Package=%-15s, Line=%-3d called Sub=%-20s\n",$f,$p,$l,$s;
    }
}


#############################################################################
# get File with lwp-mirror
#############################################################################
sub mirror_file($$){
    my $url            = shift;
    my $local_filename = shift;

    my $mirror=1;

    return 1 if $no_mirror;

    # LPW::UserAgent initialisieren
    my $ua = LWP::UserAgent->new;
    $ua->proxy(['http','ftp'],"http://$PROXY/") if $PROXY;
    
    #$ua->level("+trace") if $debug;

    debug("mirror_file($url --> $local_filename)");
    print "mirror_file($url) " if $verbose;
    print "\n" if $debug;
    my $response = $ua->mirror($url,$local_filename);
#    debug(sprintf("success = %d <%s>",$response->is_success,$response->status_line));
    
    if ( ! $response->is_success ) {
	if ( $response->status_line =~ /^304/ ) {
	    print "\tNOT MOD" if $debug ;
	    $mirror=2;
	} else {
	    print "\tCOULD NOT GET ";
	    print "$url\n" unless $debug;
	    print sprintf("ERROR: %s\n",$response->message)
		if $debug;
	    $mirror=0;
	}
    } else {
	print "\tOK" if $debug;	
    }    
    print "\n" if $debug;
    return $mirror;
}

#############################################################################
# check modifikation times
# true if file1 newer than file2 
#############################################################################
sub file_newer($$){
    my $file1 = shift;
    my $file2 = shift;

    my $t_1 = (stat($file1))[9];
    my $t_2 = (stat($file2))[9];
    return 0 unless $t_2;
    debug("file 1 ($file1):".localtime($t_1));
    debug("file 2 ($file2):".localtime($t_2));
    return ($t_1 < $t_1 );
}


#############################################################################
# correct/convert lat/lon to apropriate Format
sub correct_lat_lon($){
    my $point = shift;

    #print "correct_lat_lon(".Dumper($point);

    for my $type ( qw(poi.lat poi.lon) ) {
	next unless defined $point->{$type};
	#                               N123 12.34
	if ( $point->{$type} =~ m/^\s*([NSWE]\d{1,3})\s+(\d+\.\d+)\s*$/ ) {
	    my $val1 = $1;
	    my $val2 = $2;
	    $val1 =~ s/[EN]//;
	    $val1 =~ s/[SW]/-/;
	    $point->{$type}  =  sprintf("%0.9f",$val1+$val2/60);
	}
	#                               N123.34
	if ( $point->{$type} =~ m/^\s*([NSEW]\d{1,3}\.\d+)\s*$/ ) {
	    my $val = $1;
	    $val =~ s/[NE]//;
	    $val =~ s/[SW]/-/;
	    $point->{$type}  =  sprintf("%0.9f",$val);
	}
    }

    unless (  defined($point->{Position}) && $point->{Position} ){
	my $lat = $point->{'poi.lat'};
	my $lon = $point->{'poi.lon'};
	if ( $lat =~ s/^-// ) {
	    $lat ="S$lat";
	} else {
	    $lat ="N$lat";
	}
	if ( $lon =~ s/^-// ) {
	    $lon ="W$lon";
	} else {
	    $lon ="E$lon";
	}
	
	$point->{Position} = "$lat $lon";
    }
}
1;
