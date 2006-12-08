#!/usr/bin/perl
#####################################################################
#
#  This script handles the XML-Files for the POI-Types in gpsdrive.
#  It has to be run from the data directory.
#  
#  Default actions, when no options are given:
#  - Create basic XML-File if none is available
#  - Search icons directories for PNG files
#  - Add those files as new POI-Types if they are not yet existent
#  - Update the poi_type table in the database
#  
#  $Id:$
#
#  Development work in progress:
#
#  - Rebuild the poi_type database from the xml file
#
#  - Maybe provide an interface to enter easily new title and
#    description fields to the xml file
#   
#  - Create overview index.html from the XML-File  using an XSLT to
#    show all available poi_types and icons.
#
#####################################################################
#
#  Scheme for the entries (only poi_type relevant elements are shown):
#
#  <rule>
#    <condition k="poi" v="$NAME" />
#    <scale_min>$SCALE_MIN</scale_min>
#    <scale_max>$SCALE_MAX</scale_max>
#    <title lang="$LANG">$TITLE</title>
#    <description lang="$LANG">$DESCRIPTION</description>
#    <geoinfo>
#      <poi_type_id>$POI_TYPE_ID</poi_type_1id>
#      <name>$NAME</name>
#    </geoinfo>
#  </rule>
#
#####################################################################

#use diagnostics;
use strict;
use warnings;

use utf8;
use IO::File;
use File::Find;
use File::Copy;
use XML::Twig;
use Getopt::Std;
use Pod::Usage;

our ($opt_v, $opt_f, $opt_l, $opt_h, $opt_d) = 0;
getopts('hdvf:l:') or $opt_h = 1;
pod2usage( -exitval => '1',  
           -verbose => '1') if $opt_h;

my $file_xml = './icons.xml';
my $file_html = './index.html';
my %icons = ('','');
my %h_icons = ('', '');
my $i = 0;
my $poi_type_id_base = 30;
my $default_scale_min = 1;
my $default_scale_max = 20000;
my $default_title_en = 't i t l e';
my $default_desc_en = 'd e s c r i p t i o n';
my $lang = 'de';

# parsing options
#
my $VERBOSE = $opt_v;
$lang = $opt_l if $opt_l;


#####################################################################
#
#  M A I N
#
#
unless ($opt_d)
{
  unless (-e $file_xml)
  {
    create_xml($file_xml);	# Create a new XML-File if none exists
  }
  get_icons();			# read available icons from dirs
  update_xml($file_xml);	# parse and update contents  of XML-File
  update_overview($file_html);	# update html overview from XML-File
}

update_db($file_xml,$lang);			# update databse from XML-File


exit (0);


#####################################################################
#
#  Update HTML Overview of available Icons and POI-Types
#
#
sub update_overview
{
  my $file = shift(@_);
  print STDOUT "\n----- Updating HTML Overview '$file' -----\n";
  
  print STDOUT "  NOT YET IMPLEMENTED !\n";

  return;  
}


#####################################################################
#
#  Update poi_type Table in geoinfo Database from XML-File
#
#
sub update_db
{
  my $file = shift(@_);
  my $lang = shift(@_);
  
  print STDOUT "\n----- Updating database using local language '$lang' -----\n";
  
  print STDOUT "  NOT YET IMPLEMENTED !\n";

  return;
}


#####################################################################
#
#  Parse available XML-File aund update with contents from icons dirs
#
#
sub update_xml
{
  my $file = shift(@_);
  print STDOUT "\n----- Parsing and updating '$file' -----\n";
  
  # Parse XML-File and look for already existing POI-Type entries
  #
  my $twig= new XML::Twig
    (
      pretty_print => 'indented',
      empty_tags => 'normal',
      comments => 'keep',
      TwigHandlers => { geoinfo => \&sub_geoinfo }
    );
  $twig->parsefile( "$file");	# build the twig
  my $rules= $twig->root;	# get the root of the twig (rules)

  # Insert new POI-Type entries from hash of available icons
  #
  $i = 0;
  my @tmp_icons = sort(keys(%icons));
  
  foreach (@tmp_icons)
  {
     insert_poi_type($_,\$rules);
     $i++;
  }
  print STDOUT "  New POI-Types added:\t$i\n";

  # Print Status for poi_type_ids
  #
  my @rule= $rules->children;	# get the updated rule list

  my @a_id = '';
  $i = 0;
  foreach my $entry (@rule)
  {
    if  (my $id =
         $entry->first_child('geoinfo')->first_child('poi_type_id')->text)
    {
      $i++;
      $a_id[$i] = $id;
    }
  }
  my $id_max = pop(@a_id);
  my %unused = ('','');
  for ( my $j = 1; $j<$id_max; $j++ ) { $unused{$j}=$j; } 
  print STDOUT "  POI-Types defined:\t$i\n";
  print STDOUT "  Max. poi_type_id:\t$id_max\n";
  print STDOUT "  Unused IDs:\n  \t\t";
  foreach (@a_id)
  { 
    if (exists $unused{$_}) { delete $unused{$_}; }
  }
  foreach (sort(keys(%unused))) { print STDOUT "$_  "; }
  print STDOUT "\n  \t\t( IDs <31 are reserved )\n\n";

  # Write XML-File containing modified contents
  #
  open TMPFILE,">:utf8","./icons.tmp";
    select TMPFILE;
    print "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    print "<rules>\n";
    
    my $j=1;


    foreach my $entry (@rule)
     {
       $entry->print; 
       print "\n"; 
     }
    print "</rules>\n";
  close TMPFILE;

  # Create backup copy of old XML-File
  #
  move("$file","$file.bak") or die (" Couldn't create backup file!");
  move("./icons.tmp","$file") or die (" Couldn't remove temp file!");
  print STDOUT " XML-File successfully updated!\n";

  return;

  # look, if POI-Type already exists in the file by checking for a
  # known name inside the geoinfo tag. If true, kick it from the icons
  # hash, because it's not needed anymore.
  sub sub_geoinfo
   {
     my( $twig, $geoinfo)= @_;
     my $poi_type_id = $geoinfo->first_child('poi_type_id')->text;
     my $name = $geoinfo->first_child('name')->text;

     if (exists $icons{$name}) 
     {
       print STDOUT "  o  $poi_type_id\t\t$name\n" if $VERBOSE;
       $poi_type_id_base = $poi_type_id if ($poi_type_id > $poi_type_id_base);
       delete $icons{"$name"};
     }
     return;
   }
}


#####################################################################
#
#  Insert new POI-Type into the file
#
#
sub insert_poi_type
{
  my $name = shift(@_);
  my $twig_root = shift(@_);

  my $new_rule = new XML::Twig::Elt( 'rule');
  $poi_type_id_base++;
 
  my $new_condition = new XML::Twig::Elt('condition');
  $new_condition->set_att(k=>'poi');
  $new_condition->set_att(v=>"$name");
  my $new_title_en = new XML::Twig::Elt('title_en',$default_title_en);
  my $new_desc_en = new XML::Twig::Elt('description_en',$default_desc_en);
  my $new_scale_min = new XML::Twig::Elt('scale_min',$default_scale_min);
  my $new_scale_max = new XML::Twig::Elt('scale_max',$default_scale_max);
  my $new_poi_type_id = new XML::Twig::Elt('poi_type_id',$poi_type_id_base);
  my $new_name = new XML::Twig::Elt('name',$name);

  $new_poi_type_id->paste('last_child',$new_rule);
  $new_name->paste('last_child',$new_rule);
  $new_rule->insert('geoinfo');
  $new_desc_en->paste('first_child',$new_rule);
  $new_title_en->paste('first_child',$new_rule);
  $new_scale_max->paste('first_child',$new_rule);
  $new_scale_min->paste('first_child',$new_rule);
  $new_condition->paste('first_child',$new_rule);

  $new_rule->paste('last_child',$$twig_root); 

  print STDOUT "  +  $poi_type_id_base\t\t$name\n" if $VERBOSE;
  return;
}


#####################################################################
#
#  Get all the available icons in data/icons
#
#
sub get_icons
{
  print STDOUT "\n----- Looking for available icons -----\n";
  chdir('./icons');
  $i = 0;
  find( \&format_icons, ('square.big', 'square.small', 'classic') );
  sub format_icons()
  { 
    if (m/\.png$/ && !m/empty.png$/)
    { 
      $i++;
      my $icon_file = $File::Find::name;
      print STDOUT "  Found icon:\t$i\t$icon_file\n" if $VERBOSE;
      $icon_file =~ s#(^(classic/|square\.big/|square\.small/))|\.png##g;
      $icon_file =~ s#/#.#g;
      $icons{"$icon_file"} = '1';
    }
  }
  delete $icons{''} if (exists $icons{''});
  print STDOUT " $i icons for ".keys(%icons)." POI-Types found in data/icons\n";
  chdir('..');
  
  return;
}


#####################################################################
#
#  Create a new XML File and fill it with the basic POI-Types
#
#
sub create_xml
 { 
   my $file = shift(@_);
   print STDOUT "\n----- Creating new basic XML-File \"$file\" -----\n";
   my @poi_types = (

     { name => 'unknown',
       poi_type_id => '1',
       scale_min => '1',
       scale_max => '25000',
       description_en => 'Unassigned POI',
       description_de => 'Nicht zugewiesener POI',
       title_en => 'Unknown',
       title_de => 'Unbekannt',
     },
     { name => 'accommodation',
       poi_type_id => '2',
       scale_min => '1',
       scale_max => '50000',
       description_en => 'Places to stay',
       description_de => 'Hotels, Jugendherbergen, Campingpl&#228;tze',
       title_en => 'Accomodation',
       title_de => '&#220;bernachtungsm&#246;glichkeit',
     },
     { name => 'education',
       poi_type_id => '3',
       scale_min => '1',
       scale_max => '25000',
       description_en => 'Schools and other educational facilities',
       description_de => 'Schulen und andere Bildungseinrichtungen',
       title_en => 'Education',
       title_de => 'Bildung',
     },
     { name => 'food',
       poi_type_id => '4',
       scale_min => '1',
       scale_max => '25000',
       description_en => 'Restaurants, Bars, and so on...',
       description_de => 'Restaurants, Bars, usw.',
       title_en => 'Food',
       title_de => 'Speiselokal',
     },
     { name => 'geocache',
       poi_type_id => '5',
       scale_min => '1',
       scale_max => '25000',
       description_en => 'Geocaches',
       description_de => 'Geocaches',
       title_en => 'Geocache',
       title_de => 'Geocache',
     },
     { name => 'health',
       poi_type_id => '6',
       scale_min => '1',
       scale_max => '25000',
       description_en => 'Hospital, Doctor, Pharmacy, etc.',
       description_de => 'Krankenh&#228;user, &#196;rzte, Apotheken',
       title_en => 'Health',
       title_de => 'Gesundheit',
     },
     { name => 'money',
       poi_type_id => '7',
       scale_min => '1',
       scale_max => '25000',
       description_en => 'Bank, ATMs, and other money-related places',
       description_de => 'Banken, Geldautomaten, und &#228;hnliches',
       title_en => 'Money',
       title_de => 'Geld',
     },
     { name => 'nautical',
       poi_type_id => '8',
       scale_min => '1',
       scale_max => '50000',
       description_en => 'Special aeronautical Points',
       description_de => 'Spezielle aeronautische Punkte',
       title_en => 'aeronautical',
       title_de => 'aeronautisch',
     },
     { name => 'people',
       poi_type_id => '9',
       scale_min => '1',
       scale_max => '50000',
       description_en => 'Your home, work, friends, and other people',
       description_de => 'Dein Zuhause, die Arbeitsstelle, Freunde, und andere Personen',
       title_en => 'People',
       title_de => 'Person',
     },
     { name => 'places',
       poi_type_id => '10',
       scale_min => '10000',
       scale_max => '500000',
       description_en => 'Settlements, Mountains, and other geographical stuff',
       description_de => 'Siedlungen, Berggipfel, und anderes geografisches Zeug',
       title_en => 'Place',
       title_de => 'Ort',
     },
     { name => 'public',
       poi_type_id => '11',
       scale_min => '1',
       scale_max => '25000',
       description_en => 'Public facilities and Administration',
       description_de => 'Verwaltung und andere &#246;ffentliche Einrichtungen',

       title_en => 'Public',
       title_de => '&#214;ffentlich',
     },
     { name => 'recreation',
       poi_type_id => '12',
       scale_min => '1',
       scale_max => '25000',
       description_en => 'Places used for recreation (no sports)',
       description_de => 'Freizeiteinrichtungen (kein Sport)',
       title_en => 'Recreation',
       title_de => 'Freizeit',
     },
     { name => 'religion',
       poi_type_id => '13',
       scale_min => '1',
       scale_max => '25000',
       description_en => 'Places and facilities related to religion',
       description_de => 'Kirchen und andere religi&#246;se Einrichtungen',
       title_en => 'Religion',
       title_de => 'Religion',
     },
     { name => 'shopping',
       poi_type_id => '14',
       scale_min => '1',
       scale_max => '25000',
       description_en => 'All the places, where you can buy something',
       description_de => 'Orte, an denen man etwas k&#228;uflich erwerben kann',
       title_en => 'Shopping',
       title_de => 'Einkaufen',
     },
     { name => 'sightseeing',
       poi_type_id => '15',
       scale_min => '1',
       scale_max => '25000',
       description_en => 'Historic places and other interesting buildings',
       description_de => 'Historische Orte und andere interessante Bauwerke',
       title_en => 'Sightseeing',
       title_de => 'Sehensw&#252;rdigkeit',
     },
     { name => 'sports',
       poi_type_id => '16',
       scale_min => '1',
       scale_max => '25000',
       description_en => 'Sports clubs, stadiums, and other sports facilities',
       description_de => 'Sportpl&#228;tze und andere sportliche Einrichtungen',
       title_en => 'Sports',
       title_de => 'Sport',
     },
     { name => 'transport',
       poi_type_id => '17',
       scale_min => '1',
       scale_max => '25000',
       description_en => 'Airports and public transportation',
       description_de => 'Flugh&#228;fen und &#246;ffentliche Transportmittel',
       title_en => 'Public Transport',
       title_de => '&#214;ffentliches Transportmittel',
     },
     { name => 'vehicle',
       poi_type_id => '18',
       scale_min => '1',
       scale_max => '25000',
       description_en => 'Facilites for drivers, like gas stations or parking places',
       description_de => 'Dinge f&#252;r Selbstfahrer, z.B. Tankstellen oder Parkpl&#228;tze',
       title_en => 'Vehicle',
       title_de => 'Fahrzeug',
     },
     { name => 'wlan',
       poi_type_id => '19',
       scale_min => '1',
       scale_max => '25000',
       description_en => 'WiFi-related points (Kismet)',
       description_de => 'Accesspoints und andere WLAN-Einrichtungen (Kismet)',
       title_en => 'WLAN',
       title_de => 'WLAN',
     },
     { name => 'misc',
       poi_type_id => '20',
       scale_min => '1',
       scale_max => '20000',
       description_en => 'POIs not suitable for another category, and custom types',
       description_de => 'Eigenkreationen, und Punkte, die in keine der anderen Kategorien passen',
       title_en => 'Miscellaneous',
       title_de => 'Verschiedenes',
     },
    
   );

   open NEWFILE,">:utf8","./$file";
   select NEWFILE;

   print"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
   print"<rules>\n\n";
   foreach (@poi_types)
     {
       print"  <rule>\n";
       print"    <condition k=\"poi\" v=\"$$_{'name'}\" />\n";
       print"    <scale_min>$$_{'scale_min'}</scale_min>\n";
       print"    <scale_max>$$_{'scale_max'}</scale_max>\n";
       print"    <title lang=\"de\">$$_{'title_de'}</title>\n";
       print"    <title lang=\"en\">$$_{'title_en'}</title>\n";
       print"    <description lang=\"de\">$$_{'description_de'}</description>\n";
       print"    <description lang=\"en\">$$_{'description_en'}</description>\n";
       print"    <geoinfo>\n";
       print"      <poi_type_id>$$_{'poi_type_id'}</poi_type_id>\n";
       print"      <name>$$_{'name'}</name>\n";
       print"    </geoinfo>\n";
       print"  </rule>\n\n";
       print STDOUT "  +  $$_{'poi_type_id'}\t\t$$_{'name'}\n" if $VERBOSE;
     }
   print "</rules>\n";
 
   close NEWFILE;

   if (-e $file)
     { print STDOUT " New XML-File \"$file\" successfully created!\n"; }
   else
     { die " ERROR: Failed in creating new XML-File \"$file\" !\n"; }

   return;
 }


__END__


=head1 SYNOPSIS
 
update_icons.pl [-h] [-v] [-d] [-l LANGUAGE] [-f XML-FILE]
 
=head1 OPTIONS
 
=over 2
 
=item B<--h>

 Show this help

=item B<-d>

 When this option is given, only the database will be updated from the
 xml-file. The xml-file itself will not be changed.
 This option should be used together with the '-l' option in the (user)
 install scripts.

=item B<-f> XML-FILE

 Set file, that holds all the necessary icon and poi_type information.
 The default file is 'icons.xml'.

=item B<-l> LANGUAGE

 Set local language, which is used for the database entries:
 The poi_type database can only hold 'title' and 'description' entries
 for english and one additional language. When setting this option, you
 can choose the language, which is used for these entries. If there
 don't exist any entries for the chosen language in the xml file, the
 English values will be used.
 The default language is 'de' for German, if this option is omitted.

=item B<-v>

 Enable verbose output




=back
