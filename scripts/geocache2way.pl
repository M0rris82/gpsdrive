#!/usr/bin/perl

#
# geocache2way - Kevin Stephens
# 04/30/02
#
# Script to read .loc files from geocaching.com, parse it and add it onto the end of way.txt.
#   an example 'geocaching.loc' file can be found in the GpsBabel source code.
#

use strict;
use XML::Simple;
use Data::Dumper;
use Getopt::Long;

# Open the XML file
my $FILE 	= 'geocaching.loc';
my $OUTFILE = "$ENV{'HOME'}/.gpsdrive/way.txt";
my ($DEBUG,$verbose);
GetOptions ('debug' => \$DEBUG,'file=s' => \$FILE,'output=s' => \$OUTFILE,'verbose' => \$verbose, 'help' => \&usage);

# Setup the XML object
my $xs = new XML::Simple(keyattr => "id");
my $location = $xs->XMLin($FILE);

if ($DEBUG) {
   print Dumper($location);
} else {
   # Check if it is single listing or multiple
   if ($location->{'waypoint'}{'name'}) {
      print_it($location->{'waypoint'});
   } else {
      foreach my $point (@{$location->{'waypoint'}}) {
         print_it($point);
      }
   }
}

sub print_it {
   my ($CACHE_ref) = @_;
   my $id      = $CACHE_ref->{'name'}{'id'};
   my $lat     = $CACHE_ref->{'coord'}{'lat'};
   my $lon     = $CACHE_ref->{'coord'}{'lon'};
   my $name    = $CACHE_ref->{'name'}{'content'};

# These are here for future expansion.
# Waiting for changes to gpsdrive way.txt.
#    my $type    = $CACHE_ref->{'type'};
#    my $URL     = $CACHE_ref->{'link'}{'content'};

   # Clean out extra space
    $name =~ s/^\s*//;
    $name =~ s/\s*$//;
#    $URL  =~ s/\s//g;
   
   if ($verbose) {
      print "$id $lat $lon\n";
   } else {
      open(OUTFILE,">>$OUTFILE") || die "Can't open $OUTFILE\n";
      print OUTFILE "$id $lat $lon\n";
      close(OUTFILE);
      
      # Now change the filename for the dsc file
      my $DSCFILE = $OUTFILE;
      $DSCFILE =~ s/\..*$//;
      open(OUTFILE,">>$DSCFILE.dsc") || die "Can't open $DSCFILE\n";
      print OUTFILE "\$$id\n$name\n\n";
      close(OUTFILE);
   }      
}

sub usage {
   print <<EOP;
Usage:   geoparse.pl [-f | --file <filename>] [-o | --output <filename>] [-v | --verbose] 
                     [-d | --debug] [-h | --help]
   
      -f | --file    = File to parse, takes a filename
      -o | --output  = File to write to. Default: \$HOME/.gpsdrive/way.txt 
      -v | --verbose = Print output to STDOUT instead of a file
      -d | --debug   = Debug
      -h | --help    = This usage screen

EOP
   exit (-1);
}
