package POI::NGA;

use strict;
use warnings;

use IO::File;
use POI::DBFuncs;
use POI::Utils;

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
    'cÃte d\'ivoire'	=> 'iv',
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
		"10 United States Code Section Â§445."
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

# *****************************************************************************
sub import_Data($){
    my $what = shift;
    my $earthinfo_dir="$main::CONFIG_DIR/MIRROR/earthinfo";
    
    unless ( -d $earthinfo_dir ) {
	print "Creating Directory $earthinfo_dir\n";
	mkpath $earthinfo_dir
	    or die "Cannot create Directory $earthinfo_dir:$!\n";
    }
    
    my @do_countries;
    my $country;
    if ($what eq "??" ) {
	print "Available counties:\n\n";
	print join("\n	",map { "$_ ($name2country->{$_})" } sort keys %{$name2country} );
	print "\n";
	print "\n";
	print "See http://earth-info.nga.mil/gns/html/cntry_files.html for more Information\n";
	print "\n";
    } elsif ( $what =~  m/^\D/ ) {
	for $country ( split(",",$what) ) {
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

1;
