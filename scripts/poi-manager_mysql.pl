#!/usr/bin/perl
#####################################################################
#
#  This script handles all things regarding Points of Interest
#  
#
#  currently supported file formats
#  - basic geoinfo gpx export (gpsdrive standard format)
#  - basic geoinfo gpx import
#  - basic groundspeak gpx import
#
#  files that could be supported
#  - groundspeak loc import
#  - ...
#  
#  other optional functionality planned
#  - update existing gpx file from database
#  - export all POIs in a specific area
#  - export only POIs with a specific poi_type
#
#  Functionality that should be transfered from geoinfo.pl
#  - Retrieve POI Data from Different Sources
#    and import them into mySQL for use with gpsdrive
#
#
#  $Id: poi-manager.pl 1766 2007-10-25 12:17:55Z dse $
#
#####################################################################

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
use IO::File;
use File::Find;
use File::Copy;
use XML::Twig;
use Geo::Gpsdrive::DBFuncs;
use Getopt::Std;
use Pod::Usage;

our ($opt_p, $opt_v, $opt_f, $opt_h, $opt_e, $opt_i, $opt_b, $opt_s) = 0;
getopts('pvf:heibs');
pod2usage( -exitval => '1',  
           -verbose => '1') if $opt_h;

unless ($opt_f)
  { die "No filename given. Please specify GPX-File for Import/Export!\n"; }

my $VERBOSE = $opt_v;
my $file = $opt_f;
my $basic = $opt_b;
my $privacy = $opt_p;
my $safe = $opt_s;

our $script_user = $ENV{USER};
our $db_user     = $ENV{DBUSER} || 'gast';
our $db_password = $ENV{DBPASS} || 'gast';
our $db_host     = $ENV{DBHOST} || 'localhost';
our $GPSDRIVE_DB_NAME = "geoinfo";

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
our $dbh = Geo::Gpsdrive::DBFuncs::db_connect();

if ($opt_i)	{ import_gpx($file) }
elsif ($opt_e)	{ export_gpx_geoinfo($file) }

else { pod2usage( -exitval => '1', -verbose => '1') }

print STDOUT "\n";
exit (0);



#####################################################################
#
# export data from poi table into geoinfo gpx file
#
sub export_gpx_geoinfo
{
  my $file = shift;
  die ("File '$file' already existing!\n") if (-e $file);
   
  print STDOUT "\n Exporting POI data from database into file '$file'\n";

  $count = 0;
  get_poi_types();
  get_source_ids();
  open NEWFILE,">:utf8","./$file";
  select NEWFILE;
     
  write_gpx_header('Geoinfo POI Dump','Dump from GPS-Drive Geoinfo Database');
  
  # get poi data from database
  #
  my $db_query = 'SELECT * FROM poi;';
  my $dbh = Geo::Gpsdrive::DBFuncs::db_connect();
  my $sth=$dbh->prepare($db_query) or die $dbh->errstr;
  $sth->execute()               or die $sth->errstr;
  
  # write entries into gpx file
  #
  while (my $row = $sth->fetchrow_hashref)
  {
    unless ( $$row{name} || $$row{lat} || $$row{lon} )
    {
      print STDOUT " Skipping invalid POI-Database Entry Nr. $$row{poi_id}!\n"
    }
    elsif ( $$row{private} || $privacy )
    {
      print STDOUT " Skipping private POI-Database Entry Nr. $$row{poi_id}!\n"
    }
    else
    {
      my $source = 'unknown';
      $source = $source_ids{$$row{source_id}} if ($source_ids{$$row{source_id}});
      
      print"\n<wpt lat=\"$$row{lat}\" lon=\"$$row{lon}\">\n";
      print"  <name>$$row{name}</name>\n";
      print"  <desc>$$row{name}</desc>\n";
      print"  <cmt>$$row{comment}</cmt>\n" if ($$row{comment});
      print"  <src>$source</src>\n";
      print"  <time>$$row{last_modified}</time>\n" if ($$row{last_modified});
      print"  <url>$$row{url}</url>\n" if ($$row{url});
      if ($$row{poi_type_id})
      {
        my $sym = $poi_types{$$row{poi_type_id}};
        $sym =~ s#\..*##;
	print"  <sym>$sym</sym>\n";
        print"  <type>$poi_types{$$row{poi_type_id}}</type>\n";
      }
      else
      {
        print"  <sym>unknown</sym>\n";
        print"  <type>unknown</type>\n";
      }
      print"  <ele>$$row{alt}</ele>\n" if ($$row{alt});
      print"  <proximity>$$row{proximity}</proximity>\n" if ($$row{proximity});
      print"  <poi_extra>\n";
      print"    <address_id>$$row{address_id}</address_id>\n"
        if ($$row{address_id});
      print"    <private>1</private>\n" if ($$row{private});
      print"  </poi_extra>\n";
      print"</wpt>\n";
      $count++;
      progress_bar();
    }
  }
  print"\n</gpx>\n";

  close NEWFILE;
  $sth->finish;
  print STDOUT "  $count Database Entries written.\n";
}


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

  print STDOUT "\n  SAFE MODE enabled !\n" if ($safe);

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
# export data from poi_extra table into gpx file
#
sub export_gpx_geoinfo_extra
{
}


#####################################################################
#
# import data from geoinfo gpx into poi table
#
sub import_gpx_geoinfo
{
  print STDOUT "\n----- Parsing Geoinfo GPX -----\n";
  get_poi_types('1');
  get_source_ids('1');

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

    my ($lat,$lon,$alt,$name,$comment,$private) = (0,0,0,'','','');
    my ($source_id,$source,$proximity) = (1,'unknown',0);
    my ($poi_type_id,$poi_type,$last_modified) = (1,'unknown','2007-01-01');
    my $db_insert = '';

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
      $proximity = $wpt->first_child('proximity')->text
        if ($wpt->first_child('proximity'));
      $private = $wpt->first_child('private')->text
        if ($wpt->first_child('private'));

# TODO: add some handling for poi_extra data .....

      $poi_type_id = $poi_types{$poi_type} if ($poi_types{$poi_type});
      $source_id = $source_ids{$source} if ($source_ids{$source});

# print STDOUT "\n$lat\t$lon\t$name\t$poi_type\n";

      my $dbh = Geo::Gpsdrive::DBFuncs::db_connect();

      # check, if entry already exists in poi table (by comparing name,lat,lon,poi_type_id)
      my $db_query = sprintf("SELECT poi_id FROM poi WHERE lat='$lat' AND lon='$lon' AND name=%s AND poi_type_id='$poi_type_id';",$dbh->quote($name));
      my $sth=$dbh->prepare($db_query) or die $dbh->errstr;
      $sth->execute()               or die $sth->errstr;
      my $row = $sth->fetchrow_hashref;

      if ($$row{poi_id})
      {
        unless ($safe)
	{
	  $db_insert = sprintf("UPDATE poi SET alt='$alt', proximity='$proximity', comment=%s, last_modified='$last_modified', source_id='$source_id', private='$private' WHERE poi_id=$$row{poi_id};",$dbh->quote($comment));
	  $updated++;
	}
      }
      else
      {
        $db_insert = sprintf("INSERT INTO poi (name,poi_type_id,lat,lon,alt,proximity,comment,last_modified,source_id,private) VALUES(%s,'$poi_type_id','$lat','$lon','$alt','$proximity',%s,'$last_modified','$source_id','$private');",$dbh->quote($name),$dbh->quote($comment));
	$inserted++;
      }

      if ($db_insert)
      {
        $sth=$dbh->prepare($db_insert) or die $dbh->errstr;
	$sth->execute()               or die $sth->errstr;
	$count++;
      }
      progress_bar();
    }

  }
  $twig->purge;
}


#####################################################################
#
#  import extra data from geoinfo gpx into poi_extra table
#
sub import_gpx_geoinfo_extra
{
}


#####################################################################
#
#  import groundspeak geocaching info
#
sub import_gpx_groundspeak
{
  print STDOUT "\n----- Parsing Groundspeak GPX -----\n";
  get_poi_types('1');
  get_source_ids('1');

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

    my ($lat,$lon,$alt,$name,$comment,$private) = (0,0,0,'','','');
    my ($source_id,$source,$proximity) = (5,'groundspeak',0);
    my ($poi_type_id,$poi_type,$last_modified) = (5,'geocache','2007-01-01');
    my ($type,$found)  = ('Geocache|Unknown Cache','');
    my $db_insert = '';

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

# TODO: - add some handling for poi_extra data .....
# 	- check for night/mystery/drivein-types

      $poi_type_id = $poi_types{$poi_type} if ($poi_types{$poi_type});
      $source_id = $source_ids{$source} if ($source_ids{$source});

      my $dbh = Geo::Gpsdrive::DBFuncs::db_connect();

      # check, if entry already exists in poi table (by comparing name, which should be unique)
      my $db_query = sprintf("SELECT poi_id FROM poi WHERE name=%s AND poi_type_id='$poi_type_id';",$dbh->quote($name));
      my $sth=$dbh->prepare($db_query) or die $dbh->errstr;
      $sth->execute()               or die $sth->errstr;
      my $row = $sth->fetchrow_hashref;

      if ($$row{poi_id})
      {
	unless ($safe)
	{
	  $db_insert = sprintf("UPDATE poi SET lat='$lat', lon='$lon', proximity='$proximity', comment=%s, last_modified='$last_modified', source_id='$source_id' WHERE poi_id=$$row{poi_id};",$dbh->quote($comment));
	  $updated++;
	}
      }
      else
      {
        $db_insert = sprintf("INSERT INTO poi (name,poi_type_id,lat,lon,alt,proximity,comment,last_modified,source_id,private) VALUES(%s,'$poi_type_id','$lat','$lon','$alt','$proximity',%s,'$last_modified','$source_id','$private');",$dbh->quote($name),$dbh->quote($comment));
	$inserted++;
      }

      if ($db_insert)
      { 
        $sth=$dbh->prepare($db_insert) or die $dbh->errstr;
	$sth->execute()               or die $sth->errstr;
	$count++;
      }
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
  get_poi_types('1');
  get_source_ids('1');

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

    my ($lat,$lon,$alt,$name,$comment,$private) = (0,0,0,'','','');
    my ($source_id,$source,$proximity) = (6,'opencaching',0);
    my ($poi_type_id,$poi_type,$last_modified) = (5,'geocache','2007-01-01');
    my ($type,$found)  = ('Geocache|Unknown Cache','');
    my $db_insert = '';

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

# TODO: - add some handling for poi_extra data .....
# 	- check for night/mystery/drivein-types

      $poi_type_id = $poi_types{$poi_type} if ($poi_types{$poi_type});
      $source_id = $source_ids{$source} if ($source_ids{$source});

      my $dbh = Geo::Gpsdrive::DBFuncs::db_connect();

      # check, if entry already exists in poi table (by comparing name, which should be unique)
      my $db_query = sprintf("SELECT poi_id FROM poi WHERE name=%s AND poi_type_id='$poi_type_id';",$dbh->quote($name));
      my $sth=$dbh->prepare($db_query) or die $dbh->errstr;
      $sth->execute()               or die $sth->errstr;
      my $row = $sth->fetchrow_hashref;

      if ($$row{poi_id})
      {
	unless ($safe)
	{
	  $db_insert = sprintf("UPDATE poi SET lat='$lat', lon='$lon', proximity='$proximity', comment=%s, last_modified='$last_modified', source_id='$source_id' WHERE poi_id=$$row{poi_id};",$dbh->quote($comment));
	  $updated++;
	}
      }
      else
      {
        $db_insert = sprintf("INSERT INTO poi (name,poi_type_id,lat,lon,alt,proximity,comment,last_modified,source_id,private) VALUES(%s,'$poi_type_id','$lat','$lon','$alt','$proximity',%s,'$last_modified','$source_id','$private');",$dbh->quote($name),$dbh->quote($comment));
	$inserted++;
      }

      if ($db_insert)
      { 
        $sth=$dbh->prepare($db_insert) or die $dbh->errstr;
	$sth->execute()               or die $sth->errstr;
	$count++;
      }
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
  get_poi_types('1');
  get_source_ids('1');

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

    my ($lat,$lon,$alt,$name,$comment,$private) = (0,0,0,'','','');
    my ($source_id,$source,$proximity) = (8,'fon',0);
    my ($poi_type_id,$poi_type,$last_modified) = (5,'wlan.pay.fon','2007-01-01');
    my ($type,$found)  = ('FON Access Point','');
    my $db_insert = '';

    if ($wpt->att('lat') && $wpt->att('lon') && $wpt->first_child('name'))
    {
      $lat = $wpt->att('lat');
      $lon = $wpt->att('lon');
      $name = $wpt->first_child('name')->text;
      $comment = $wpt->first_child('desc')->text
        if ($wpt->first_child('desc'));
      $last_modified = $wpt->first_child('time')->text
        if ($wpt->first_child('time'));

      $poi_type_id = $poi_types{$poi_type} if ($poi_types{$poi_type});
      $source_id = $source_ids{$source} if ($source_ids{$source});

      # check, if entry already exists in poi table (by comparing name, which should be unique)
      my $db_query = sprintf("SELECT poi_id FROM poi WHERE name=%s AND poi_type_id='$poi_type_id';",$dbh->quote($name));
      my $sth=$dbh->prepare($db_query) or die $dbh->errstr;
      $sth->execute()               or die $sth->errstr;
      my $row = $sth->fetchrow_hashref;

      if ($$row{poi_id})
      {
	unless ($safe)
	{
	  $db_insert = sprintf("UPDATE poi SET lat='$lat', lon='$lon', proximity='$proximity', comment=%s, last_modified='$last_modified', source_id='$source_id' WHERE poi_id=$$row{poi_id};",$dbh->quote($comment));
	  $updated++;
	}
      }
      else
      {
        $db_insert = sprintf("INSERT INTO poi (name,poi_type_id,lat,lon,alt,proximity,comment,last_modified,source_id,private) VALUES(%s,'$poi_type_id','$lat','$lon','$alt','$proximity',%s,'$last_modified','$source_id','$private');",$dbh->quote($name),$dbh->quote($comment));
	$inserted++;
      }

      if ($db_insert)
      { 
        $sth=$dbh->prepare($db_insert) or die $dbh->errstr;
	$sth->execute()               or die $sth->errstr;
	$count++;
      }
      progress_bar();
    }

  }
  $twig->purge;
}


#####################################################################
#
#  get available poi_types from database
#
sub get_poi_types
{
   my $mode = shift || '0';
   my $db_query = 'SELECT poi_type_id,name FROM poi_type;';
   my $dbh = Geo::Gpsdrive::DBFuncs::db_connect();
   my $sth=$dbh->prepare($db_query) or die $dbh->errstr;
   $sth->execute()               or die $sth->errstr;
 
   while (my @row = $sth->fetchrow_array)
   { 
     if ($mode=='1') { $poi_types{$row[1]} = $row[0]; }
     else { $poi_types{$row[0]} = $row[1]; }
   }
   $sth->finish;
}


#####################################################################
#
#  get available source_ids from database
#
sub get_source_ids
{
   my $mode = shift || '0';
   my $db_query = 'SELECT source_id,name FROM source;';
   my $dbh = Geo::Gpsdrive::DBFuncs::db_connect();
   my $sth=$dbh->prepare($db_query) or die $dbh->errstr;
   $sth->execute()               or die $sth->errstr;
 
   while (my @row = $sth->fetchrow_array)
   { 
     if ($mode=='1') { $source_ids{$row[1]} = $row[0]; }
     else { $source_ids{$row[0]} = $row[1]; }
   }
   $sth->finish;
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


#####################################################################
#
#  write gpx header
#     
sub write_gpx_header
{       
  my $name = shift;
  my $desc = shift; 
  print"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
  print"<gpx\n xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"\n";
  print" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n";
  print" xmlns=\"http://www.topografix.com/GPX/1/0\"\n";
  print" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/0 http://www.topografix.com/GPX/1/0/gpx.xsd\"\n";
  print" version=\"1.0\"\n creator=\"GPSDrive-Geoinfo - www.gpsdrive.cc\">\n\n";
  print"<name>$name</name>\n";
  print"<desc>$desc</desc>\n";
  print"<author>$script_user</author>\n" if ($script_user);
  print"<url>www.gpsdrive.cc</url>\n";
  print"<urlname>GPS-Drive Geoinfo-Database</urlname>\n";
  print"<time>".utc_time()."</time>\n";
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
