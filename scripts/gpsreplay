#!/usr/bin/perl -w

# @(#)$Id$

# Copyright (C) 2002 - 2003 Tim Witham <twitham@surewest.net>

# (see the files README and COPYING for more details)

# replay track logs for gpsdrive by pretending to be a GPS (gpsd)

use strict;
use vars qw/*Server *Client/;	# socket handles
use Socket;
use Tk qw(MainLoop Ev DoOneEvent);
use Time::Local;
use POSIX qw(strftime);

# this should be calculated based on position but gpsdrive doesn't need it:
my $variation = 0;		# degrees magnetic deviation, negative for west
my $filter = 5.0002;		# ignore points < seconds.latlon from last
my $rate = 4;			# update rate, times per second
my $timescale = 1;		# speed up time by this factor
my $format = "%a %b %e %H:%M:%S %Y"; # strftime format for clocks
my $font = '7x13bold';		# font for the input/output text
my $port = 2947;		# port to listen on for gpsdrive (like gpsd)
my $connected = 0;		# whether gpsdrive is connected
my $paused;			# whether replay is paused
my $doc = [];			# documentation defines the bindings!
my $unit = 'nm';		# preferred units display
my $leglabel = '';		# label for the leg
my $alt = 0;			# current altitude

chdir "$ENV{HOME}/.gpsdrive" or warn "can't cd $ENV{HOME}/.gpsdrive: $!\n";
if (open FILE, "gpsdriverc") {
    while (<FILE>) {		# snag units from gpsdrive, if possible
	$unit = $1 if /units = (\S+)/;
    }
    close FILE;
}
$unit =~ s/miles/mi/;		# convert to my format
$unit =~ s/metric/km/;
$unit =~ s/nautic/nm/;

{				# listen for gpsdrive on the gpsd socket
    my $proto = getprotobyname('tcp');
    socket(Server, PF_INET, SOCK_STREAM, $proto) || die "socket: $!";
    setsockopt(Server, SOL_SOCKET, SO_REUSEADDR, pack("l", 1))
	|| die "setsockopt: $!";
    bind(Server, sockaddr_in($port, INADDR_ANY)) || die "bind: $!";
    listen(Server,SOMAXCONN) || die "listen: $!";
}

my $main = MainWindow->new;	# Tk window setup starts here

$main->fileevent(\*Server, 'readable', \&accept); # socket handlers
$SIG{PIPE} = \&close;

my($file, $log, $p, $time, $leg); # all reset by readfile
&readfile;			# initially with DATA below _END_, then
my $tmp = shift @ARGV;		# re-set with command-line or file-open
&readfile($tmp) if $tmp && -s $tmp;

my $top = $main->Frame->pack(qw/-fill x/);
my $canvas = $main->Canvas(qw/-width 800 -height 50 -highlightthickness 0
			   -scrollregion/, [0, 0, 800, 50])->pack;
my $scale = $main->Scale(qw/-orient horizontal -length 800 -variable/
			 => \$time)->pack;

my $bot = $main->Frame->pack(qw/-fill x/);
my $inlabel = $bot->Label(-font => $font)->pack(qw/-side left/);
my $in = $bot->Label(-font => $font)->pack(qw/-side right/);
$bot = $main->Frame->pack(qw/-fill x/);
my $outlabel = $bot->Label(-font => $font)->pack(qw/-side left/);
my $out = $bot->Label(-font => $font)->pack(qw/-side right/);

my $h = $top->Button(qw/-text Help -underline 0 -command/ => \&help
		     )->pack(qw/-side right/);
my $utctime = $top->Label->pack(qw/-side right/);
my $loctime = $top->Label(qw/-relief raised/)->pack(qw/-side right/);
my $menu = $top->Menubutton(qw/-underline 0 -relief raised
			    -text File -direction below/
			    )->pack(qw/-side left/);
$menu->command(qw/-label ~Open... -command/ => sub {
    my $tmp; &readfile($tmp) if $tmp = &fileDialog($main) });
$menu->command(qw/-label ~Filter -command/ => sub {
    &readfile('-reload') if $file});
$menu->separator;
my $rc = $menu->cascade(-label => '~Units');
map {$rc->radiobutton(-label => "~$_", -value => $_, -variable => \$unit,
		      -command => [\&updatelabels]) } qw/nm mi km/;
$menu->command(-label => '~Run gpsdrive', -command
	       => sub { fork || exec 'gpsdrive' });
$menu->separator;
$menu->command(qw/-label ~Quit -command/ => [$main => 'destroy']);
$top->Entry(-textvariable => \$filter, -width => 0)->pack(qw/-side left/);
$top->Button(qw/-text << -command/ => sub { $leg-- }  )->pack(qw/-side left/);
$top->Button(qw/-text  < -command/ => [\&speed, 0, -1])->pack(qw/-side left/);
$top->Button(qw/-text  > -command/ => [\&speed, 0,  1])->pack(qw/-side left/);
$top->Button(qw/-text >> -command/ => sub { $leg++ }  )->pack(qw/-side left/);
my $speedlabel = $top->Label->pack(qw/-side left/);
my $space = $top->Checkbutton(qw/-text Pause -underline 0 -variable/
			      => \$paused)->pack(qw/-side left/);

my $bindings = [[sub {}],	# bindings for the POD =items, in order
		[sub { $time -= 60}],
		[sub { $time += 60 unless $_[1]}, Ev('%s')],
		[sub { $time--}],
		[sub { $time++}],
		[sub { $leg--}],
		[sub { $leg++}],
		[\&speed, -1],
		[\&speed,  1],
		[sub { $space->toggle }],
		[sub { $h->configure(qw/-relief sunken/); DoOneEvent;
		       $h->invoke;
		       $h->configure(qw/-relief raised/) }],
		[$main => 'destroy'],
		];

for (@$doc) {			# cool hack gets bindings from POD below!
    my $binding = shift @$bindings;
    last unless $binding;
    s/E<[^>]+>//g;
    while (m/B(<\S+>)/g) {
	$main->bind($1, $binding);
    }
}
#$main->bind('<KeyPress>' => [ sub { print "$_[1]\n" }, Ev('%K') ]); # key debgr

$main->repeat(1000 / $rate, \&update);
MainLoop;			# no return, we're done!

sub reset {			# reset scale for new leg
    $leg = shift || 1;
    $leg = 1 if $leg < 1 || $leg >= @{$log->{leg}};
    $p = $log->{$log->{leg}[$leg]{begin}};
    $scale->configure(-from => $p->{time}, -to => $log->{leg}[$leg]{end});
    $time = $p->{time};
    $canvas->delete('all');	# draw new speed spectrum graph
    my $q = $p;
    my $maxspeed = $log->{leg}[$leg]{max};
    my $maxalt = $log->{leg}[$leg]{alt} || 1;
    while ($q->{leg} == $leg) {
	my $fast = $q->{speed} / $maxspeed;
	my $color = sprintf "#%02X%02X00", 255 - 255 * $fast, 255 * $fast;
	my $begin = ($scale->coords($q->{time}))[0];
	my $end = ($scale->coords($q->{time} + $q->{diff}))[0];
	$canvas->createRectangle($begin, 45 - 45 * $fast, $end, 49,
				 -fill => $color, -outline => undef);
	my $alt = 45 - 45 * ($q->{alt} / $maxalt);
	$canvas->createLine($begin, $alt, $end, $alt, -fill => '#000000');
	$q = $q->{n};
	last if $q == $p;
    }
    $canvas->createLine(qw/0 45 800 45 -fill black/); # time axis
    my $inc = 1;
    my $max = $scale->get(799, 0);
    for (my $i = $scale->get(0, 0); $i <= $max; $i += $inc) {
	my($s, $m, $h) = localtime $i;
	unless ($s || $m % 5) { # do every 5 minutes at 0 seconds
	    my $x = ($scale->coords($i))[0];
	    if ($m % 60) {	# each 5 minutes
		$canvas->createLine($x, 0, $x, 5, qw/-fill blue/);
	    } elsif ($x > 50 and $x < 750) { # each hour
		$canvas->createText($x, 0,
				    qw/-anchor n -fill red -text/ => $h);
	    }
	    $canvas->createLine($x, 5, $x, 49, # each 15 minutes
				qw/-fill blue -dash/ => [1, 5])
		unless $m % 15;
	    $inc = 300;		# 5 minutes: be more efficient from now on
	}
    }
    $canvas->createText(qw/  0 45 -anchor sw -text/	=> sprintf "%02d:%02d",
			(localtime $log->{leg}[$leg]{begin})[2,1]);
    $canvas->createText(qw/800 45 -anchor se -text/	=> sprintf "%02d:%02d",
			(localtime $log->{leg}[$leg]{end})[2,1]);
    &updatelabels;
}

sub units {			# format distance or speed
    my $nm = shift;
    $nm *= 1.852		if $unit eq 'km';
    $nm *= 1.15077944802	if $unit eq 'mi';
    return $nm;
}

sub updatelabels {		# update only when needed for efficiency
    $inlabel->configure(-text => sprintf
			"%.1f %s in leg %d/%d, segment %d/%d for %2d seconds:",
			&units($log->{leg}[$leg]{dist}), $unit,
			$leg, @{$log->{leg}} - 1, $p->{point},
			$log->{leg}[$leg]{point} - 1, $p->{diff});
    $in->configure(-text => $p->{in});
    $outlabel->configure(-text => (qw/Listening Connected/)[$connected]
			 . " port $port, NMEA OUTPUT:");
}

sub update {			# all the real work is done here, often
    $time += $timescale / $rate unless $paused;
    &reset($leg) unless $leg == $p->{leg};
    unless ($p->{time} <= $time && $time < $p->{n}{time}) { # find line seg
	if ($time >= $log->{end}) {
	    &reset(1);
	} else {
	    $p = $log->{$log->{begin}} if $time < $p->{time};
	    until ($time < $p->{n}{time}) {
		$p = $p->{n};	# find line segment that time is on
	    }
	    &reset($p->{leg}) unless $leg == $p->{leg};
	}
	&updatelabels;
    }
    &reset($p->{leg}) unless $leg == $p->{leg};
    my $pos = ($scale->coords)[0]; # time/speed marker
    $canvas->delete('marker');
    $canvas->createLine($pos, 0, $pos, 49, qw/-tags marker -fill white/);
    $canvas->createText($pos, 25, qw/-tags marker -text/ =>
			sprintf "%.1f", &units($p->{speed}));
    $canvas->createText(qw/  0 5 -anchor nw -tags marker -text/
			=> &hms($time - $log->{leg}[$leg]{begin}));
    $canvas->createText(qw/800 5 -anchor ne -tags marker -text/
			=> &hms($log->{leg}[$leg]{end} - $time));
    $utctime->configure(-text => strftime "  $format UTC  ",
			my($s, $m, $h, $day, $mon, $y) = gmtime $time);
    $loctime->configure(-text => strftime "  $format %Z  ",
			localtime $time);

    my $offset = $time - $p->{time}; # NMEA output to gpsdrive
    my $nmea;
    if ($alt != $p->{alt}) {	# altitude!
	$nmea = sprintf "GPGGA,,,,,,,99,,%05.1f,M,,,,", $p->{alt};
	$nmea = "\$" . $nmea . '*' . &cksum($nmea) . "\r\n";
	print Client $nmea or &close if $connected;	# to gpsdrive
	$alt = $p->{alt};
    }
    $nmea = sprintf "GPRMC,%02d%02d%02d,A,%02d%07.4f,%s,%03d%07.4f,%s,"
	. "%05.1f,%05.1f,%02d%02d%02d,%05.1f,%s", $h, $m, $s,
	&d2dm($p->{lat} + $p->{latinc} * $offset, qw(N S)),
	&d2dm($p->{lon} + $p->{loninc} * $offset, qw(E W)),
	$p->{speed}, $p->{dir}, $day, $mon + 1, $y % 100,
	(&d2dm($variation, qw(E W)))[0,2];
    $nmea = "\$" . $nmea . '*' . &cksum($nmea) . "\r\n";
    print Client $nmea or &close if $connected;	# to gpsdrive
    chomp $nmea;
    $speedlabel->configure(-text => sprintf "%dx", $timescale);
    $out->configure(-text => $nmea);
    $p = $p->{n} and &reset($p->{leg}) if $p->{leg} != $p->{n}{leg}; # skip gaps
}

sub help {
    my $help = $main->Toplevel(qw/-title gpsreplay(1)/);;
    my $t = $help->Scrolled(qw/Text -setgrid true -width  80 -height 32
			    -font normal -wrap none -scrollbars se/)->pack;
    $help->Button(qw/-text Dismiss -command/ => [$help => 'destroy'])->pack;
    $help->bind('<Button-4>', sub { $t->yviewScroll(-5, 'units') });
    $help->bind('<Button-5>', sub { $t->yviewScroll( 5, 'units') });
    $t->pack(qw/-expand yes -fill both/);
    $t->tag(qw/configure bold -foreground blue/);
    $t->tag(qw/configure underline -underline on/);
    $ENV{TERM} = 'xterm';	# get formatted output!
    open PIPE, "perldoc $0 |" or warn "can't run perldoc $0: $!\n";
    while(<PIPE>) {
	while (length $_) {
	    if (!/\010/) {	# backspace
		$t->insert('end', $_);
		$_ = '';
	    } elsif (s/^((.)\010)+\2//) {
		$t->insert ('end', $2, 'bold');
	    } elsif (s/^_\010(.)//) {
		$t->insert ('end', $1, 'underline');
	    } else {
		$t->insert('end', $1) if s/^(.)//s;
	    }
	}
    }
    close PIPE;
}

sub hms {			# return given time formatted as h:m:s
    my $time = shift;
    return sprintf "%02d:%02d:%02d",
    $time / 3600, $time % 3600 / 60, $time % 60;
}

BEGIN {				# tried to pick time scales that make sense:
    my @speed = (1,		#   1 second per second (real time default)
		 2,		#   2 seconds
		 5,		#   5 seconds
		 10,		#  10 seconds
		 15,		# 1/4 minute
		 30,		# 1/2 minute
		 60,		#   1 minute
		 120,		#   2 minutes
		 300,		#   5 minutes
		 600,		#  10 minutes
		 900,		# 1/4 hour
		 1800,		# 1/2 hour
		 );
    my $index = 0;
    sub speed {
	my($w, $accel) = @_;
	if ($accel > 0) {
	    $timescale = $speed[++$index] unless $index >= $#speed;
	} else {
	    $timescale = $speed[--$index] if $index;
	}
    }
}

sub accept {			# accept connection from gpsdrive
    my $paddr;
    $paddr = accept(Client,Server);
    my($port,$iaddr) = sockaddr_in($paddr);
    my $name = gethostbyaddr($iaddr,AF_INET);
    select Client; $| = 1; select STDOUT;
    $connected = 1;
    &updatelabels;
}

sub close {			# close connection from gpsdrive
    close Client;
    $connected = 0;
    &updatelabels;
}

sub fileDialog {		# file open dialog
    my $w = shift;
    my @types = (
		 ["Track Logs", ['.sav', 'track*']],
		 ["All files", '*']);
    return $w->getOpenFile(-filetypes => \@types);
}

# Take a filename of log content and reset global variables.  Good formats:
# gpstrans -m:	1 38.5116076 -121.4091825 34.0 0 12/17/2002 20:26:53
# gpsdrive:	38.376603 -121.963256          0 Tue Dec 24 20:37:54 2002
# garble:	38.7005, -121.257 / Mon Dec 23 05:40:43 2002
sub readfile {			# read GPS tracklog file
    $file = shift unless $_[0] && $_[0] eq '-reload';
    my $self = {};
    my $legp = {};
    my $prev = 0;
    my $reset = 1;
    my $i = 1;
    my $legn = 0;		# leg index
    my @leg = ();
    my %MON;
    map { $MON{$_} = $i++ } qw(Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec);

    my $fh;
    if ($file) {
	open $fh, $file or warn "can't open $file: $!" and return;
    } else {
	$fh = \*DATA;
    }
    while (<$fh>) {
	chomp;
	if (/__END__/) {	# snag wanted bindings from my documentation!
	    while (<$fh>) {
		push @$doc, $_ if /^=item(.+)/;
	    }
	    last;
	}
	s/^\d?\s+//;
	next if /^\#/;		# allow comments
	next if /^\w+log/;	# mayko xmap/hugo header
	unless ($_) {
	    $reset = 1;
	    next;
	}
	my($lat, $lon, $alt, $dt) = split /,?\s+/, $_, 4;
	next if $lat > 90;	# invalid?  1001 == gpsdrive lost GPS signal
	next unless $dt;
	my($i, $mon, $day, $h, $m, $s, $y) = split /[\s:]+/, $dt;
	$mon = $MON{$mon};
	if ($dt =~ s/^(\d+)\s+//) { # gpstrans -m format
	    $alt = $1;
	    ($mon, $day, $y, $h, $m, $s) = split '[\s:/]', $dt;
	}
	my $time = &timegm($s, $m, $h, $day, $mon - 1, $y - 1900);
	if ($prev) {		# filter too much detail
	    next if $time - $prev->{time} < int($filter);
	    next if abs($prev->{lat} - $lat) < ($filter - int($filter))
		&& abs($prev->{lon} - $lon) < ($filter - int($filter))
	    }
	$self->{begin} = $time unless $self->{begin};
	$self->{end} = $time;
	if ($reset) {		# new leg
	    $legp = $self->{leg}[++$legn] = {
		begin	=> $time,
		end	=> $time,
		point	=> 0,	# logged points
		dist	=> 0,	# total distance
		max	=> 1,	# max speed
		alt	=> 0,	# max altitude
	    };
	}
	$reset = 0;
	my $this = $self->{$time} = { # one point record...
	    in		=> $_,
	    time	=> $time,
	    lat		=> $lat,
	    lon		=> $lon,
	    alt		=> $alt,
	    leg		=> $legn,
	    dir		=> 0,	# these are overwritten when
	    dist	=> 0,	# calculated from the next point
	    diff	=> 1,
	    speed	=> 0,
	    latinc	=> 0,
	    loninc	=> 0,
	    point	=> ++$legp->{point},
	    n		=> $self->{$self->{begin}}, # pointer to the next point
	};
	$prev = $this unless $prev;
	$prev->{n} = $this;	# calculate segment between last and this
	my $dir = &direction($this->{lat}, $this->{lon},
			     $prev->{lat}, $prev->{lon});
	my $dist = &distance($this->{lat}, $this->{lon},
			     $prev->{lat}, $prev->{lon})  / 1.852; # knots?
	my $diff = ($time - $prev->{time}) || 1;
	my $speed = $dist / $diff * 3600;
	my $latinc = ($this->{lat} - $prev->{lat}) / $diff; # change per second
	my $loninc = ($this->{lon} - $prev->{lon}) / $diff;
	$prev->{dir} = $dir;
	$prev->{dist} = $dist;	# update the segment
	$prev->{diff} = $diff;
	$prev->{speed} = $speed;
	$prev->{latinc} = $latinc;
	$prev->{loninc} = $loninc;
	$legp->{end} = $time;	# update leg stats
	$legp->{dist} += $dist;
	$legp->{max} = $speed if $speed > $legp->{max};
	$legp->{alt} = $alt if $alt > $legp->{alt};
	$prev = $this;
    }
    return unless @{$self->{leg}} > 1; # ignore if no legs
    $log = $self;		# reset global variables to new values
    $p = $self->{$self->{begin}};
    $leg = $time = 0;
    $file = '' unless $file;
    $main->configure(-title => "Gpsreplay: $file");
}

sub asin { atan2($_[0], sqrt(1 - $_[0] * $_[0])) }

BEGIN {
    my $PI = 3.14159265358979323846;
    my $DEG2RAD = $PI / 180.0;
    sub distance {		# from gpstrans-0.36/gps/getgpsinfo.c (nautical)
	my($lata, $lona, $latb, $lonb) = @_;
	my $l0 = $lona * $DEG2RAD;
	my $l1 = $lonb * $DEG2RAD;
	my $b0 = $lata * $DEG2RAD;
	my $b1 = $latb * $DEG2RAD;

	return 6371 * 2 * asin(sqrt(cos($b1) * cos($b0)
				    * sin(0.5 * ($l1 - $l0))
				    * sin(0.5 * ($l1 - $l0))
				    + sin(0.5 * ($b1 - $b0))
				    * sin(0.5 * ($b1 - $b0))));
    }
    sub direction {		# compass direction
	my($lata, $lona, $latb, $lonb) = @_; # current, old
	my $dir = atan2($lonb - $lona, $lata - $latb);
	$dir += 2 * $PI if $dir <= 0;
	$dir -= 2 * $PI if $dir > 2 * $PI;
	return 360 - $dir * 180 / $PI;
    }
}

sub cksum {			# NMEA checksum is exclusive or
    my $cksum = 0;
    for (split //, shift) {
	$cksum ^= ord($_);
    }
    return sprintf "%02X", $cksum;
}

sub d2dm {			# number, positive label, negative label
    my($n, @dir) = @_;		# output: degrees, minutes, label
    my $dir = $dir[$n < 0];
    $n = abs($n);
    my $d = int($n);
    return $d, ($n - $d) * 60, $dir;
}
# Default log populates the data structures before a real file is
# opened.  This is 2 legs with gap, good for testing (sitting in USA).
__END__
1 38.000 -100.000 0 0 01/01/2003 00:00:00
1 38.002 -100.002 0 0 01/01/2003 00:15:00

1 38.002 -100.002 0 0 01/01/2003 00:30:00
1 38.000 -100.000 0 0 01/01/2003 01:00:00
__END__

=head1 NAME

gpsreplay - replay GPS track logs for gpsdrive

=head1 SYNOPSIS

gpsreplay [track log file]

=head1 DESCRIPTION

I<gpsreplay> reads GPS track log files and plays them back for
I<gpsdrive>.  This allows you to re-live a trip or examine it in
detail to answer questions like "where did I go?" or "how long was I
there?".

All speeds are graphed on a "ruler" of time so you can easily locate
stops and movement.  Time can be set to any position in the leg by
dragging the slider with B<Button-1>.  The complete list of keyboard
and mouse controls are listed below.  Moving time beyond the end of a
leg moves to the next leg and redraws the time scale.  When the trip
is complete, it starts over from the beginning, looping forever.

Two clocks are displayed above the time scale.  These show the current
simulated GPS time in the I<LOCAL> and I<UTC> timezones.

=head1 ARGUMENTS

=over 8

=item B<track log file>

GPS track log file to initially load.  If none is given, a built-in
default is used.  B<File->>B<Open> can be used to open a new log file
at any time, replacing the current log.  I<gpsreplay> understands
I<gpstrans -m>, I<garble> and I<gpsdrive> track log formats.  Blank
lines are used to split a log into multiple legs.  This may be done
automatically when you turn your GPS off and back on.

=back

The file is filtered according to the number next to the B<File> menu.
A value of 0 will do no filtering.  Increased filtering can be used to
throw out some positional detail while smoothing out speed and
heading.  The current file can be refiltered by adjusting this number
and selecting B<File->>B<Filter>.

=over 8

The B<integer part> of this number indicates the quickest update time
to accept.  Points less than this many seconds since the last point
are ignored.

The B<fractional part> of this number indicates the minimum latitude
or logitude change distance to accept.  Points less than this far from
the last point are ignored.

=back

=head1 CONTROLS

The time-line can be completely controlled by the keyboard and/or
mouse.  All control bindings are listed here including some emacs and
vi-like key bindings.  The Button-4 and Button-5 ones may be the most
convenient if you have a working wheel mouse.

=over 8

=item B<Button-4>, B<Left>, B<b>, B<h>

Move back in time one minute.

=item B<Button-5>, B<Right>, B<f>, B<l>

Move forward in time one minute.

=item B<Shift-Button-4>, B<Shift-Left>, B<Control-b>

Move back in time one second.

=item B<Shift-Button-5>, B<Shift-Right>, B<Control-f>

Move forward in time one second.

=item [B<E<lt>E<lt>>], B<Control-Button-4>, B<Control-Left>, B<less>, B<p>, B<k>

Move to the beginning of the previous leg or the beginning of the log.
The leg number is displayed below the time scale.

=item [B<E<gt>E<gt>>], B<Control-Button-5>, B<Control-Right>, B<greater>, B<n>, B<j>

Move to the beginning of the next leg.  Wraps around to the first leg
after the last leg.

=item [B<E<lt>>], B<Alt-Button-4>, B<Alt-Left>, B<Down>, B<comma>

Slow down playback speed.  The playback speed is displayed below the
time scale.  The slowest speed is 1x (real time) and this is also the
initial default speed.

=item [B<E<gt>>], B<Alt-Button-5>, B<Alt-Right>, B<Up>, B<period>

Increase playback speed.

=item [B<Pause>], B<space>, B<Alt-p>

Pause automatic playback at the current point in time.  Time can still
be moved manually while paused.

=item [B<Help>], B<Alt-h>, B<Control-h>

Open this documentation in a window.

=item B<Escape>, B<q>

Quit the program.

=back

=head1 FOOTER

The footer displays the following information on two lines below the
time scale.

=head2 INPUT

=over 2

=item *

Distance traveled in the current leg

=item *

Current leg number / total legs in the file

=item *

Current line segment / total segments in the leg

=item *

Real time duration of the current line segment

=item *

Log file input line of the last point passed

=back

=head2 OUTPUT

=over 2

=item *

Whether I<Listening> for or I<Connected> to I<gpsdrive> on the given
port number.

=item *

NMEA output being made available to I<gpsdrive>.  Speed and heading
are calculated between two log points and assumed constant for the
segment.  Position is updated linearly between the two points.

=back

=head1 BUGS

I<gpsreplay> assumes logged timestamps are UTC as recorded by the GPS.
But currently, I<gpsdrive> logs local computer time rather than GPS
time.  This causes the clocks to be off.  As a workaround, you can
consider the UTC clock to be local time and ignore the other one.

I consider this to be a bug in I<gpsdrive>.  UTC is the only way to
log a time in an unambiguous way without specifying a timezone.  Also,
GPS time is more precise than some computer time.

=head1 LIMITATIONS

Because of the above bug, it is not valid to re-save a replayed track
log from I<gpsdrive>.  Even if you ignore the wrong timestamps, the
speeds will be wrong if the playback speed was anything other than 1x.
Re-saving a replay is a bad idea anyway because it loses detail.

Since I<gpsreplay> makes it possible to "teleport" rapidly from one
point to another, or even travel backward in time, you may need to
turn off B<Show Track> in I<gpsdrive>.  In order for the track to look
right in I<gpsdrive>, you must not jump around on the time line or
playback at a speed too fast for the map scale.

I<gpsreplay> replaces I<gpsd> to appear like a real GPS to
I<gpsdrive>.  Because of this, only one of the two can be running at
the same time.  I<gpsreplay> must be started before I<gpsdrive>.
I<gpsreplay> can only support one I<gpsdrive> connection at a time.

=head1 AUTHOR

Tim Witham <twitham@surewest.net>

=head1 COPYRIGHT

Copyright (C) 2002 - 2003 Tim Witham <twitham@surewest.net>

I<gpsreplay> is released under the conditions of the GNU General
Public License.  See the files README and COPYING in the distribution
for details.

=cut
