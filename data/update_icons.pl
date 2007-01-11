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
#  - Create overview.html from the XML-File to show all
#    available poi_types and icons.
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

our ($opt_v, $opt_f, $opt_h ) = 0;
getopts('hvf:') or $opt_h = 1;
pod2usage( -exitval => '1',  
           -verbose => '1') if $opt_h;

my $file_xml = './icons.xml';
my %icons = ('','');
my %h_icons = ('', '');
my $i = 0;
my $poi_reserved = 30;
my $poi_type_id_base = $poi_reserved;
my $default_scale_min = 1;
my $default_scale_max = 50000;
my $default_title_en = '';
my $default_desc_en = '';
my $VERBOSE = $opt_v;


#####################################################################
#
#  M A I N
#
#
chdir('./map-icons');
unless (-e $file_xml)
{
  create_xml();	# Create a new XML-File if none exists
}
get_icons();			# read available icons from dirs
update_xml();	# parse and update contents  of XML-File
update_overview('en');	# update html overview from XML-File
update_overview('de');
chdir('..');
exit (0);


#####################################################################
#
#  Update HTML Overview of available Icons and POI-Types
#
#
sub update_overview
{
  my $lang = shift || 'en';
  my $file_html = './overview.html';
  unless ($lang eq 'en') { $file_html = "./overview.$lang.html" }

  print STDOUT "\n----- Updating HTML Overview '$file_html' -----\n";

  my $twig = new XML::Twig
    (
      ignore_elts => { 'scale_min' => 1, 'scale_max' => 1 }
    );
  $twig->parsefile( "$file_xml");
  my $rules = $twig->root;
  my @rule = $rules->children;
   
  # create backup of old overview.html
  move("$file_html","$file_html.bak") or die (" Couldn't create backup file!")
    if (-e $file_html);

  # html 'template'
  my $html_head =
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n".
    "  \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n".
    "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" ".
    "xml:lang=\"en\">\n<head>\n<meta http-equiv=\"Content-Type\" ".
    "content=\"text/html; charset=utf-8\" />".
    "<title>Available POI-Types in gpsdrive</title>\n".
    "<style type=\"text/css\">\ntable { width:100%; }\n".
    "tr { border-top:5px solid black; }\n".
    "tr.id { background-color:#6666ff; color:white; font-weight:bold; }\n".
    "td.id { text-align:right; }\ntd.icon { text-align:center; }\n".
    "td.empty { text-align:center; height:32px; }\n".
    "img.big { width:32px; height:32px; }\n".
    "img.small { width:16px; height:16px; }\n".
    "img.classic { max-height:32px; }\n".
    "span.desc { font:x-small italic condensed }\n".
    "</style>\n</head>\n<body>\n<table>\n<tr><th>ID</th><th>Name</th>\n".
    "<th colspan=\"3\">Icons</th><th>Description</th></tr>\n";
  my $html_tdi = "<td class=\"icon\"><img src=\"";
  my $html_td0 = "<td class=\"empty\">&mdash;</td>\n";
  my %out;

  open HTMLFILE,">:utf8","$file_html";
  select HTMLFILE;

  print $html_head;
  
  foreach my $entry (@rule)
  {
    my $content = '';
    my $id = $entry->first_child('geoinfo')->first_child('poi_type_id')->text;
    my $nm = $entry->first_child('geoinfo')->first_child('name')->text;

    my $ti = $default_title_en;
    my @a_ti = $entry->children('title');
    foreach (@a_ti)
    {
      if ($_->att('lang') eq $lang) { $ti = $_->text;}
    }

    my $de = $default_desc_en;
    my @a_de = $entry->children('description');
    foreach (@a_de)
    {
      if ($_->att('lang') eq $lang)
        { $de = '<span class="desc">&nbsp;&nbsp;'.$_->text.'</span>'; }
    }

    my $icon = $nm;
    my $ind = $nm;

    # accentuate base categories
    if ($id <= $poi_reserved)
    {
      $content .= "<tr><td>&nbsp;</td></tr>\n";
      $content .= "<tr class=\"id\"><td class=\"id\">$id</td><td>&nbsp;$nm</td>";
    }
    else
    {
      my $level = ($icon =~ tr#\.#/#);
      my $html_space = '';
      while ($level)
      { $html_space .='&nbsp;&nbsp;&nbsp;&nbsp;&rsaquo;&nbsp;'; $level--; }
      $nm =~ s#.*\.##g;
      $content .= "<tr><td class=\"id\">$id</td><td>&nbsp;$html_space$nm</td>";
    }

    # exchange empty or missing icon files with a char for faster display
    my $icon_sb = "./square.big/$icon.png";
    my $icon_sm = "./square.small/$icon.png";
    my $icon_cl = "./classic/$icon.png";

    if ( -z $icon_sb or not -e $icon_sb )
      { $content .= $html_td0; }
    else 
      { $content .= $html_tdi.$icon_sb."\" class=\"big\" alt=\"$nm\" /></td>\n" }
    if (-z $icon_sm or not -e $icon_sm )
      { $content .= $html_td0; }
    else 
      { $content .= $html_tdi.$icon_sm."\" class=\"small\" alt=\"$nm\" /></td>\n" }
    if (-z $icon_cl or not -e $icon_cl )
      { $content .= $html_td0; }
    else 
      { $content .= $html_tdi.$icon_cl."\" class=\"classic\" alt=\"$nm\" /></td>\n" }
    $content .= "<td>$ti<br>$de</td></tr>\n";
    $out{$ind} = $content;
  }
  
  # sorted output
  my @to_sort = keys(%out);
  my @sorted = sort(@to_sort);
  foreach (@sorted)
  {
    print $out{$_};
  }

  print "</table>\n</body>\n</html>";
  close HTMLFILE;
  $twig->purge;
  return;

}


#####################################################################
#
#  Parse available XML-File aund update with contents from icons dirs
#
#
sub update_xml
{
  print STDOUT "\n----- Parsing and updating '$file_xml' -----\n";
  
  # Parse XML-File and look for already existing POI-Type entries
  #
  my $twig= new XML::Twig
    (
      pretty_print => 'indented',
      empty_tags => 'normal',
      comments => 'keep',
      TwigHandlers => { geoinfo => \&sub_geoinfo }
    );
  $twig->parsefile( "$file_xml");	# build the twig
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
  for ( my $k = 1; $k<$id_max; $k++ ) { $unused{$k}=$k; } 
  print STDOUT "  POI-Types defined:\t$i\n";
  print STDOUT "  Max. poi_type_id:\t$id_max\n";
  print STDOUT "  Unused IDs:\n  \t";
  foreach (@a_id)
  { 
    if (exists $unused{$_}) { delete $unused{$_}; }
  }
  foreach (sort(keys(%unused)))
    { print STDOUT "$_  " if ($_ > $poi_reserved) }
  print STDOUT "\n\n";

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
  move("$file_xml","$file_xml.bak") or die (" Couldn't create backup file!");
  move("./icons.tmp","$file_xml") or die (" Couldn't remove temp file!");
  print STDOUT " XML-File successfully updated!\n";

  $twig->purge;

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
  my $new_title_en = new XML::Twig::Elt('title',$default_title_en);
  $new_title_en->set_att(lang=>'en');
  my $new_desc_en = new XML::Twig::Elt('description',$default_desc_en);
  $new_desc_en->set_att(lang=>'en');
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
}


#####################################################################
#
#  Get all the available icons in data/icons
#
#
sub get_icons
{
  print STDOUT "\n----- Looking for available icons -----\n";
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
  print STDOUT " $i icons for ".keys(%icons)." POI-Types found in data/map-icons\n";
  
}


#####################################################################
#
#  Create a new XML File and fill it with the basic POI-Types
#
#
sub create_xml
 { 
   print STDOUT "\n----- Creating new basic XML-File \"$file_xml\" -----\n";
   print STDOUT "\n  ATTENTION: It is possible, that the IDs will change,\n";
   print STDOUT "\n  so it would be better, if you update an existing icons.xml!\n";
   my @poi_types = (

     { name => 'unknown',
       poi_type_id => '1',
       scale_min => '1',
       scale_max => '50000',
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
       title_en => 'Accommodation',
       title_de => 'Unterkunft',
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
       scale_max => '50000',
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
       scale_max => '25000',
       description_en => 'POIs not suitable for another category, and custom types',
       description_de => 'Eigenkreationen, und Punkte, die in keine der anderen Kategorien passen',
       title_en => 'Miscellaneous',
       title_de => 'Verschiedenes',
     },
     { name => 'waypoint',
       poi_type_id => '21',
       scale_min => '1',
       scale_max => '50000',
       description_en => 'Waypoints, for example  to temporarily mark several places',
       description_de => 'Wegpunkte, um z.B. temporäre Punkte zu markieren',
       title_en => 'Waypoint',
       title_de => 'Wegpunkt',
     },
    
   );

   open NEWFILE,">:utf8","./$file_xml";
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

   if (-e $file_xml)
     { print STDOUT " New XML-File \"$file_xml\" successfully created!\n"; }
   else
     { die " ERROR: Failed in creating new XML-File \"$file_xml\" !\n"; }

 }


__END__


=head1 SYNOPSIS
 
update_icons.pl [-h] [-v] [-f XML-FILE]
 
=head1 OPTIONS
 
=over 2
 
=item B<--h>

 Show this help

=item B<-f> XML-FILE

 Set file, that holds all the necessary icon and poi_type information.
 The default file is 'icons.xml'.

=item B<-v>

 Enable verbose output




=back
