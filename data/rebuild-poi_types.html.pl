#!/usr/bin/perl -w
#############################################################################
# This script will rebuild file poi_types.html from the entries found in the
# geoinfo poi_type database to give an overview of the available POIs.
#
# Run it from the data directory
#
# 
# $Id$
#
#############################################################################


use DBI;
use strict;

my $database ='DBI:mysql:geoinfo:127.0.0.1:3306';
my $db_user = 'gast';
my $db_pass = 'gast';

my $html =
    "<html>\n<head>\n<title>Available POI-Types in gpsdrive</title>\n".
    "<style type=\"text/css\">\ntable { width:100%; }\n".
    "tr { border-top:5px solid black; }\n".
    "td.id { text-align:right; }\ntd.icon { text-align:center; }\n".
    "</style>\n</head>\n<table>\n".
    "<tr><th>ID</th><th>Name</th><th colspan=\"3\">Icons</th><th>Description</th></tr>\n";


my $dbh = DBI->connect( $database, $db_user, $db_pass,
     { RaiseError => 1, AutoCommit => 0 } )
     or die "Can't connect to database!: $!";
my $db_query =
     "SELECT poi_type_id,name,description FROM poi_type ORDER BY name;";
my $sth = $dbh->prepare($db_query); $sth->execute;


while ( my @poi = $sth->fetchrow_array )
 {
  my $base = '';
  my $name = '';

  my @icon = split(/\./,$poi[1]); 
  for (@icon) { $name .= '&nbsp;&nbsp;&nbsp;&nbsp;&rsaquo;&nbsp;'; }
  $name .= pop(@icon);
   
  if ( index($poi[1],'.')=='-1' )
      { $html .= "<tr><td>&nbsp;</td></tr>\n";
        $base = 'background-color:#6666ff; color:white; font-weight:bold;';
      }

  $html .= "<tr style=\"$base\"><td class=\"id\">$poi[0]</td><td>&nbsp;$name</td>\n";

  $poi[1] =~ s#\.#/#g;
  
  $html .=
        "<td class=\"icon\"><img src=\"./icons/square.big/$poi[1].png\"></td>\n".
        "<td class=\"icon\"><img src=\"./icons/square.small/$poi[1].png\"></td>\n".
        "<td class=\"icon\"><img src=\"./icons/classic/$poi[1].png\"></td>\n".
	"<td>&nbsp;$poi[2]</td></tr>\n";
   }


$dbh->disconnect;

open( POIFILE, ">./poi_types.html" ) or die "Can't open file poi_type.html: $!";
print POIFILE "$html</table>\n</html>";
close(POIFILE);



