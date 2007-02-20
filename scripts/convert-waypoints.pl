#!/usr/bin/perl
##########################################################################
#
#  This script allows easy transition to the new poi scheme by converting
#  the old waypoint info from the database table 'waypoints' and the file
#  'way.txt' into gpx style files.
#  You can import the created files into the new database with the script
#  poi-manager.pl
#
#  Without any options both files are created, but you can also choose:
#
#  -w  :  only write content from way.txt to way_converted_txt.gpx
#  -d  :  only write content from waypoints table to way_converted_sql.gpx
#
#  -f FILE  :  use other input file than ~/gpsdrive/way.txt
#
#
##########################################################################

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

my $Version = '$Revision$'; 
$Version =~ s/\$Revision:\s*(\d+)\s*\$/$1/; 

my $VERSION ="convert-waypoints.pl (c) Guenther Meyer
  Initial Version (Dec,2006) by Guenther Meyer <d.s.e (at) sordidmusic.com>
  Version 0.1-$Version ";

use utf8;
use IO::File;
use Geo::Gpsdrive::DBFuncs;
use Getopt::Std;

our ($opt_w, $opt_d, $opt_f) = 0;

getopts('wdf:');

my $way_txt = $opt_f || "$ENV{'HOME'}/.gpsdrive/way.txt";
my $file_txt = 'way_converted_txt.gpx';
my $file_sql = 'way_converted_sql.gpx';

our $db_user     = $ENV{DBUSER} || 'gast';
our $db_password = $ENV{DBPASS} || 'gast';
our $db_host     = $ENV{DBHOST} || 'localhost';
our $GPSDRIVE_DB_NAME = "geoinfo";

my $count = 0;

my %wpt_types = (
  'wlan'        =>  'wlan',
  'wlan-wep'    =>  'wlan.wep',
  'rest'        =>  'food.restaurant',
  'mcdonalds'   =>  'food.fastfood.mc-donalds',
  'burgerking'  =>  'food.fastfood.burger-king',
  'hotel'       =>  'accommodation.hotel',
  'shop'        =>  'shopping',
  'monu'        =>  'sightseeing',
  'speedtrap'   =>  'vehicle.speed_trap',
  'nightclub'   =>  'recreation.nightclub',
  'airport'     =>  'transport.airport',
  'golf'        =>  'sports.golf',
  'gasstation'  =>  'vehicle.fuel_station',
  'cafe'        =>  'food.cafe',
  'geocache'    =>  'geocache'
  );

my %poi_types = %{Geo::Gpsdrive::DBFuncs::get_poi_types()};

if ($opt_d)
  { export_waypoints_sql() }
elsif ($opt_w)
  { export_waypoints_txt() }
else
  { export_waypoints_sql();
    export_waypoints_txt();
  }

exit (0);


##########################################################################
#
#  export waypoints table to gpx file
#
#
sub export_waypoints_sql
{
die ("File '$file_sql' already existing!\n") if (-e $file_sql);
   
print STDOUT "\n Exporting Waypoints Data from database into file '$file_sql'\n";

$count = 0;

open NEWFILE,">:utf8","./$file_sql";
select NEWFILE;
     
# get waypoint data from database
#
my $db_query = 'SELECT * FROM waypoints;';
my $dbh = Geo::Gpsdrive::DBFuncs::db_connect();
my $sth=$dbh->prepare($db_query) or die $dbh->errstr;
$sth->execute()               or die $sth->errstr;

write_gpx_header('Geoinfo Waypoints Dump', 'Dump from GPS-Drive Waypoints Database');

# write entries into gpx file
#
while (my $row = $sth->fetchrow_hashref)
{
  unless ( $$row{name} && $$row{lat} && $$row{lon} )
  {
    print STDOUT " Skipping invalid Database Entry Nr. $$row{id}!\n"
  }
  else
  {
    print"\n<wpt lat=\"$$row{lat}\" lon=\"$$row{lon}\">\n";
    print"  <name>$$row{name}</name>\n";
    print"  <desc>$$row{name}</desc>\n";
    print"  <cmt>$$row{comment}</cmt>\n" if ($$row{comment});
    if ($$row{type})
    {
      if ($wpt_types{lc($$row{type})})	# known old waypoint type
      {
        my $sym = $wpt_types{lc($$row{type})};
        $sym =~ s#\..*##;
        print"  <sym>$sym</sym>\n";
        print"  <type>$wpt_types{lc($$row{type})}</type>\n";
      }
      elsif ($poi_types{$$row{type}})	# known new style poi_type
      {
        my $sym = $$row{type};
        $sym =~ s#\..*##;
        print"  <sym>$sym</sym>\n";
	print"  <type>$$row{type}</type>\n";
      }
      else				# unknown waypoint type entry
      {
        print"  <sym>$$row{type}</sym>\n";
	print"  <type>waypoint.wptred</type>\n";
      }
    }
    else				# no waypoint type present
    {
      print"  <sym>waypoint</sym>\n";
      print"  <type>waypoint.wptorange</type>\n";
    }
    print"  <src>way.txt</src>\n";
    print"  <poi_extra>\n";
    print"    <wep>$$row{wep}</wep>\n" if ($$row{wep});
    print"    <macaddr>$$row{macaddr}</macaddr>\n" if ($$row{macaddr});
    print"    <nettype>$$row{nettype}</nettype>\n" if ($$row{nettype});
    print"  </poi_extra>\n";
    print"</wpt>\n";
    $count++;
  }
}
print"\n</gpx>\n";

close NEWFILE;
$sth->finish;
print STDOUT "  $count Database Entries written.\n\n";
}

##########################################################################
#
#  export way.txt to gpx file
#
#
sub export_waypoints_txt
{
  die ("File '$file_txt' already existing!\n") if (-e $file_txt);
     
  print STDOUT "\n Exporting waypoints from $way_txt into file '$file_txt'\n";

  $count = 0;

  # get entries from way.txt and write them to the gpx file
  #
  #
  # way.txt structure: name  lat  lon  type  wlan  action  sqlnr  proximity
  #
  open my $fh_way,"$way_txt" or die ("  Input file $way_txt missing!!!\n");

  open my $fh_new,">:utf8","./$file_txt";
  select $fh_new;

  write_gpx_header('Geoinfo way.txt converted', 'Converted GPS-Drive way.txt waypoints');
  while(<$fh_way>)
  {
    chomp;
    my @row = split /\s+/;

    unless ( $row[0] && $row[1] && $row[2] )
    {
      print STDOUT " Skipping invalid line: $_\n";
    }
    else
    {
      print"\n<wpt lat=\"$row[1]\" lon=\"$row[2]\">\n";
      print"  <name>$row[0]</name>\n";
      print"  <desc>$row[0]</desc>\n";
      if ($row[3])
      {
	if ($wpt_types{lc($row[3])})
        {
          my $sym = $wpt_types{lc($row[3])};
          $sym =~ s#\..*##;
          print"  <sym>$sym</sym>\n";
          print"  <type>$wpt_types{lc($row[3])}</type>\n";
        }
        else
        {
          print"  <sym>".lc($row[3])."</sym>\n";
          print"  <type>waypoint</type>\n";
        }
      }
      else
      {
        print"  <sym>waypoint</sym>\n";
        print"  <type>waypoint</type>\n";
      }
      print"  <src>way.txt</src>\n";
      print"</wpt>\n";
      $count++;
    }
  }
  print"\n</gpx>\n";
 
  close $fh_way;
  close $fh_new;
  print STDOUT "  $count Entries from way.txt written.\n\n";
}


# write gpx header
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
  print"<url>www.gpsdrive.cc</url>\n";
  print"<urlname>GPS-Drive Geoinfo-Database</urlname>\n";
  print"<time>".utc_time()."</time>\n";
}


# get current time formatted in ISO 8601 UTC
#
sub utc_time
{
  (my $sec, my $min, my $hour, my $mday, my $mon,
   my $year, my $wday, my $yday, my $isdst) = gmtime(time);
  my $t = sprintf "%4d-%02d-%02dT%02d:%02dZ",
    1900+$year,$mon+1,$mday,$hour,$min;
  return $t;
}


__END__

