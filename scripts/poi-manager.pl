#!/usr/bin/perl
#####################################################################
#
#  This script handles all things regarding Poins of Interest
#  
#  Currently planned:
#
#  - import poi data in gpx format into the database
#  - export/backup poi-data from the database into gpx file
#
#  currently supported file formats
#  - geoinfo gpx export
#
#  files that could be supported
#  - groundspeak gpx import
#  - groundspeak loc import
#  - ...
#  
#  other optional functionality
#  - update existing gpx file from database
#  - export all POIs in a specific area
#  - export only POIs with a specific poi_type
#
#  Functionality that should be transfered from geoinfo.pl
#  - Retrieve POI Data from Different Sources
#    and import them into mySQL for use with gpsdrive
#
#
#  $Id$
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

my $VERSION ="poi-manager.pl (c) Guenther Meyer
  Initial Version (Dec,2006) by Guenther Meyer <d.s.e (at) sordidmusic.com>
  Version 0.01 ";

use utf8;
use IO::File;
use File::Find;
use File::Copy;
use XML::Twig;
use Geo::Gpsdrive::DBFuncs;
use Getopt::Std;
use Pod::Usage;

our ($opt_p, $opt_v, $opt_f, $opt_h, $opt_e, $opt_i, $opt_b) = 0;
getopts('pvf:heib');
pod2usage( -exitval => '1',  
           -verbose => '1') if $opt_h;

unless ($opt_f)
  { die "No filename given. Please specify GPX-File for Import/Export!\n"; }

my $VERBOSE = $opt_v;
my $file = $opt_f;
my $basic = $opt_b;
my $privacy = $opt_p;

our $script_user = $ENV{USER};
our $db_user     = $ENV{DBUSER} || 'gast';
our $db_password = $ENV{DBPASS} || 'gast';
our $db_host     = $ENV{DBHOST} || 'localhost';
our $GPSDRIVE_DB_NAME = "geoinfo";

my $progress_char = '.';
my $progress_offset = 20;
my $progress_counter = 0;
my $count = 0;
my %poi_types;		# 0: poi_type_id => name
			# 1: name => poi_type_id
my %source_ids;		# name => source_id

#####################################################################
#
#  M A I N
# 
export_gpx_geoinfo($file) if ($opt_e);
import_gpx($file) if ($opt_i);

print STDOUT "\n";
exit (0);




# export data from poi table into gpx file
#
sub export_gpx_geoinfo
{
  my $file = shift;
  die ("File '$file' already existing!\n") if (-e $file);
   
  print STDOUT "\n Exporting POI data from database into file '$file'\n";

  $count = 0;
  get_poi_types();

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
      print"\n<wpt lat=\"$$row{lat}\" lon=\"$$row{lon}\">\n";
      print"  <name>$$row{name}</name>\n";
      print"  <desc>$$row{name}</desc>\n";
      print"  <cmt>$$row{comment}</cmt>\n" if ($$row{comment});
      print"  <src>$$row{source_id}</src>\n" if($$row{source_id});
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
#     progress_bar();
    }
  }
  print"\n</gpx>\n";

  close NEWFILE;
  $sth->finish;
  print STDOUT "  $count Database Entries written.\n";
}


#####################################################################
#
#  import gpx file
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

  if ( $creator =~ /geoinfo/i )
    { import_gpx_geoinfo($file); }
  elsif ( $creator =~ /groundspeak/i )
    { import_gpx_groundspeak($file); }
  elsif ( $creator =~ /opencaching/i )
    { import_gpx_opencaching($file); }
  else
    { die "Unknown GPX file detected!\n" }


#  my $twig= new XML::Twig
#    (
#      pretty_print => 'indented',
#      empty_tags => 'normal',
#      comments => 'keep',
#      TwigHandlers => { geoinfo => \&sub_geoinfo }
#    );
#  $twig->parsefile( "$file");	# build the twig
#  my $gpx= $twig->root;	# get the root of the twig (gpx)
#  # Insert new POI-Type entries from hash of available icons
#  #
#  $i = 0;
#  my @tmp_icons = sort(keys(%icons));
#  
#  foreach (@tmp_icons)
#  {
#     insert_poi_type($_,\$rules);
#     $i++;
#  }
#  print STDOUT "  New POI-Types added:\t$i\n";
#
#  # Print Status for poi_type_ids
#  #
#  my @rule= $rules->children;	# get the updated rule list
#
#  my @a_id = '';
#  $i = 0;
#  foreach my $entry (@rule)
#  {
#    if  (my $id =
#         $entry->first_child('geoinfo')->first_child('poi_type_id')->text)
#    {
#      $i++;
#      $a_id[$i] = $id;
#    }
#  }
#  my $id_max = pop(@a_id);
#  my %unused = ('','');
#  for ( my $j = 1; $j<$id_max; $j++ ) { $unused{$j}=$j; } 
#  print STDOUT "  POI-Types defined:\t$i\n";
#  print STDOUT "  Max. poi_type_id:\t$id_max\n";
#  print STDOUT "  Unused IDs:\n  \t\t";
#  foreach (@a_id)
#  { 
#    if (exists $unused{$_}) { delete $unused{$_}; }
#  }
#  foreach (sort(keys(%unused))) { print STDOUT "$_  "; }
#  print STDOUT "\n  \t\t( IDs <31 are reserved )\n\n";
#
#  # Write XML-File containing modified contents
#  #
#  open TMPFILE,">:utf8","./icons.tmp";
#    select TMPFILE;
#    print "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
#    print "<rules>\n";
#    
#    my $j=1;
#
#
#    foreach my $entry (@rule)
#     {
#       $entry->print; 
#       print "\n"; 
#     }
#    print "</rules>\n";
#  close TMPFILE;
#
#  # Create backup copy of old XML-File
#  #
#  move("$file","$file.bak") or die (" Couldn't create backup file!");
#  move("./icons.tmp","$file") or die (" Couldn't remove temp file!");
#  print STDOUT " XML-File successfully updated!\n";
#
#  return;
#
#  # look, if POI-Type already exists in the file by checking for a
#  # known name inside the geoinfo tag. If true, kick it from the icons
#  # hash, because it's not needed anymore.
#  sub sub_geoinfo
#   {
#     my( $twig, $geoinfo)= @_;
#     my $poi_type_id = $geoinfo->first_child('poi_type_id')->text;
#     my $name = $geoinfo->first_child('name')->text;
#
#     if (exists $icons{$name}) 
#     {
#       print STDOUT "  o  $poi_type_id\t\t$name\n" if $VERBOSE;
#       $poi_type_id_base = $poi_type_id if ($poi_type_id > $poi_type_id_base);
#       delete $icons{"$name"};
#     }
#     return;
#   }
#}










}









#####################################################################
#
# export data from poi_extra table into gpx file
#
sub export_gpx_geoinfo_extra
{
}


# import data from geoinfo gpx into poi table
#
sub import_gpx_geoinfo
{
  print STDOUT "\n----- Parsing Geoinfo GPX -----\n";
  get_poi_types('1');
  get_source_ids();

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

      #  poi_extra .....

      $poi_type_id = $poi_types{$poi_type} if ($poi_types{$poi_type});
      $source_id = $source_ids{$source} if ($source_ids{$source});

      print STDOUT "\n$lat\t$lon\t$name\t$comment\n";
      print STDOUT "    $source_id\t$source\n";
      print STDOUT "    $poi_type_id\t$poi_type\n";
      print STDOUT "    $alt\t$proximity\t$private\t$last_modified\n";

# XXX

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
  die "Groundspeak files will be supported soon!\n";
}


#####################################################################
#
#  import opencaching.de geocaching info
#
sub import_gpx_opencaching
{
  print STDOUT "\n----- Parsing opencaching.de GPX -----\n";
  die "Opencaching files will be supported soon!\n";
}


#####################################################################
#
#  get available poi_types from database
#
sub get_poi_types
{
   my $mode = shift;
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

   my $db_query = 'SELECT source_id,name FROM source;';
   my $dbh = Geo::Gpsdrive::DBFuncs::db_connect();
   my $sth=$dbh->prepare($db_query) or die $dbh->errstr;
   $sth->execute()               or die $sth->errstr;
 
   while (my @row = $sth->fetchrow_array)
   { 
     $source_ids{$row[1]} = $row[0];
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





#####################################################################
#
#  Insert new POI-Type into the file
#
#
#sub insert_poi_type
#{
#  my $name = shift(@_);
#  my $twig_root = shift(@_);
#
#  my $new_rule = new XML::Twig::Elt( 'rule');
#  $poi_type_id_base++;
# 
#  my $new_condition = new XML::Twig::Elt('condition');
#  $new_condition->set_att(k=>'poi');
#  $new_condition->set_att(v=>"$name");
#  my $new_title_en = new XML::Twig::Elt('title_en',$default_title_en);
#  my $new_desc_en = new XML::Twig::Elt('description_en',$default_desc_en);
#  my $new_scale_min = new XML::Twig::Elt('scale_min',$default_scale_min);
#  my $new_scale_max = new XML::Twig::Elt('scale_max',$default_scale_max);
#  my $new_poi_type_id = new XML::Twig::Elt('poi_type_id',$poi_type_id_base);
#  my $new_name = new XML::Twig::Elt('name',$name);
#
#  $new_poi_type_id->paste('last_child',$new_rule);
#  $new_name->paste('last_child',$new_rule);
#  $new_rule->insert('geoinfo');
#  $new_desc_en->paste('first_child',$new_rule);
#  $new_title_en->paste('first_child',$new_rule);
#  $new_scale_max->paste('first_child',$new_rule);
#  $new_scale_min->paste('first_child',$new_rule);
#  $new_condition->paste('first_child',$new_rule);
#
#  $new_rule->paste('last_child',$$twig_root); 
#
#  print STDOUT "  +  $poi_type_id_base\t\t$name\n" if $VERBOSE;
#  return;
#}



__END__


=head1 SYNOPSIS
 
poi-manager.pl [-h] [-v] [-b] [-i] [-e] [-f GPX-FILE]
 
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

 Import/Export Data only from/to basic poi table.
 Use this option, if you don't need the info stored in the poi_extra table
 Default is to use all available data if possible.
 
=item B<-p>

 Export only data from table which are not flagged as private.
 You may Use this option, if you are generating files, that you will give away
 to foreign people or services.


=back
