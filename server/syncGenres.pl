
#=============================================================================================================================
# APPLICATION NAME: syncGenres.pl
#
#
# DESCRIPTION: Automatically updates the fm radio station database with the latest genre data available from streema.com
#
# AUTHORS: Kyle Nolan, Marcel Marki
#
# DATE: 2014.12.13
#
# CONTRIBUTING AUTHORS:
#
# REVISIONS: 
# 
# 
#============================================================================================================================

#load modules
use Term::ProgressBar;

use DBI;
use Math::Round;
use Math::Trig;
#use warnings;
use Time::HiRes;
use LWP::UserAgent;
use POSIX qw(ceil floor);
no warnings 'utf8';
use Scalar::Util qw(looks_like_number);

# ----- GLOBAL VARS ----- #
$baseUrl = "http://streema.com/radios/country/United_States/state/";
$cityBaseUrl = "http://streema.com/radios/";
$saveFileBase = "genres";
$saveFile = "";
$currentState = "";
$debug = 0;

#%stateHash = split /\s\s+/,
#      q(AK  Alaska           LA  Louisiana);

%stateHash = split /\s\s+/,
      q(AK  Alaska           LA  Louisiana        OH  Ohio
        AL  Alabama          MA  Massachusetts    OK  Oklahoma
        AR  Arkansas         MD  Maryland         OR  Oregon
        AZ  Arizona          ME  Maine            PA  Pennsylvania
        CA  California       MI  Michigan         RI  Rhode_Island
        CO  Colorado         MN  Minnesota        SC  South_Carolina
        CT  Connecticut      MO  Missouri         SD  South_Dakota
        DE  Delaware         MS  Mississippi      TN  Tennessee
        FL  Florida          MT  Montana          TX  Texas
        GA  Georgia          NC  North_Carolina   UT  Utah
        HI  Hawaii           ND  North_Dakota     VA  Virginia
        IA  Iowa             NE  Nebraska         VT  Vermont
        ID  Idaho            NH  New_Hampshire    WA  Washington
        IL  Illinois         NJ  New_Jersey       WI  Wisconsin
        IN  Indiana          NM  New_Mexico       WV  West_Virginia
        KS  Kansas           NV  Nevada           WY  Wyoming
        KY  Kentucky         NY  New_York		  DC  DC);

$startTime = time();
$counter = 0;

print "\n_____________________________________\n\nBuilding Genre Index By State\n\n\n";

#go through state by state, because that's the way streema.com is organized
#build a master list of callsigns, cities, frequencies, and genres
#@calls;
$unknownGenreCounter = 0;
$stationCounter = 0;
@citiesDb;
@freqs;
@states;
@genres;
@genreNickNames = ('country','jazz','classical','club','hip-hop','rnb','rock','oldies','christian','alternative','pop','talk','foreign');

@genreKeyWords = (
	['folk',' blue','blue-grass','country','country','country','country','country','country'],
	['jazz','ragtime','swing','funk','jazz','jazz','jazz','jazz','jazz'],
	['classical','chamber','orchestra','symphony','instrumental','concert','classical','classical','classical'],
	['dance','club','electr','disco','dubstep','step','dance','trance','house'],
	['hip','urban','hip-hop','rap','dj','hip','hip','hip','hip'],
	['rnb','r&b','soul','blues','rnb','rnb','rnb','rnb','rnb'],
	['rock','rock&roll','rock and roll','metal','grunge','punk','80s','70s','60s'],
	['oldies','50s','classic','40s','20s','30s','oldies','oldies','oldies'],
	['christian','inspirational','gospel','christ','hymn','worship','chapel','religious','spirit'],
	['contemporary','alternative','acoustic','garage','ecclectic','new age','alternative','alternative','alternative'],
	['top','2000s', 'hits','variety','pop','90s','billboard',' hot','top'],
	['talk','comedy','public','politics','sports','news','entertainment','commun','college'],
	['spanish','french','foreign','latin','celtic','caribbean','africa','world','asia']
);

if ($debug==1)
{
print $genreNickNames[2]."\n";
print $genreKeyWords[0][1]."\n";
print $length = @genreNickNames."\n";
sleep(2);
}

#show how many states we've done
$progress = Term::ProgressBar->new ({count => 51});

for (keys %stateHash)
{

	$counter++;
	$currentState = $stateHash{$_};

	#---- GET ALL THE CITIES WITH KNOWN STATIONS IN THIS STATE ----#
	@cities;

	$httpAgent = new LWP::UserAgent;
	$request = HTTP::Request->new('GET');

	if ($debug==1) {
	print "\nretrieving cities...\n";
	}

	$request->url($baseUrl."/$currentState");
	$response = $httpAgent->request($request);

	#save the data to a local HTML file
	$saveFile = $saveFileBase.$currentState;
	open(SAVE,'>',$saveFile.".html") or die ("unable to create $saveFile save file\n");
	print SAVE $response->decoded_content;
	close(SAVE);

	if ($debug==1) {
	print "cities saved as raw HTML\n";
	}

	#use w3m to make the HTML more parsable
	`w3m -dump $saveFile.html > $saveFile.txt`;

	if ($debug==1) {
	print "raw HTML converted to text\n\n";
	}

	#print "state number: $counter\n";

	#remove the original HTML --> we now have a nice text file to deal with instead
	`rm $saveFile.html`;

	#read in the file detailing what cities in this state contain radio station genre info
	open(IN,'<',$saveFile.".txt") or die ("unable to open $saveFile.txt for reading\n");

	if ($debug==1) {
	print "parsing $saveFile.txt for cities:\n\n";
	}

	#---- IDENTIFY CITIES ----#

			#start at 'cities', end at 'more'. remember that cities are preceded with bullet points
			@cities=();

			$lineCounter = 0;
			$startParsing = 0;

			while (my $line = <IN>) {

				$lineCounter++;

				#trim left side whitespace
				$line =~ s/^\s+//;

				#skip empty lines
				if ($line =~ m/^$/)
				{
					next;
				}

				#start parsing at 'cities'
				if ($line =~ m/^Cities/) 
				{
					$startParsing = 1;
					next;
				}					

				#stop parsing when you reach the line beginning with 'more'
				if ($line =~ m/^More/) 
				{
					#print join(", ",@cities);
					#cprint $cities[0];
					#print ("\n\n");
					last;
				}

				#parse only if you've reached the right part of the file
				if ($startParsing==1)
				{
					#parse the line
					($bullet, $city, $city2) = split /\s+/, $line;

					if ($city2)
					{
						$city = $city."_".$city2."_$_";
					} else {
						$city = $city."_$_";
					}

					push(@cities,$city);
				}

			}

	#remove the text file --> we have everything we need from it stored in memory
	close(IN);
	`rm $saveFile.txt`;

	#we now know which cities in this state have radio stations with online genre info
	#let's get that station info for each city in the list, and add data for each station we find to the master list
	if ($debug==1) {
	print "retrieving stations for each city...\n\n";
	}

	for my $i (0..@cities-1)
	{
		if ($debug==1) {
			print "\n------------------------\n$cities[$i]\n------------------------\n";
		}

		$request->url($cityBaseUrl."/$cities[$i]");
		$response = $httpAgent->request($request);

		#save the data to a local HTML file
		$saveFile = $saveFileBase.$cities[$i];
		open(SAVE,'>',$saveFile.".html") or die ("unable to create $saveFile save file\n");
		print SAVE $response->decoded_content;
		close(SAVE);

		if ($debug==1) {
			print "$cities[$i]: stations saved as raw HTML\n";
		}

		#use w3m to make the HTML more parsable
		`w3m -dump $saveFile.html > $saveFile.txt`;

		if ($debug==1) {
			print "raw HTML converted to text\n";
		}

		#remove the original HTML --> we now have a nice text file to deal with instead
		`rm $saveFile.html`;

		#read in the file detailing the stations for this city
		open(IN,'<',$saveFile.".txt") or die ("unable to open $saveFile.txt for reading\n");

		if ($debug==1) {
			print "\n";
		}

		$lineCounter = 0;
		$skipLines = 0;
		$descrNext = 0;
		$city = $cities[$i];
		$freq = "";
		$genre = "";

		#get all the station details and add to master list
		while (my $line = <IN>) {
				$lineCounter++;

				#trim left side whitespace
				$line =~ s/^\s+//;

				#skip empty lines
				if ($line =~ m/^$/)
				{
					next;
				}

				#skip blacklisted lines
				if ($skipLines > 0)
				{
					$skipLines --;
					next;
				}

				#start parsing at 'Radio Stations from'
				if ($line =~ m/^Radio Stations from/) 
				{
					$startParsing = 1;
					$skipLines = 4;

					next;
				}					

				#stop parsing when you reach Listen...
				if ($line =~ m/^Listen/) 
				{
					#print join(", ",@cities);
					#cprint $cities[0];
					if ($debug==1) {
						print ("\n\n");
					}
					last;
				}

				#parse only if you've reached the right part of the file
				if ($startParsing==1)
				{
					#parse the line
					($one, $two, $three) = split /\s+/, $line;

					if ($one eq "FM")
					{
						$freq = $two;
						$skipLines = 1;
						$descrNext = 1;
						next;
					}

					if ($descrNext == 1)
					{
						$descrNext = 0;

						#figure out what genre this station is
						$line =~ tr/[A-Z]/[a-z]/;
						#print "description = $line\n";
						OUTER: foreach my $j (0..@genreKeyWords-1)
						{
							foreach my $k (0..@{$genreKeyWords[$j]}-1)
							{
								#print ("testing $genreKeyWords[$j][$k]\n");
								if ($line =~ m/$genreKeyWords[$j][$k]/)
								{
									#print "\n\nMATCHED $genreNickNames[$j]\n\n";
									$genre = $genreNickNames[$j];
									last OUTER;
									#print "genre is $genre\n";
								}

							} #end checking each keyword for this genre

						} #end checking genres

						#assign this station's details to the master list and move on
						if ($genre eq "")
						{
							$genre = "never heard this kind of music before";
							#print "ERROR: GENRE UNKNOWN!";
							$unknownGenreCounter++;
						}

						#make all city letters uppercase
						$city =~ tr/[a-z]/[A-Z]/;

						#replace underscores with -'s
						$city =~ s/_/-/;

						push (@freqs,$freq);
						push (@citiesDb,substr($city,0,-3));
						push (@genres,$genre);
						push (@states,substr($city,-2));

						$stationCounter ++;

						if ($debug==1) {
							print("$freq, $city, $genre\n");
						}

						$freq = "";
						$genre = "";
						$skipLines = 3;

						#print "\tnext station\n";

					} #end what to do when the description comes in

				} #end what to do when parsing

			} #end this city


		#remove this city's text file --> we have everything we need from it stored in memory
		close(IN);
		`rm $saveFile.txt`;

		#update the progress bar
		$progress->update ($counter);

	} #end adding each city's station details to master list

	#now do the next state
}

print "\n\n\n------------------------\nMaster List\n------------------------\n";

for ($i=0; $i<@freqs; $i++)
{
	print "$freqs[$i]\t$citiesDb[$i], $states[$i]\t$genres[$i]\n";
}

print "\n------------------\nStations of Unknown Genre: \t$unknownGenreCounter / $stationCounter\n------------------------\n";

$mins = (time()-$startTime)/60;
print "\n------------------\nTime Elapsed: ".floor($mins)." minutes\n------------------------\n";

Time::HiRes::usleep(250000);


#---- UPDATE THE FMSTATIONS DATABASE ----#

print "\n\n\n===============================================\nUPDATE THE FM STATIONS DATABASE\n===============================================\n";

$stationsUpdated = 0;
$stationsMissing = 0;
$stationCounter2 = 0;

Time::HiRes::usleep(250000);

$fmStations = DBI->connect("DBI:Pg:dbname=fmStations;host=localhost","postgres","",{'RaiseError'=>1}) or die "Failed to connect to the fmStations database";

print("Established a connection to the fmStations database\n");

Time::HiRes::usleep(250000);



foreach $i (0..@freqs-1)
{
	#see if it is possible to update a record for this station

		#syncDb.pl always nulls out the genre field

		#boot any commas that made their way into the frequency column somehow
		$freqs[$i] =~ s/,//;

		if (!(($genres[$i] ne "never heard this kind of music before")&&($freqs[$i] > 1)&&(looks_like_number($freqs[$i]))&&(index($freqs[$i],',')<0))) {
			next;
		}

		$expression = "SELECT count(*) FROM stations WHERE freq=\'$freqs[$i]\' and city=\'$citiesDb[$i]\' and state=\'$states[$i]\' and genre=\'\'";
		#print $expression."\n";qq
		$count = $fmStations->selectrow_array($expression, undef);
		print "$freqs[$i], $citiesDb[$i], $states[$i] = $count\n";

		print("freq = $freqs[$i]\n");

		#now actually do the update for each row
			$statement = $fmStations->prepare("UPDATE stations SET genre=\'$genres[$i]\' WHERE freq=\'$freqs[$i]\' and city=\'$citiesDb[$i]\' and state=\'$states[$i]\' and genre=\'\'");
	        $statement->execute();

	        #keep track of stats
	        $stationCounter2++;
	        $stationsUpdated += $count;
    		if ($count==0) {
				$stationsMissing++;
			}
}

		print ("\n\n_________________________________________\n\nStations Affected: $stationsUpdated ... Expected: ".$stationCounter2."\n");
		print ("\nStations Missing: $stationsMissing\n\n\n");