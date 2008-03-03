#!/usr/bin/perl
#####################################################################
#
#  This script imports Points of Interest Data into the local
#  waypoints.db database file
#  
#
#  currently supported import formats
#  - basic geoinfo gpx import
#  - basic groundspeak gpx import
#  - basic fon gpx import
#
#  $Id: poi-import.pl 1766 2007-10-25 12:17:55Z dse $
#
####################################################################

BEGIN {
    my $dir = $0;
    $dir =~s,[^/]+/[^/]+$,,;
    unshift(@INC,"$dir/perl_lib");

    # For Debug Purpose in the build Directory
    unshift(@INC,"./perl_lib");
    unshift(@INC,"./scripts/perl_lib");
    unshift(@INC,"../scripts/perl_lib");

    # For DSL
    unshift(@INC,"/opt/gpsdrive/share/perl5");
    unshift(@INC,"/opt/gpsdrive"); # For DSL
};

#use diagnostics;
use strict;
use warnings;

my $Version = '$Revision: 1766 $'; 
$Version =~ s/\$Revision:\s*(\d+)\s*\$/$1/; 

my $VERSION ="poi-manager.pl (c) Guenther Meyer
  Initial Version (Dec,2006) by Guenther Meyer <d.s.e (at) sordidmusic.com>
  Version 0.01 svn-$Version";

use encoding 'utf8';
use utf8;
use DBI;
use File::Find;
use File::Copy;
use XML::Twig;
use Getopt::Std;
use Pod::Usage;

our ($opt_f, $opt_h) = 0;
getopts('f:h');
pod2usage( -exitval => '1',  
           -verbose => '1') if $opt_h;

unless ($opt_f)
  { die "No filename given. Please specify GPX-File for Import/Export!\n"; }

my $file = $opt_f;

our $script_user = $ENV{USER};

my $progress_char = '.';
my $progress_offset = 10;
my $progress_counter = 0;
my ($count,$updated,$inserted) =('0','0','0');
my %poi_types;		# 0: poi_type_id => name	1: name => poi_type_id
my %source_ids;		# 0: source_id => name		1: name => source_id


#####################################################################
#
#  M A I N
# 
  
my ($lat,$lon,$alt,$name,$comment,$private) = (0,0,0,'','','');
my ($source_id,$source) = (1,'unknown');
my ($poi_type,$last_modified) = (1,'unknown','2008-01-01');

my $db_file = "$ENV{HOME}/.gpsdrive/waypoints.db";

my $dbh = DBI->connect("dbi:SQLite:dbname=$db_file",'','');
$dbh->{unicode} = 1;


if ($file)
  {
    get_source_ids(1);
    import_gpx($file);
  } 
else { pod2usage( -exitval => '1', -verbose => '1') }

print STDOUT "\n";
exit (0);


#####################################################################
#
#  check and import gpx file
#
sub import_gpx
{
  $file = shift;
  die ("File '$file' not found!\n") unless (-e $file);

  print STDOUT "\n----- Getting file info -----\n";

  my $twig= new XML::Twig
    ( 
      ignore_elts => { 'wpt' => 1, 'rte' => 1, 'trk' => 1 }
    );
  $twig->parsefile( "$file");	# build the twig
  my $gpx= $twig->root;	# get the root of the twig (gpx)

  print "  File        : $file";
  print "\n  Name        : ".$gpx->first_child('name')->text
    if ($gpx->first_child('name'));
  print "\n  Description : ".$gpx->first_child('desc')->text
    if ($gpx->first_child('desc'));
  print "\n  Author      : ".$gpx->first_child('author')->text
    if ($gpx->first_child('author'));
  print "\n  Time        : ".$gpx->first_child('time')->text
    if ($gpx->first_child('time'));
  print "\n  created by  : ".$gpx->att('creator')."\n"
    if ($gpx->att('creator'));
  my $creator = $gpx->att('creator');

  $twig->purge;
  $count = '0';
  $updated = '0';
  $inserted = '0';

  if ( $creator =~ /geoinfo/i )
    { import_gpx_geoinfo($file); }
  elsif ( $creator =~ /groundspeak/i )
    { import_gpx_groundspeak($file); }
  elsif ( $creator =~ /opencaching/i )
    { import_gpx_opencaching($file); }
  elsif ( $creator =~ /FON/i )
    { import_gpx_fon($file); }
  else
    { die "Unknown GPX file detected!\n" }

  print STDOUT "\n  $count POI-Entries written to Database in total.\n";
  print STDOUT "  - POIs updated   : $updated\n" if ($updated);
  print STDOUT "  - New POIs added : $inserted\n" if ($inserted);
}


#####################################################################
#
# import data from geoinfo gpx into poi table
#
sub import_gpx_geoinfo
{
  print STDOUT "\n----- Parsing Geoinfo GPX -----\n";

  my $twig= new XML::Twig
    (
      ignore_elts => { 'rte' => 1, 'trk' => 1 },
      TwigHandlers => { wpt => \&sub_wpt }
    );
  $twig->parsefile( "$file");
  my $gpx= $twig->root;
  
  sub sub_wpt
  {
    my( $twig, $wpt)= @_;

    ($lat,$lon,$alt,$name,$comment,$private) = (0,0,0,'','','');
    ($source_id,$source) = (1,'unknown');
    ($poi_type,$last_modified) = ('unknown','2007-01-01');

    if ($wpt->att('lat') && $wpt->att('lon') && $wpt->first_child('name'))
    {
      $lat = $wpt->att('lat');
      $lon = $wpt->att('lon');
      $name = $wpt->first_child('name')->text;
      $comment = $wpt->first_child('cmt')->text
        if ($wpt->first_child('cmt'));
      $source = $wpt->first_child('src')->text
        if ($wpt->first_child('src'));
      $last_modified = $wpt->first_child('time')->text
        if ($wpt->first_child('time'));
      $poi_type = $wpt->first_child('type')->text
        if ($wpt->first_child('type'));
      $alt = $wpt->first_child('ele')->text
        if ($wpt->first_child('ele'));
      $private = $wpt->first_child('private')->text
        if ($wpt->first_child('private'));

      $source_id = $source_ids{$source} if ($source_ids{$source});

      db_exec_insert();
      $count++;
      progress_bar();
    }

  }
  $twig->purge;
}


#####################################################################
#
#  import groundspeak geocaching info
#
sub import_gpx_groundspeak
{
  print STDOUT "\n----- Parsing Groundspeak GPX -----\n";

  my $twig= new XML::Twig
    (
      ignore_elts => { 'rte' => 1, 'trk' => 1 },
      TwigHandlers => { wpt => \&sub_geocache }
    );
  $twig->parsefile( "$file");
  my $gpx= $twig->root;
  
  sub sub_geocache
  {
    my( $twig, $wpt)= @_;

    ($lat,$lon,$alt,$name,$comment,$private) = (0,0,0,'','','');
    ($source_id,$source) = (5,'groundspeak');
    ($poi_type,$last_modified) = ('geocache','2007-01-01');
    my ($type,$found)  = ('Geocache|Unknown Cache','');

    if ($wpt->att('lat') && $wpt->att('lon') && $wpt->first_child('name'))
    {
      $lat = $wpt->att('lat');
      $lon = $wpt->att('lon');
      $name = $wpt->first_child('name')->text;
      $comment = $wpt->first_child('desc')->text
        if ($wpt->first_child('desc'));
      $last_modified = $wpt->first_child('time')->text
        if ($wpt->first_child('time'));
      
      if ($wpt->first_child('type'))
      {
        $type = $wpt->first_child('type')->text;
	if ($type =~ /traditional/i)
	  { $poi_type = 'geocache.geocache_traditional' }
	elsif ($type =~ /multi/i)
          { $poi_type = 'geocache.geocache_multi' }
	elsif ($type =~ /webcam/i)
          { $poi_type = 'geocache.geocache_webcam' }
	elsif ($type =~ /virtual/i)
          { $poi_type = 'geocache.geocache_virtual' }
	elsif ($type =~ /earth/i)
          { $poi_type = 'geocache.geocache_earth' }
	elsif ($type =~ /event/i)
          { $poi_type = 'geocache.geocache_event' }
	else { $poi_type = 'geocache' }
      }

      if ($wpt->first_child('sym'))
      {
        $found = $wpt->first_child('sym')->text;
	if ( $found =~ /found/i )
	  { $poi_type = 'geocache.geocache_found'; }
      }

      $source_id = $source_ids{$source} if ($source_ids{$source});

      db_exec_insert();
      $count++;
      progress_bar();
    }

  }
  $twig->purge;
}


#####################################################################
#
#  import opencaching.de geocaching info
#
sub import_gpx_opencaching
{
  print STDOUT "\n----- Parsing opencaching.de GPX -----\n";

  my $twig= new XML::Twig
    (
      ignore_elts => { 'rte' => 1, 'trk' => 1 },
      TwigHandlers => { wpt => \&sub_opencache }
    );
  $twig->parsefile( "$file");
  my $gpx= $twig->root;
  
  sub sub_opencache
  {
    my( $twig, $wpt)= @_;

    ($lat,$lon,$alt,$name,$comment,$private) = (0,0,0,'','','');
    ($source_id,$source) = (6,'opencaching');
    ($poi_type,$last_modified) = ('geocache','2007-01-01');
    my ($type,$found)  = ('Geocache|Unknown Cache','');

    if ($wpt->att('lat') && $wpt->att('lon') && $wpt->first_child('name'))
    {
      $lat = $wpt->att('lat');
      $lon = $wpt->att('lon');
      $name = $wpt->first_child('name')->text;
      $comment = $wpt->first_child('desc')->text
        if ($wpt->first_child('desc'));
      $last_modified = $wpt->first_child('time')->text
        if ($wpt->first_child('time'));

      if ($wpt->first_child('geocache'))
      {
        $type = $wpt->first_child('geocache')->first_child('type')->text;
	if ($type =~ /traditional/i)
	  { $poi_type = 'geocache.geocache_traditional' }
	elsif ($type =~ /multi/i)
          { $poi_type = 'geocache.geocache_multi' }
	elsif ($type =~ /webcam/i)
          { $poi_type = 'geocache.geocache_webcam' }
	elsif ($type =~ /virtual/i)
          { $poi_type = 'geocache.geocache_virtual' }
	elsif ($type =~ /earth/i)
          { $poi_type = 'geocache.geocache_earth' }
	elsif ($type =~ /event/i)
          { $poi_type = 'geocache.geocache_event' }
	else { $poi_type = 'geocache' }
      }

      if ($wpt->first_child('sym'))
      {
        $found = $wpt->first_child('sym')->text;
	if ( $found =~ /found/i )
	  { $poi_type = 'geocache.geocache_found'; }
      }

      $source_id = $source_ids{$source} if ($source_ids{$source});

      db_exec_insert();
      $count++;
      progress_bar();
    }

  }
  $twig->purge;
}


#####################################################################
#
#  import FON access point info
#
sub import_gpx_fon
{
  print STDOUT "\n----- Parsing FON GPX -----\n";

  my $twig= new XML::Twig
    (
      ignore_elts => { 'rte' => 1, 'trk' => 1 },
      TwigHandlers => { wpt => \&sub_fon }
    );

  $twig->parsefile( "$file");
  my $gpx= $twig->root;

  sub sub_fon
  {
    my( $twig, $wpt)= @_;

    ($lat,$lon,$alt,$name,$comment,$private) = (0,0,0,'','','');
    ($source_id,$source) = (8,'fon');
    ($poi_type,$last_modified) = ('wlan.pay.fon','2007-01-01');
    my ($type,$found)  = ('FON Access Point','');

    if ($wpt->att('lat') && $wpt->att('lon') && $wpt->first_child('name'))
    {
      $lat = $wpt->att('lat');
      $lon = $wpt->att('lon');
      $name = $wpt->first_child('name')->text;
      $comment = $wpt->first_child('desc')->text
        if ($wpt->first_child('desc'));
      $last_modified = $wpt->first_child('time')->text
        if ($wpt->first_child('time'));

      $source_id = $source_ids{$source} if ($source_ids{$source});

      db_exec_insert();
      $count++;
      progress_bar();
    }

  }
  $twig->purge;
}


#####################################################################
#
#  get available source_ids from database
#
sub get_source_ids
{
  my $mode = shift || '0';
  my $geo_file = "$ENV{HOME}/.gpsdrive/geoinfo.db";
  my $geo_query = 'SELECT source_id,name FROM source;';

  my $geo_h = DBI->connect("dbi:SQLite:dbname=$geo_file",'','');
  $geo_h->{unicode} = 1;
  my $geo_s=$geo_h->prepare($geo_query) or die $geo_h->errstr;
  $geo_s->execute()                  or die $geo_s->errstr;
 
   while (my @row = $geo_s->fetchrow_array)
   { 
     if ($mode=='1') { $source_ids{$row[1]} = $row[0]; }
     else { $source_ids{$row[0]} = $row[1]; }
   }
   $geo_s->finish;
   $geo_h->disconnect;
}


########################################################################################
# Execute SQL statement
#
sub db_exec($){
    my $statement = shift;

    my $sth = $dbh->prepare($statement);
    unless ( $sth->execute() ) {
        warn "Error in query '$statement'\n";
        $sth->errstr;
        return 0;
    }
    return 1;
}


########################################################################################
# Insert data into database
#
sub db_exec_insert($){
    my $db_insert = sprintf("INSERT INTO poi (name,poi_type,lat,lon,alt,comment,last_modified,source_id,private) VALUES(%s,'$poi_type','$lat','$lon','$alt',%s,'$last_modified','$source_id','$private');",$dbh->quote($name),$dbh->quote($comment));
    db_exec($db_insert);
    return 1;
}


#####################################################################
#
#  get current time formatted in ISO 8601 UTC
#
sub utc_time
{
  (my $sec, my $min, my $hour, my $mday, my $mon,
   my $year, my $wday, my $yday, my $isdst) = gmtime(time);
  my $t = sprintf "%4d-%02d-%02dT%02d:%02dZ",
    1900+$year,$mon+1,$mday,$hour,$min;
  return $t;
}


#####################################################################
#
#  write some output on screen, so that the user won't get
#  nervous while we are processing bigger tables/files
#
sub progress_bar
{
  if ($progress_counter==$progress_offset)
    { $progress_counter = 0; print STDOUT $progress_char; }
  else
    { $progress_counter++; }
}


__END__


=head1 SYNOPSIS
 
poi-manager.pl [-h] [-v] [-b] [-i] [-e] [-p] [-s] [-f GPX-FILE]
 
=head1 OPTIONS
 
=over 2
 
=item B<-h>

Show this help

=item B<-f> GPX-FILE

Set gpx-file, that should be used for import/export.

=item B<-v>

Enable verbose output

=item B<-e>

Export POI data from geoinfo database into gpx file

=item B<-i>

Import POI data from gpx file into geoinfo database
 
=item B<-b>

( NOT YET IMPLEMENTED ! )
Import/Export Data only from/to basic poi table.
Use this option, if you don't need the info stored in the poi_extra table
Default is to use all available data if possible.
 
=item B<-p>

Export only data from table which are not flagged as private.
You may use this option, if you are generating files, that you will give away
to foreign people or third party services.

=item B<-s>

Safe Mode: Don't alter already existing entries in database while importing.

=back
