#!/usr/bin/perl
# Handling of POI Data:
# For now:
# Retrieve POI Data from Different Sources
# And import them into mySQL for use with gpsdrive
#
# $Log$
# Revision 1.1  2005/02/06 00:22:14  tweety
# poi.pl to mirror and fill POI Database with Names.
# This is a early Alpha version
#
# Revision 1.0  2005/01/12 22:08:13  tweety

my $VERSION ="poi.pl (c) Jörg Ostertag
Initial Version (Jan,2005) by Jörg Ostertag <joerg.ostertag\@rechengilde.de>
Version 1.17
";


BEGIN {
    unshift(@INC,"./");
};

use strict;
use warnings;

#use POI::KismetXml;
use Data::Dumper;

#use Date::Manip qw(ParseDate DateCalc UnixDate);
use File::Basename;
use File::Copy;
use File::Path;
use Getopt::Long;
use POI::Gps;
use HTTP::Request;
use IO::File;
use Pod::Usage;

use POI::DBFuncs;
use POI::Utils;

my ($man,$help);

my $CONFIG_DIR    = "$ENV{'HOME'}/.gpsdrive"; # Should we allow config of this?
my $CONFIG_FILE   = "$CONFIG_DIR/gpsdriverc";

my $do_census            = 0;
my $do_earthinfo_nga_mil = 0;
my $do_opengeodb         = 0;
my $do_mapsource_points  = 0; 
my $do_cameras           = 0;
my $do_all               = 0;
my $do_create_db         = 0;
our $db_user             = 'gast';
our $db_password         = 'gast';

# Set defaults and get options from command line
Getopt::Long::Configure('no_ignore_case');
GetOptions ( 
	     'census'              => \$do_census,
	     'earthinfo_nga_mil=s' => \$do_earthinfo_nga_mil,
	     'opengeodb'           => \$do_opengeodb,
	     'mapsource_points=s'  => \$do_mapsource_points,
	     'cameras'             => \$do_cameras,
	     'create-db'           => \$do_create_db,
	     'all'                 => \$do_all,
	     'debug'               => \$debug,      
	     'u=s'                 => \$db_user,
	     'p=s'                 => \$db_password,
	     'd'                   => \$debug,      
	     'v'                   => \$verbose,      
	     'debug_range=s'       => \$debug_range,      
	     'no-mirror'           => \$no_mirror,
	     'MAN'                 => \$man, 
	     'help|x'              => \$help, 
	     )
    or pod2usage(1);

if ( $do_all ) {
    $do_census = $do_earthinfo_nga_mil = $do_opengeodb 
	= $do_cameras   = 1;
}

pod2usage(1) if $help;
pod2usage(-verbose=>2) if $man;

sub debug($);            # {}


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

#############################################################################
my @countries = qw(aa ac ae af ag aj al am an ao ar as at au av 
		   ba bb bc bd be bf bg bh bk bl bm bn bo bp br bs bt bu bv bx by 
		   ca cb cd ce cf cg ch ci cj ck cm cn co cr cs ct cu cv cw cy
		   da dj do dr ec eg ei ek en er es et eu ez 
		   fg fi fj fk fm fo fp fr fs
		   ga gb gg gh gi gj  gk gl gm go gp gr gt gv gy gz 
		   ha hk hm ho hr hu
		   ic id im in io ip ir is it iv iz 
		   ja je jm jn jo ju ke kg kn kr ks kt ku kz
		   la le lg lh li lo ls lt lu ly 
		   ma mb mc md mf mg mh mi mk ml mn mo mp mr mt mu mv mx my mz 
		   nc ne nf ng nh ni nl nm no np nr ns nt nu nz 
		   os 
		   pa pc pe pf pg pk pl pm po pp ps pu
		   qa re rm ro 
		   rp rs rw 
		   sa sb sc se sf sg sh si sl sm sn so sp st su sv sw sx sy sz
		   td te th ti tk tl tn to tp ts tt tu tv tw tx tz 
		   uf ug uk up uv uy uz 
		   vc ve vi vm vt
		   wa we wf wi ws wz 
		   yi ym 
		   za zi);

my $name2country = {
    'afghanistan'	=> 'af',
    'albania'	=> 'al',
    'algeria'	=> 'ag',
    'andorra'	=> 'an',
    'angola'	=> 'ao',
    'anguilla'	=> 'av',
    'antigua and barbuda'	=> 'ac',
    'argentina'	=> 'ar',
    'armenia'	=> 'am',
    'aruba'	=> 'aa',
    'ashmore and cartier islands'	=> 'at',
    'australia'	=> 'as',
    'austria'	=> 'au',
    'azerbaijan'	=> 'aj',
    'bahamas, the'	=> 'bf',
    'bahrain'	=> 'ba',
    'bangladesh'	=> 'bg',
    'barbados'	=> 'bb',
    'bassas da india'	=> 'bs',
    'belarus'	=> 'bo',
    'belgium'	=> 'be',
    'belize'	=> 'bh',
    'benin'	=> 'bn',
    'bermuda'	=> 'bd',
    'bhutan'	=> 'bt',
    'bolivia'	=> 'bl',
    'bosnia and herzegovina'	=> 'bk',
    'botswana'	=> 'bc',
    'bouvet island'	=> 'bv',
    'brazil'	=> 'br',
    'british indian ocean territory'	=> 'io',
    'british virgin islands'	=> 'vi',
    'brunei'	=> 'bx',
    'bulgaria'	=> 'bu',
    'burkina faso'	=> 'uv',
    'burma'	=> 'bm',
    'burundi'	=> 'by',
    'cambodia'	=> 'cb',
    'cameroon'	=> 'cm',
    'canada'	=> 'ca',
    'cape verde'	=> 'cv',
    'cayman islands'	=> 'cj',
    'central african republic'	=> 'ct',
    'chad'	=> 'cd',
    'chile'	=> 'ci',
    'china'	=> 'ch',
    'christmas island'	=> 'kt',
    'clipperton island'	=> 'ip',
    'cocos (keeling) islands'	=> 'ck',
    'colombia'	=> 'co',
    'comoros'	=> 'cn',
    'congo'	=> 'cf',
    'congo, democratic republic of the'	=> 'cg',
    'cook islands'	=> 'cw',
    'coral sea islands'	=> 'cr',
    'costa rica'	=> 'cs',
    'croatia'	=> 'hr',
    'cuba'	=> 'cu',
    'cyprus'	=> 'cy',
    'czech republic'	=> 'ez',
    'cÔte d\'ivoire'	=> 'iv',
    'denmark'	=> 'da',
    'djibouti'	=> 'dj',
    'dominica'	=> 'do',
    'dominican republic'	=> 'dr',
    'east timor'	=> 'tt',
    'ecuador'	=> 'ec',
    'egypt'	=> 'eg',
    'el salvador'	=> 'es',
    'equatorial guinea'	=> 'ek',
    'eritrea'	=> 'er',
    'estonia'	=> 'en',
    'ethiopia'	=> 'et',
    'europa island'	=> 'eu',
    'falkland islands (islas malvinas)'	=> 'fk',
    'faroe islands'	=> 'fo',
    'fiji'	=> 'fj',
    'finland'	=> 'fi',
    'france'	=> 'fr',
    'french guiana'	=> 'fg',
    'french polynesia'	=> 'fp',
    'french southern and antarctic lands'	=> 'fs',
    'gabon'	=> 'gb',
    'gambia, the'	=> 'ga',
    'gaza strip'	=> 'gz',
    'georgia'	=> 'gg',
    'germany'	=> 'gm',
    'ghana'	=> 'gh',
    'gibraltar'	=> 'gi',
    'glorioso islands'	=> 'go',
    'greece'	=> 'gr',
    'greenland'	=> 'gl',
    'grenada'	=> 'gj',
    'guadeloupe'	=> 'gp',
    'guatemala'	=> 'gt',
    'guernsey'	=> 'gk',
    'guinea'	=> 'gv',
    'guinea-bissau'	=> 'pu',
    'guyana'	=> 'gy',
    'haiti'	=> 'ha',
    'heard island and mcdonald islands'	=> 'hm',
    'honduras'	=> 'ho',
    'hong kong'	=> 'hk',
    'hungary'	=> 'hu',
    'iceland'	=> 'ic',
    'india'	=> 'in',
    'indonesia'	=> 'id',
    'iran'	=> 'ir',
    'iraq'	=> 'iz',
    'ireland'	=> 'ei',
    'isle of man'	=> 'im',
    'israel'	=> 'is',
    'italy'	=> 'it',
    'jamaica'	=> 'jm',
    'jan mayen'	=> 'jn',
    'japan'	=> 'ja',
    'jersey'	=> 'je',
    'jordan'	=> 'jo',
    'juan de nova island'	=> 'ju',
    'kazakhstan'	=> 'kz',
    'kenya'	=> 'ke',
    'kiribati'	=> 'kr',
    'kuwait'	=> 'ku',
    'kyrgyzstan'	=> 'kg',
    'laos'	=> 'la',
    'latvia'	=> 'lg',
    'lebanon'	=> 'le',
    'lesotho'	=> 'lt',
    'liberia'	=> 'li',
    'libya'	=> 'ly',
    'liechtenstein'	=> 'ls',
    'lithuania'	=> 'lh',
    'luxembourg'	=> 'lu',
    'macau'	=> 'mc',
    'macedonia, the former yugoslav republic of'	=> 'mk',
    'madagascar'	=> 'ma',
    'malawi'	=> 'mi',
    'malaysia'	=> 'my',
    'maldives'	=> 'mv',
    'mali'	=> 'ml',
    'malta'	=> 'mt',
    'marshall islands'	=> 'rm',
    'martinique'	=> 'mb',
    'mauritania'	=> 'mr',
    'mauritius'	=> 'mp',
    'mayotte'	=> 'mf',
    'mexico'	=> 'mx',
    'micronesia, federated states of'	=> 'fm',
    'moldova'	=> 'md',
    'monaco'	=> 'mn',
    'mongolia'	=> 'mg',
    'montserrat'	=> 'mh',
    'morocco'	=> 'mo',
    'mozambique'	=> 'mz',
    'namibia'	=> 'wa',
    'nauru'	=> 'nr',
    'nepal'	=> 'np',
    'netherlands antilles'	=> 'nt',
    'netherlands'	=> 'nl',
    'new caledonia'	=> 'nc',
    'new zealand'	=> 'nz',
    'nicaragua'	=> 'nu',
    'niger'	=> 'ng',
    'nigeria'	=> 'ni',
    'niue'	=> 'ne',
    'no man\'s land'	=> 'nm',
    'norfolk island'	=> 'nf',
    'north korea'	=> 'kn',
    'norway'	=> 'no',
    'oceans'	=> 'os',
    'oman'	=> 'mu',
    'pakistan'	=> 'pk',
    'palau'	=> 'ps',
    'panama'	=> 'pm',
    'papua new guinea'	=> 'pp',
    'paracel islands'	=> 'pf',
    'paraguay'	=> 'pa',
    'peru'	=> 'pe',
    'philippines'	=> 'rp',
    'pitcairn islands'	=> 'pc',
    'poland'	=> 'pl',
    'portugal'	=> 'po',
    'qatar'	=> 'qa',
    'reunion'	=> 're',
    'romania'	=> 'ro',
    'russia'	=> 'rs',
    'rwanda'	=> 'rw',
    'saint helena'	=> 'sh',
    'saint kitts and nevis'	=> 'sc',
    'saint lucia'	=> 'st',
    'saint pierre and miquelon'	=> 'sb',
    'saint vincent and the grenadines'	=> 'vc',
    'samoa'	=> 'ws',
    'san marino'	=> 'sm',
    'sao tome and principe'	=> 'tp',
    'saudi arabia'	=> 'sa',
    'senegal'	=> 'sg',
    'serbia and montenegro'	=> 'yi',
    'seychelles'	=> 'se',
    'sierra leone'	=> 'sl',
    'singapore'	=> 'sn',
    'slovakia'	=> 'lo',
    'slovenia'	=> 'si',
    'solomon islands'	=> 'bp',
    'somalia'	=> 'so',
    'south africa'	=> 'sf',
    'south georgia and the south sandwich islands'	=> 'sx',
    'south korea'	=> 'ks',
    'spain'	=> 'sp',
    'spratly islands'	=> 'pg',
    'sri lanka'	=> 'ce',
    'sudan'	=> 'su',
    'suriname'	=> 'ns',
    'svalbard'	=> 'sv',
    'swaziland'	=> 'wz',
    'sweden'	=> 'sw',
    'switzerland'	=> 'sz',
    'syria'	=> 'sy',
    'taiwan'	=> 'tw',
    'tajikistan'	=> 'ti',
    'tanzania'	=> 'tz',
    'thailand'	=> 'th',
    'togo'	=> 'to',
    'tokelau'	=> 'tl',
    'tonga'	=> 'tn',
    'trinidad and tobago'	=> 'td',
    'tromelin island'	=> 'te',
    'tunisia'	=> 'ts',
    'turkey'	=> 'tu',
    'turkmenistan'	=> 'tx',
    'turks and caicos islands'	=> 'tk',
    'tuvalu'	=> 'tv',
    'uganda'	=> 'ug',
    'ukraine'	=> 'up',
    'undersea features'	=> 'uf',
    'united arab emirates'	=> 'ae',
    'united kingdom'	=> 'uk',
    'uruguay'	=> 'uy',
    'uzbekistan'	=> 'uz',
    'vanuatu'	=> 'nh',
    'vatican city'	=> 'vt',
    'venezuela'	=> 've',
    'vietnam'	=> 'vm',
    'wallis and futuna'	=> 'wf',
    'west bank'	=> 'we',
    'western sahara'	=> 'wi',
    'yemen'	=> 'ym',
    'zambia '	=> 'za',
    'zimbabwe'	=> 'zi',
};




#############################################################################
# Args: 
#    $filename : Filename to read 
# returns:
#    $waypoints : Hash of read Waypoints
#############################################################################
sub add_earthinfo_nga_mil_to_db($$){
    my $full_filename = shift;
    my $source = shift;

    print "Reading earthinfo_nga_mil ($full_filename) and writing to db\n";

    my $fh = IO::File->new("<$full_filename");
    $fh or die ("add_earthinfo_nga_mil_to_db: Cannot open $full_filename:$!\n");

    delete_all_from_source($source);
    my $source_id = POI::DBFuncs::source_name2id($source);

    unless ( $source_id ) {
	my $source_hash = {
	    'source.url'     => "http://earth-info.nga.mil/gns/html/",
	    'source.name'    => $source ,
	    'source.comment' => '' ,
	    'source.licence' => "Licensing GNS Data".
		"There are no licensing requirements or restrictions ".
		"in place for the use of  ".
		"the GNS data.\n".
		"\n".
		"Toponymic information is based on the Geographic Names Data Base, ".
		"containing official standard names approved by the ".
		"United States Board on Geographic Names and maintained ".
		"by the National Geospatial-Intelligence Agency. ".
		"More information is available at the Products and Services ".
		"link at www.nga.mil. The National Geospatial-Intelligence ".
		"Agency name, initials, and seal are protected by ".
		"10 United States Code Section §445."
	    };
	POI::DBFuncs::insert_hash("source", $source_hash);
	$source_id = POI::DBFuncs::source_name2id($source);
    }
    

    my @columns;
    @columns = qw( rc      ufi     uni     
		   poi.lat     poi.lon    
		   dms_lat dms_long        
		   utm     jog     fc      dsg     pc      cc1     
		   adm1    adm2    
		   dim     
		   cc2     nt      lc      
		   short_form      
		   generic 
		   sort_name full_name full_name_nd
		   mod_date
		   );
    my $lines_count_file =0;
    my $line = $fh->getline(); # Header entfernen
    while ( $line = $fh->getline() ) {
	$lines_count_file ++;
	print "$lines_count_file\r" if $verbose && ! ($lines_count_file % 100);
	$line =~ s/[\t\r\n\s]*$//g;
#	 print "line: '$line'\n";
	if ( $line =~ m/^$/ ) {
	} elsif ( $line =~ m/^\#/ ) {
	} else {
	    die "Spalten nicht definiert" unless @columns;
#	    print "-----------------------------------------------------------------\n";
#	    print "WP: $line\n";
	    my @values = split(/\t/,$line);
#	    print Dumper(\@values);
	    my $values;
	    for my $i ( 0 .. scalar @columns -1 ) {
#		next unless $columns[$i] =~ m/\./;
		$values->{$columns[$i]} = ($values[$i]||'');
##		print $columns[$i].": \t".($values[$i]||'-')."\n";
	    }


	    $values->{'poi.source_id'}=$source_id;
	    #$values->{'source.name'} = $source;
#	    print Dumper(\$values);

	    # A specific part of the name that could substitute for the full name.
	    $values->{'poi.name'}=$values->{'short_form'}||$values->{'full_name'};

	    print "Name longer than 80 chars: $values->{'poi.name'} \n"
		if length($values->{'poi.name'}) > 80 ;
	    
	    $values->{'poi.comment'}=$values->{'generic'};
	    # GENERIC
	    # The descriptive part of the full name (does not apply to populated 
	    # place names).


	    # SORT_NAME
	    # A form of the full name which allows for easy sorting of the name 
	    # into alpha-numeric sequence. It is comprised of the specific name, 
	    # generic name, and any articles or prepositions. 
	    # This field is all upper case with spaces, diacritics, and hyphens 
	    # removed and numbers are substituted with lower case alphabetic characters.

	    # FULL_NAME
	    # The full name is a complete name which identifies the named feature.  
	    # It is comprised of  the specific name, generic name, and any articles 
	    # or prepositions (refer to REGIONS.PDF for character mapping).
	    if ( $values->{'full_name'} eq $values->{'poi.name'} ) {
		$values->{'poi.comment'}='';
	    } else {
		$values->{'poi.comment'}=$values->{'full_name'};
	    }
	    
	    # FULL_NAME_ND
	    # Same as the full name but the diacritics and special characters are 
	    # substituted with Roman characters (refer to REGIONS.PDF for character 
	    # mapping).   ND = No Diacritics / Stripped Diacritics.


	    $values->{'poi.last_modified'}=$values->{'mod_date'};

	    {
		my $pc = $values->{'pc'};
		# Populated Place Classification.  
		# A graduated numerical scale denoting the relative importance 
		# of a populated place.  
		# The scale ranges from 1,  relatively high, to 5, relatively low.  
		# The scale could also include NULL (no value) as a value for 
		# populated places with unknown or undetermined classification.
		my $level_min = 0;
		my $level_max = 99;
		if ( defined($pc) && $pc ne '' ) { 
		    #print "pc : $pc \n";
		    if    ( $pc == 1 ) {   $level_min = 0;	$level_max = 99; }
		    elsif ( $pc == 2 ) {   $level_min = 10;	$level_max = 99; }
		    elsif ( $pc == 3 ) {   $level_min = 20;	$level_max = 99; }
		    elsif ( $pc == 4 ) {   $level_min = 30;	$level_max = 99; }
		    elsif ( $pc == 5 ) {   $level_min = 40;	$level_max = 99; }
		} else {
		    $level_min = 50;	$level_max = 99; 
		};
		$values->{'poi.level_min'} = $level_min;
		$values->{'poi.level_max'} = $level_max;
	    }


	    
	    {   # NT Name Type:
		# C = Conventional;
		# D = Not verified;
		# N = Native;
		# V = Variant or alternate.
		my $nt = $values->{'nt'};
	    }

	    { # decide which symbol
		my $fc = $values->{'fc'};
		my $symbol = "City";
		#FC
		#Feature Classification:
		if ( $fc eq "A" ) { $symbol = "Administrative region" }
		elsif ( $fc eq "P " ) { $symbol = "Populated place" }
		elsif ( $fc eq "V " ) { $symbol = "Vegetation" }
		elsif ( $fc eq "L " ) { $symbol = "Locality or area" }
		elsif ( $fc eq "U " ) { $symbol = "Undersea" }
		elsif ( $fc eq "R " ) { $symbol = "Streets, highways, roads, or railroad" }
		elsif ( $fc eq "T " ) { $symbol = "Hypsographic" }
		elsif ( $fc eq "H " ) { $symbol = "Hydrographic" }
		elsif ( $fc eq "S " ) { $symbol = "Spot feature." }
		$values->{'poi.symbol'} = $symbol;
	    }

	    { # Dimension.  Usually used to display elevation or population data.
		my $proximity = $values->{'dim'} ;
		$proximity ||= 1   if  $values->{'fc'} eq 'R'; # Roads
		$proximity ||= 800 if  $values->{'fc'} eq 'P'; # Roads
		$proximity ||= ' 100m';
		$values->{'poi.proximity'} = $proximity;
	    }

	    poi_add($values);
	}
    }
    print "$lines_count_file read\n" if $verbose;
}


#############################################################################
# Args: 
#    $filename : Filename to read 
# returns:
#    $waypoints : Hash of read Waypoints
#############################################################################
sub read_open_geo_db($){
    my $full_filename = shift;

    my $waypoints={};
    print "Reading open geo db: $full_filename\n";
    my $fh = IO::File->new("<$full_filename");
    $fh or die ("read_open_geo_db: Cannot open $full_filename:$!\n");

    my $source = "open geo db";
    my $source_id = source_name2id($source);

    my @columns;
    @columns = qw( primarykey 
		   address.staate address.bundesland address.regierungsbezirk 
		   address.landkreis address.verwaltungszusammenschluss
		   address.ort   address.ortsteil address.gemeindeteil
		   poi.lat poi.lon
		   poi.autokennzeichen
		   address.plz);
    my $lines_count_file =0;
    while ( my $line = $fh->getline() ) {
	$lines_count_file ++;
	$line =~ s/[\t\r\n\s]*$//g;;
#	 print "line: '$line'\n";
	if ( $line =~ m/^$/ ) {
	} elsif ( $line =~ m/^\#/ ) {
	} else {
	    die "Spalten nicht definiert" unless @columns;
	    
#	    print "WP: $line\n";
	    my @values = split(/\s*\;\s*/,$line);
	    #print Dumper(\@values);
	    my $values;
	    for my $i ( 0 .. scalar @columns -1 ) {
		$values->{$columns[$i]} = $values[$i];
		$values->{'poi.comment'} .= ",$values[$i]";
	    }

	    ############################################
	    # Set Default Proximity 
	    $values->{'poi.proximity'} ||= "1000 m";

	    $values->{'poi.symbol'} ||= "City";


	    for my $plz ( split(',',$values->{'address.plz'})) {
		#	    print Dumper($values);
		my $wp_name = $plz;
		#	    $wp_name .= "_$values->{'poi.primarykey'}";
		#	    $wp_name .= "_$values->{'address.staat'}";
		$wp_name .= "_$values->{'address.bundesland'}";
		#	    $wp_name .= "_$values->{'address.regierungsbezirk'}";
		#	    $wp_name .= "_$values->{'address.landkreis'}";
		#	    $wp_name .= "_$values->{'address.verwaltungszusammenschluss'}";
		$wp_name .= "_$values->{'address.ort'}";
		$wp_name .= "_$values->{'address.ortsteil'}";
		$wp_name .= "_$values->{'address.gemeindeteil'}";
		$values->{'poi.name'}=$wp_name;
		if (  $plz =~ m/000$/ ) {
		    print "$plz $values->{'address.ort'} $values->{'address.ortsteil'} $values->{'address.gemeindeteil'}\n";
		    $values->{'poi.level'}=0;
		} elsif (  $plz =~ m/00$/ ) {
		    $values->{'poi.level'}=1;
		} elsif (  $plz =~ m/0$/ ) {
		    $values->{'poi.level'}=2;
		} else {
		    $values->{'poi.level'}=3;
		}
		
		unless ( defined($values->{'poi.lat'}) ) {
		    print "Undefined lat".Dumper(\$values);
		}
		unless ( defined($values->{'poi.lon'}) ) {
		    print "Undefined lon".Dumper(\$values);
		}

		$values->{'poi.source_id'}=$source_id;
		
		correct_lat_lon($values);
		$waypoints->{$wp_name} = $values;
		#print "Values:".Dumper(\$values);
	    }
	}
    }
    return $waypoints;
}

#############################################################################
# Args: 
#    $filename : Filename to read 
# RETURNS:
#    $waypoints : Has of read Waypoints
#############################################################################
sub read_mapsource_waypoints($){
    my $full_filename = shift;

    my $waypoints={};

    print "Reading: $full_filename\n";

    my $fh = IO::File->new("<$full_filename");
    $fh or die ("read_mapsource_waypoints: Cannot open $full_filename:$!\n");
    my @columns;

    my $lines_count_file =0;
    while ( my $line = $fh->getline() ) {
	$lines_count_file ++;
	$line =~ s/[\t\r\n\s]*$//g;;
	# print "line: '$line'\n";
	if ($line =~ m/^Grid\s+Breite\/LÃÂ¤nge hddd\.dddddÂÂ°/ ) {
	} elsif ( $line =~ m/^$/ ) {
	} elsif ( $line =~ m/^Datum\s+WGS 84/ ) {
	} elsif ( $line =~ m/^Header/ ) {
	    @columns = split(/\s+/,$line);
	    #print Dumper(\@columns);
	} elsif ( $line =~ m/^Waypoint\s+/ ) {
	    die "Spalten nicht definiert" unless @columns;
	    
#	    print "WP: $line\n";
	    my @values = split(/\t/,$line);
	    #print Dumper(\@values);
	    my $values;
	    for my $i ( 0 .. scalar @columns -1 ) {
		$values->{$columns[$i]} = $values[$i];
	    }

	    ############################################
	    ############################################
	    # Set Default Proximity to 800m
	    $values->{'Proximity'} ||= "800 m";


	    $values->{Symbol} ||= "";

	    ( $values->{lat},$values->{lon}) = split(/\s+/,$values->{'Position'});

	    ############################################
	    correct_lat_lon($values);

	    #print Dumper($values) if defined $values->{'Proximity'};
	    my $wp_name = $values->{'Name'};
	    $waypoints->{$wp_name} = $values;
	} else {
	    print "Unknown Line: '$line'\n";
	}
	
    }
    return $waypoints;
}

#############################################################################
# Args: 
#    $filename : Filename to read 
# RETURNS:
#    $waypoints : Hash of read Waypoints
#############################################################################
sub read_speedstrap_wp($$){
    my $full_filename = shift;
    my $type = shift; # Type of Speedtrap / Photo

    print "Reading: $full_filename\n";

    my $fh = IO::File->new("<$full_filename");
    $fh or die ("read_speedtrap_wp: Cannot open $full_filename:$!\n");

    my $waypoints;
    my $lines_count_file =0;
    while ( my $line = $fh->getline() ) {
	$lines_count_file ++;
	$line =~ s/[\t\r\n\s]*$//g;;
	# print "line: '$line'\n";
	if ($line =~ m/^Longitude,Latitude,Name/ ) {
	} elsif ( $line =~ m/^$/ ) {
	} elsif ( $line =~ m/^\-?\d+/ ) {
	    my @values = split(/\,/,$line);
	    #print Dumper(\@values);
	    my $values;
	    $values->{lat} = $values[0];
	    $values->{lon} = $values[1];
	    $values->{Name} = $values[2];
	    $values->{Name} =~ s/^\"//;
	    $values->{Name} =~ s/\"$//;

	    ############################################
	    # Set Default Proximity for speedtraps to 500m
	    $values->{'Proximity'} ||= "500 m";

	    $values->{Symbol} = "SPEEDTRAP-$type";

	    ############################################
	    correct_lat_lon($values);

	    #print Dumper($values) if defined $values->{'Proximity'};
	    my $wp_name = $values->{'Name'};
	    $waypoints->{$wp_name} = $values;
	} else {
	    print "Unknown Line: '$line'\n";
	}
	
    }
    return $waypoints;
}

#############################################################################
# Write the Waypoints in gpsdrive Format
#############################################################################
sub write_gpsdrive_waypoints($$){
    my $waypoints = shift;
    my $full_filename= shift;
    print "Writing gpsdrive waypoints to '$full_filename'\n";
    my $fo = IO::File->new(">$full_filename");
    $fo or die ("write_gpsdrive_waypoints: Cannot open $full_filename:$!\n");

    for my $wp_name ( sort keys  %{$waypoints} ) {
	my $values = $waypoints->{$wp_name};

	unless ( defined($values->{'poi.lat'}) ) {
	    print "Error undefined poi.lat: ".Dumper(\$values);
	}
	unless ( defined($values->{'poi.lon'}) ) {
	    print "Error undefined poi.lon: ".Dumper(\$values);
	}

	$wp_name =~ s/ÃÂ¤/ae/g;
	$wp_name =~ s/ÃÂ¶/oe/g;
	$wp_name =~ s/ÃÂ¼/ue/g;
	$wp_name =~ s/ÃÂ¤/Ae/g;
	$wp_name =~ s/Ã–/Oe/g;
	$wp_name =~ s/Ãœ/Ue/g;
	$wp_name =~ s/ÃŸ/sss/g;

	correct_lat_lon($values);

	for my $t (qw(Wlan Action Sqlnr Proximity) ) {
	    unless ( defined ( $values->{$t})) {
		$values->{$t} = 0;
	    }
	}

	$values->{Symbol} ||= "-";

	print $fo $wp_name;
	print $fo "\t".$values->{'poi.lat'};
	print $fo "\t".$values->{'poi.lon'};
	print $fo "\t".($values->{'poi.Symbol'}||'');
	print $fo "\t".($values->{'poi.Wlan'}||'');
	print $fo "\t".$values->{'Action'};
	print $fo "\t".$values->{'Sqlnr'};
	my $proximity = $values->{'Proximity'};
	$proximity =~ s/\s*m$//;
	print $fo "\t".$proximity;
	print $fo "\n";
    }
    $fo->close()
	or die ("write_gpsdrive_waypoints: Cannot close $full_filename:$!\n");
}


########################################################################################
########################################################################################
########################################################################################
#
#                     Main
#
########################################################################################
########################################################################################
########################################################################################


if ( $do_create_db ) {
    POI::DBFuncs::create_db();
};



########################################################################################
# Get and Unpack Census Data
# http://www.census.gov/geo/cob/bdy/
########################################################################################
if ( $do_census ) {
    print "=============================================================================\n";
    print "=============================================================================\n";
    print "=============================================================================\n";
    print "Census Download Not working yet.\n";
    print "Skipping \n";
    print "=============================================================================\n";
    print "=============================================================================\n";
    print "=============================================================================\n";
}
if ( 0 ) {
    my $CENSUS_DIR = "$CONFIG_DIR/MIRROR/CENSUS";

    unless ( -d $CENSUS_DIR ) {
	print "Creating Directory $CENSUS_DIR\n";
	mkpath $CENSUS_DIR
	    or die "Cannot create Directory $CENSUS_DIR:$!\n";
    }

    `wget --mirror --directory-prefix= --no-host-directories --include-directories=geo/cob/bdy --force-directories --level=2 http://www.census.gov/geo/cob/bdy/ --accept=zip,html -nv -D$CENSUS_DIR`;

    # download
    for my $state ( qw(  aia/     an/      bg/      cc/      cd/      co/     
			 cs/      dv/      econ/    ir/      ma/      mcd/    
			 na/      ne/      ou/      pl/      pu/      rg/     
			 sb/      se/      sl/      sn/      ss/      st/     
			 su/      tb/      tr/      ts/      tt/      tz/     
			 ua/      vt/      zt/     
			 90_data/
			 scripts/
			 )) {
#	for my $type ( qw( 00ascii/	     00e00/	     00shp/   ) ) {
	my $mirror = mirror_file("http://www.census.gov/geo/cob/bdy/".
				 "$state/${state}00ascii/${state}99_d00ascii.zip",
				 "$CENSUS_DIR/${state}99_d00ascii.zip");
	
	print "Mirror: $mirror\n";
	if ( $mirror ) {
	    # Unpack it 
	    `(cd $CENSUS_DIR/; unzip -o xy.zip)`;

	    for my $file_name ( glob("$CENSUS_DIR/pocketgps_*.csv") ) {
		my ( $type ) = ($file_name =~ m/pocketgps_(.*)\.csv/);
		my $out_file_name = "/home/gpsdrive/way_pocketgps_$type.txt";
		my $waypoints = read_speedstrap_wp($file_name,$type);
		write_gpsdrive_waypoints($waypoints,$out_file_name);
		write_mapsource_waypoints($waypoints,"way_pocketgps_$type.txt");
	    }
	}
    }	
}


########################################################################################
# Get and Unpack POCKETGPS_DIR
# http://www.pocketgpspoi.com
########################################################################################
if ( $do_cameras ) {
    print "=============================================================================\n";
    print "=============================================================================\n";
    print "=============================================================================\n";
    print "Pocketgps Not working yet.\n";
    print "Skipping \n";
    print "=============================================================================\n";
    print "=============================================================================\n";
    print "=============================================================================\n";
}
if ( 0 ) {
    my $POCKETGPS_DIR = "$CONFIG_DIR/MIRROR/POCKETGPS";

    unless ( -d $POCKETGPS_DIR ) {
	print "Creating Directory $POCKETGPS_DIR\n";
	mkpath $POCKETGPS_DIR
	    or die "Cannot create Directory $POCKETGPS_DIR:$!\n";
    }

    # download
    my $mirror = mirror_file("http://www.pocketgpspoi.com/downloads/pocketgps_uk_sc.zip",
			 "$POCKETGPS_DIR/pocketgps_uk_sc.zip");

    print "Mirror: $mirror\n";

    if ( $mirror ) {
	# Unpack it 
	`(cd $POCKETGPS_DIR/; unzip -o pocketgps_uk_sc.zip)`;

	for my $file_name ( glob("$POCKETGPS_DIR/pocketgps_*.csv") ) {
	    my ( $type ) = ($file_name =~ m/pocketgps_(.*)\.csv/);
	    my $out_file_name = "/home/gpsdrive/way_pocketgps_$type.txt";
	    my $waypoints = read_speedstrap_wp($file_name,$type);
	    write_gpsdrive_waypoints($waypoints,$out_file_name);
	    write_mapsource_waypoints($waypoints,"way_pocketgps_$type.txt");
	}
    }
}


#############################################################################
# Convert MapSource Waypoints to gpsdrive POI
#############################################################################
if ( $do_mapsource_points ) {
    my $waypoints = read_mapsource_waypoints($do_mapsource_points);
    db_add_waypoints($waypoints);
}


########################################################################################
# Get and Unpack earth-info.nga.mil
# http://earth-info.nga.mil/gns/html/cntyfile/gm.zip
########################################################################################
if ( $do_earthinfo_nga_mil ) {
    my $earthinfo_dir="$CONFIG_DIR/MIRROR/earthinfo";

    unless ( -d $earthinfo_dir ) {
	print "Creating Directory $earthinfo_dir\n";
	mkpath $earthinfo_dir
	    or die "Cannot create Directory $earthinfo_dir:$!\n";
    }

    my @do_countries;
    my $country;
    if ($do_earthinfo_nga_mil eq "??" ) {
	print "Available counties:\n\n";
	print join("\n	",map { "$_ ($name2country->{$_})" } sort keys %{$name2country} );
	print "\n";
	print "\n";
	print "See http://earth-info.nga.mil/gns/html/cntry_files.html for more Information\n";
	print "\n";
    } elsif ( $do_earthinfo_nga_mil =~  m/^\D/ ) {
	for $country ( split(",",$do_earthinfo_nga_mil) ) {
	    if ( $name2country->{$country}  ) {
		push ( @do_countries , $name2country->{$country} );
	    } else {
		if ( grep { $_ eq $country } @countries ) {
		    push ( @do_countries , $country );
		} else {
		    print "Country $country not valid\n";
		    print "List of valid countries:\n";
		    print join(",",@countries);
		    print "\n";
		    print "Use:\npoi.pl -earthinfo_nga_mil=??\n";
		    print "For detailed list\n;"
		    
		}
	    }
	}
    } else {
	@do_countries =  @countries;
    }
    
    for $country ( @do_countries ) {
	# download
	# http://earth-info.nga.mil/gns/html/cntyfile/gm.zip # Germany
	my $mirror = mirror_file("http://earth-info.nga.mil/gns/html/cntyfile/".
				 "$country.zip"
				 ,"$earthinfo_dir/geonames_$country.zip");
	
	# print "Mirror: $mirror\n";
	if ( (!-s "$earthinfo_dir/$country.txt") ||
	     file_newer("$earthinfo_dir/geonames_$country.zip",
			"$earthinfo_dir/$country.txt") ) {
	    print "Unpacking geonames_$country.zip\n";
	    `(cd $earthinfo_dir/; unzip -o geonames_$country.zip)`;
	} else {
	    print "unpack: $country.txt up to date\n" unless $verbose;
	}
	
	add_earthinfo_nga_mil_to_db("$earthinfo_dir/$country.txt","earth-info.nga.mil $country");
    }
    
}

########################################################################################
# Get and Unpack opengeodb
# http://www.opengeodb.de/download/
########################################################################################
    if ( $do_opengeodb ) {
	my $opengeodb_dir="$CONFIG_DIR/MIRROR/opengeodb";

    unless ( -d $opengeodb_dir ) {
	print "Creating Directory $opengeodb_dir\n";
	mkpath $opengeodb_dir
	    or die "Cannot create Directory $opengeodb_dir:$!\n";
    }

    # download
    my $mirror = mirror_file("http://ovh.dl.sourceforge.net/".
			     "sourceforge/geoclassphp/opengeodb-0.1.3-txt.tar.gz",
			     "$opengeodb_dir/opengeodb-0.1.3-txt.tar.gz");

    print "Mirror: $mirror\n";

    # Unpack it 
    `(cd $opengeodb_dir/; tar -xvzf opengeodb-0.1.3-txt.tar.gz)`;

    for my $file_name ( glob("$opengeodb_dir/opengeodb*.txt") ) {
	my $out_file_name = "$CONFIG_DIR/way_opengeodb.txt";
	my $waypoints = read_open_geo_db($file_name);
	write_gpsdrive_waypoints($waypoints,$out_file_name);
	db_add_waypoints($waypoints);
    }

}


__END__

=head1 NAME

B<convert_wp> Version 0.000001

=head1 DESCRIPTION

B<poi.pl> is a program to download and convert waypoints 
for use with the new gpsdrive POI Database. 
You need to have mySQL Support.

This Programm is completely experimental, but some Data 
can already be retrieved with it.

So: Have Fun, improve it and send me fixes :-))

=head1 SYNOPSIS

B<Common usages:>

poi.pl [-d] [-v] [-h] [-earthinfo_nga_mil>]

=head1 OPTIONS

=over 8

=item B<-earthinfo_nga_mil=xx[,yy][,zz]...>

Still experimental, but works partly.

Download Country File from
 http://earth-info.nga.mil/gns/html/

where xx is one or more countries. 
 poi.pl -earthinfo_nga_mil=??
Gives you a complete list of allowed county tupels.

For more info on Countries have a look at
 http://earth-info.nga.mil/gns/html/cntry_files.html

=item B<-opengeodb>

Experimental

=item B<-mapsource_points='Filename'>

Experimental

=item B<-cameras>

Experimental

=item B<-all>

Experimental

=item B<--create-db>

Try creating the tables inside the geodata database

=item B<--db_user>

username to connect to mySQL database

=item B<--db_password>

password for user to connect to mySQL database

=item B<--no-mirror>

do not try mirroring the files from the original Server
