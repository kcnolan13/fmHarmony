
#=================================================================
# APPLICATION NAME: updateDb.pl
#
#
# DESCRIPTION: Automatically updates the fm radio station database with the latest data available from the FCC
#
# AUTHORS: Kyle Nolan, Marcel Marki
#
# DATE: 2013.08.14
#
# CONTRIBUTING AUTHORS:
#
# REVISIONS: 
# 
#
#================================================================

#load modules
use DBI;
use Math::Round;
use Math::Trig;
#use warnings;
use Time::HiRes;
use LWP::UserAgent;

# ----- GLOBAL VARS ----- #
$URL = "http://transition.fcc.gov/fcc-bin/fmq?state=&call=&city=&arn=&serv=&vac=&freq=0.0&fre2=107.9&facid=&asrn=&class=&dkt=&list=2&dist=&dlat2=&mlat2=&slat2=&NS=N&dlon2=&mlon2=&slon2=&EW=W&size=9";
#$URL = "http://transition.fcc.gov/fcc-bin/fmq?state=ME&call=&city=&arn=&serv=&vac=&freq=0.0&fre2=107.9&facid=&asrn=&class=&dkt=&list=1&dist=&dlat2=&mlat2=&slat2=&NS=N&dlon2=&mlon2=&slon2=&EW=W&size=9";
$saveFile = "fccData";

print "\n\n========================\nPHASE I: DATA RETRIEVAL\n========================\n\n";


#retrieve the latest data from the FCC
$httpAgent = new LWP::UserAgent;
$request = HTTP::Request->new('GET');

$requestTime = time();
print "commencing FCC data retrieval\n";

$request->url($URL);
$response = $httpAgent->request($request);

print time()-$requestTime;
print " seconds spent retrieving FCC data\n";

#save the data to a local HTML file
open(SAVE,'>',$saveFile.".html") or die ("unable to create $saveFile save file\n");
print SAVE $response->decoded_content;
close(SAVE);
print "FCC data saved successfully as HTML\n";

#use lynx to make the HTML more parsable
`w3m -dump fccData.html > fccData.txt`;
print "raw HTML converted to text\n";

#open up that text file for parsing
open(IN,'<',$saveFile.".txt") or die ("unable to open $saveFile.txt for reading\n");

print("FCC DATA DOWNLOADED, DECODED, AND OPENED FOR READING.\n\n");

sleep(2);

# ----- BEGIN PARSING THE FCC DATA ----- #

print("========================\nPHASE II: DATABASE UPDATES\n========================\n\n");

print("FCC text file parsing in 3...");
sleep(1);
print(" 2...");
sleep(1);
print(" 1...");
sleep(1);
print("\n\n");

#advance to the correct part of the input file
$lineCounter = 0;
$entryCounter = 0;
$trigger = 0;

while (my $line = <IN>) {

	$lineCounter++;
	#trim left side whitespace
	$line =~ s/^\s+//;

	#skip empty lines
	if ($line =~ m/^$/)
	{
		next;
	}

	#stop parsing at the *** X Records Retrieved *** line
	if ($line =~ m/\*\*\*/) 
	{
		print "$entryCounter Records Reviewed\n";
		last;
	}

	#look for the Class and Frequency keywords
	if (($line =~ m/^Call\s+Channel/)&&(!($trigger)))
	{
	    print "record parsing triggered: beginning with line ";
	    print $lineCounter+1;
	    print "\n";
	    $trigger = 1;
	} elsif ($trigger)
	{
		$entryCounter ++;

		#first set of params with a common split method
		($call, $channel, $class, $service, $freq, $freqUnits) = split /\s+/, $line;

		#next set of params with a common split method
		($status, $city) = split /\s{2,}/, substr($line,31);

		#third set of params with a common split method
		($state, $country, $fileNumber, $docket, $facilityId, $erp, $erpUnits) = split /\s+/, substr($line,65);

		#ignore DA -- not important at the moment
		#trim the white space in front of HAAT
		$sub = substr($line,130);
		$sub =~ s/^\s+//;

		($haat, $haatUnits, $rcamsl, $rcamslUnits, $rcagl, $rcaglUnits, $latDir, $latDeg, $latMin, $latSec, $lonDir, $lonDeg, $lonMin, $lonSec, $asrn) = split /\s+/, $sub;

		$uniqueId = $call."_".$city."_".$freq;
		$uniqueId =~ s/\./-/;

			#$freqUnits, $status, $city, $state, $country, $fileNumber, $docket, $facilityId, $erp, $da, $haat, $rcamsl, $rcagl, $lat, $lon, $asrn, $license)  = split /\s+/, $line;
			print "$uniqueId\t$erpUnits\t\t\t$latDir\t$lonDir\n";
	}
}