package Geo::Gpsdrive::getstreet;
#noch nicht alle variablen angepasst
use LWP::UserAgent;
use WWW::Mechanize;
use Text::Query;
use Getopt::Long;
use Pod::Usage;
use DBI;
use strict;

sub streets(){
    my $street =$main::street;
    my $plz = $main::plz;
    my $ort = $main::ort;
    my $mysql =$main::sql;
    my $country;
    my $help;
    my $version;
    my $area;		# dont work at the moment not changed since moved form .pl to .pm
    my $scale;		# dont work
    my $name;
    my $count=0;
    my $type="STREET";
    my $comment="";
    my $file = $main::file;
    my $xx=0;
    my $VER="getstreet (c) SHB\nInitial Version (Mar,2005) by SHB\n";
#	GetOptions( 	'street=s'	=> 	\$street,
#			's=s'		=>	\$street,
#			'plz=s'		=>	\$plz,
#			'p=s'		=>	\$plz,
#			'ort=s'		=>	\$ort,
#			'o=s'		=>	\$ort,
#			'nname=s'	=>	\$name,
#			'n=s'		=>	\$name,
#			'country=s'	=>	\$country,
#			'c=s'		=>	\$country,
#			'f=s'		=>	\$file,
#			'file=s'	=>	\$file,
#			'version'	=>	\$version,
#			'v'		=>	\$version,
#			'scale=s'	=>	\$scale,
#			'a=s'		=>	\$area,
#			'type=s'	=>	\$type,
#			't=s'		=>	\$type,
#			'sql'		=>	\$mysql,
#			'comment=s'	=>	\$comment,
#			'help'		=>	\$help,
#			'h'			=>	\$help);

#pod2usages();

    if($help){
	print "\n\nHelp\n\n";
	
	print "Usage:\n";
	print "Usage getstreet.pl\n";
	print "-p, --plz\t zipcode\n";
	print "-o, --ort\t country\n";
	print "-s, --street \t\t\"name of street\"\n";
	print "-f, --file\t\t name of a file instat a streetname";
	print "\n\n";
	print "optional parameters:\n";
	print "-a Umkreis --scale scale\n";
	print "-t, --type\ttype \n";
	print "--sql insert query in the mysql database\n";
	print "optional sql parameters:\n";
	print "\t --comment \t insert a comment into the database";
	print "\n\n";
	print "-h, --help \tshow this display and exit the programm\n";
	print "-v, --version show the version\n ";
	print "At the moment only germany is supportet\n\n";
	print "note these script is beta\n\n";
	$count++;
    }
    if($country eq "help"){
    	#needed for other country than germany, not working at the moment
	print "List of countrys:\n\n";
	print "Australien : 22\n";
	print "Belgien : 1\n";
	print "Brasilien : 23\n";
	print "Dänemark : 3\n";
	print "Deutschland : 5\n";
	print "Finnland (Helsinki) : 16\n";
	print "Frankreich : 4\n";
	print "Griechenland (Athen) : 24\n";
	print "Großbritannien : 11\n";
	print "Italien : 6\n";
	print "Kanada : 2\n";
	print "Luxemburg : 7\n";
	print "Niederlande : 8\n";
	print "Norwegen : 17\n";
	print "Österreich : 0\n";
	print "Portugal : 18\n";
	print "Schweden : 19\n";
	print "Schweiz : 10\n";
	print "Spanien : 9\n";
	print "Vereinigte Staaten : 12\n";
	print "\n\n";
	$count++;
    }
    if($version){ 
	print "$VER\n";
	$count++;
    }
    if($street && $ort){
	my $url="http://mappoint.msn.de";
	my $val = "MapForm:POIText";
	my $v_ort = $ort;
	my $v_plz = $plz;
	my $v_str = $street;
	$v_str =~ s/ü/u/g;
	$v_str =~ s/ö/o/g;
	$v_str =~ s/ä/a/g;
	$v_str =~ s/Ü/U/g;
	$v_str =~ s/Ö/O/g;
	$v_str =~ s/Ä/Ä/g;
	$v_str =~ s/ß/ss/g;
	my $ort = "FndControl:CityText";
	my $plz = "FndControl:ZipText";
	my $str = "FndControl:StreetText";
	my $cou = "FndControl:ARegionSelect";
	my $form = "FindForm";
	my $mech= WWW::Mechanize->new();
	$mech->agent_alias( 'Windows IE 6' );
	$mech->get($url);
	$mech->form_name($form);
	##
	$mech->field($ort,$v_ort);
	$mech->field($plz,$v_plz);
	$mech->field($str,$v_str);
	##
	# $mech->set_fields( 
	# $ort => $v_ort,
	# $plz => $v_plz,
	# $str => $v_str);		#this will all filds faster, if it woud work ;(
	# if($countery){
	#    $mech->field($cou,$country);
	# }
	$mech->submit();
	if($mech->form_name('MCForm')){
	    $mech->form_name('MCForm');
	    my $out= $mech->value($val);
	    my @split=split(/\|/,$out);
	    my @cord=split("%2c",$split[1]);
	    my @addr=split("%2c",$split[3]);
	    my $str=$addr[0];
	    my @ort_plz=split(/\+/,$addr[1]);
	    $ort=$ort_plz[2];
	    $plz=$ort_plz[1];
	    $str =~ s/%c3%9f/ss/g;
	    $str =~ s/%c3%bc/ue/g;
	    $str =~ s/\+/\_/g;
	    if($str eq ""){
		print "$str\n";
		print "$out\n";
		print "Strasse nicht gefunden\n";
	    }else{
		print STDERR "$out\n";
		if($name ne ""){
		    $str=$name;
		}
		my $ausgabe= "$str\t$cord[0]\t$cord[1]\t$type\n";
		my $datei = "$ENV{'HOME'}/.gpsdrive/way.txt";
		if($mysql){
		    my $dbh = DBI->connect( 'dbi:mysql:geoinfo', $main::db_user, $main::db_password ) || die "Kann keine Verbindung zum MySQL-Server aufbauen: $DBI::errstr\n";
		    my $query ="insert into waypoints(name,lat,lon,type,comment) values('$str','$cord[0]','$cord[1]','$type','$comment')";
		    $dbh->prepare($query)->execute;
		}else{
		    open(FILE,">>$datei")||die "Error: File not found\n";
		}
		print FILE $ausgabe;
		close(FILE);
		if($area && $scale){
		    system("gpsfetchmap.pl -w $str -a $area --scale $scale");
		}
	    }
	}elsif($mech->form_name('FindForm')){
	   #this will happen, if you get a multiple choice answer
	    print $mech->value('FndControl:AmbiguousSelect');
	    print "\n";
	    print "Multiplie choice \nstill in development\n";
	   
	}
	$count++;
    }
    if($ort && $file){			#this is for reading a file instat of a singel streetname, not tested since moved from .pl to .pm
    					#an other problem is in big city with more than one zipcode, sometimes the streets where not found, because the zipcode is missing und you get a multiple choice as answer
	open(FILE, "<$file");
	our @street;
	while( <FILE>){
	    push(@street,$_);
	}
	close(FILE);
	our $x=0;
	for(@street){
	    print "Try to get @street[$x]";
	    my $url="http://mappoint.msn.de";
	    my $val = "MapForm:POIText";
	    my $v_ort = $ort;
	    my $v_plz = $plz;
	    my $v_str = @street[$x];
	    $v_str =~ s/ü/u/g;
	    $v_str =~ s/ö/o/g;
	    $v_str =~ s/ä/a/g;
	    $v_str =~ s/Ü/U/g;
	    $v_str =~ s/Ö/O/g;
	    $v_str =~ s/Ä/Ä/g;
	    $v_str =~ s/ß/ss/g;
	    my $ort = "FndControl:CityText";
	    my $plz = "FndControl:ZipText";
	    my $str = "FndControl:StreetText";
	    my $cou = "FndControl:ARegionSelect";
	    my $form = "FindForm";
	    my $mech= WWW::Mechanize->new();
	    $mech->agent_alias( 'Windows IE 6' );
	    $mech->get($url);
	    $mech->form_name($form);
	    ##
	    $mech->field($ort,$v_ort);
	    $mech->field($plz,$v_plz);
	    $mech->field($str,$v_str);
	    ##
	    #$mech->set_fields(
	    #						$ort => $v_ort,
#							$plz => $v_plz,
#							$str => $v_str)
	    #if($countery){
	    #	$mech->field($cou,$country);
	    #}
	    $mech->submit();
	    if($mech->form_name('MCForm')){
		$mech->form_name('MCForm');
		my $out= $mech->value($val);
		my @split=split(/\|/,$out);
		my @cord=split("%2c",$split[1]);
		my @addr=split("%2c",$split[3]);
		my $str=$addr[0];
		my @ort_plz=split(/\+/,$addr[1]);
		$ort=$ort_plz[2];
		$plz=$ort_plz[1];
		$str =~ s/%c3%9f/ss/g;
		$str =~ s/%c3%bc/ue/g;
		$str =~ s/\+/\_/g;
		if($str eq ""){
		    print "$str\n";
		    print "$out\n";
		    print "Strasse nicht gefunden\n";
		}else{
		    print STDERR "$out\n";
		    if($name ne ""){
			$str=$name;
		    }
		    my $ausgabe= "$str\t$cord[0]\t$cord[1]\t$type\n";
		    my $datei = "$ENV{'HOME'}/.gpsdrive/way.txt";
		    if($mysql){
			my $dbh = DBI->connect( 'dbi:mysql:geoinfo', $main::db_user, $main::db_password ) || die "Kann keine Verbindung zum MySQL-Server aufbauen: $DBI::errstr\n";
			my $query ="insert into waypoints(name,lat,lon,type,comment) values('$str','$cord[0]','$cord[1]','$type','$comment')";
			$dbh->prepare($query)->execute;
		    }else{
			open(FILE,">>$datei")||die "Error: File not found\n";
		    }
		    print FILE $ausgabe;
		    close(FILE);
		    if($area && $scale){
			system("gpsfetchmap.pl -w $str -a $area --scale $scale");
		    }
		}
	    }elsif($mech->form_name('FindForm')){
		print $mech->value('FndControl:AmbiguousSelect');
		#		print "\n";
		#		print "Multiplie choice \nstill in development\n";
		open(FILE_OUT, ">>errors");
		$xx++;
		print FILE_OUT "@street[$x]" ;
		close(FILE_OUT);
		print "$xx errors\n";
	    }
	    $count++;
	    $x++;
	}
    }
    if($count==0){
	#print 'Usage getstreet.pl <-p PLZ -o ORT> -s "Straßen"> [<-a Umkreis --scale Scala>] ';
	print "Use getstreet.pl -h for help\n";
	print "\n";
    }
    return 1;
}
1;
