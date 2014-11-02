
#=============================================================================================================================
# APPLICATION NAME: updateDb.pl
#
#
# DESCRIPTION: Automatically updates the fm radio station database with the latest data available from the FCC
#
# AUTHORS: Kyle Nolan, Marcel Marki
#
# DATE: 2014.08.14
#
# CONTRIBUTING AUTHORS:
#
# REVISIONS: 
# 
# 
#============================================================================================================================

#load modules
use DBI;
use Math::Round;
use Math::Trig;
#use warnings;
use Time::HiRes;
use LWP::UserAgent;
use POSIX qw(ceil floor);

# ----- SUBROUTINES ----- #
sub searchTerm
{
	$found = 0;
	#print "searching for term $_[0]\n";
	for (my $i=0; $i < @{($_[1])}; $i++)
	{
		#print "\t\ttrying $_[1][$i]\n";
		if ($_[0] eq $_[1][$i])
		{
			$found = 1;
		}
	}

	#return true for found, false for missing
	$found;
}

# ----- GLOBAL VARS ----- #
$URL = "http://transition.fcc.gov/fcc-bin/fmq?state=&call=&city=&arn=&serv=FM&vac=&freq=0.0&fre2=107.9&facid=&asrn=&class=&dkt=&list=2&dist=&dlat2=&mlat2=&slat2=&NS=N&dlon2=&mlon2=&slon2=&EW=W&size=9";
#$URL = "http://transition.fcc.gov/fcc-bin/fmq?state=ME&call=&city=&arn=&serv=&vac=&freq=0.0&fre2=107.9&facid=&asrn=&class=&dkt=&list=1&dist=&dlat2=&mlat2=&slat2=&NS=N&dlon2=&mlon2=&slon2=&EW=W&size=9";
$saveFile = "fccData";

print "\n\n======================================================\nPHASE I: DATA RETRIEVAL\n======================================================\n\n";

$startTime = time();

#retrieve the latest data from the FCC
$httpAgent = new LWP::UserAgent;
$request = HTTP::Request->new('GET');

print "commencing FCC data retrieval\n";

$request->url($URL);
$response = $httpAgent->request($request);

print time()-$startTime;
print " seconds spent retrieving FCC data\n";

#save the data to a local HTML file
open(SAVE,'>',$saveFile.".html") or die ("unable to create $saveFile save file\n");
print SAVE $response->decoded_content;
close(SAVE);
print "FCC data saved successfully as HTML\n";

#use w3m browser to make the HTML more parsable
`w3m -dump $saveFile.html > $saveFile.txt`;
print "raw HTML converted to text\n";

#remove the original HTML --> we now have a nice text file to deal with instead
`rm $saveFile.html`;


#open up that text file for parsing
open(IN,'<',$saveFile.".txt") or die ("unable to open $saveFile.txt for reading\n");

print("FCC DATA DOWNLOADED, DECODED, AND OPENED FOR READING.\n");

sleep(1);

# ----- BEGIN PARSING THE FCC DATA ----- #

print("\n======================================================\nPHASE II: fmStations DATABASE CONNECTION\n======================================================\n\n");

$fmStations = DBI->connect("DBI:Pg:dbname=fmStations;host=localhost","postgres","",{'RaiseError'=>1}) or die "Failed to connect to the fmStations database";

print("Established a connection to the fmStations database\n");

sleep(1);

print("\n======================================================\nPHASE III: DATABASE UPDATES\n======================================================\n\n");

print("parsing FCC data in :\n\t\t\t3...\n");
Time::HiRes::usleep(250000);
print("\t\t\t\t2...\n");
Time::HiRes::usleep(250000);
print("\t\t\t\t\t1...\n\n");
Time::HiRes::usleep(250000);

#advance to the correct part of the input file
$lineCounter = 0;
$entryCounter = 0;
$invalidErpHaat = 0;
$invalidNonUs = 0;
$trigger = 0;

#1st dim: channel, 2nd dim: set of uniqueId's within that channel
#allows checking for expired stations by channel --> much more efficient
@uniqueIds;


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
		print "\n\n$entryCounter Records Reviewed, ";
		print ($invalidNonUs+$invalidErpHaat);
		print " discarded\n";
		print "$invalidNonUs Stations Discarded for non-US origin\n";
		print "$invalidErpHaat Stations Discarded for lacking ERP or HAAT ratings\n";
		last;
	}

	#look for the Class and Frequency keywords
	if (($line =~ m/^Call\s+Channel/)&&(!($trigger)))
	{
	    print "\nrecord parsing triggered: beginning with line ";
	    print $lineCounter+1;
	    print "\n\n";
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

		#skip any non-US radio stations that somehow made it into the FCC results
		if ($country ne "US")
		{
			print("SKIPPING radio station $call: \t\tnon-US country code\n");
			$invalidNonUs++;
			next;
		}

		#convert latitude from degMinSec to decimal degrees
		if ($latDir eq "S") {
			$sign = -1;
		} else {
			$sign = 1;
		}

		$lat = $sign*$latDeg;
		$lat += $sign*$latMin/60; 
		$lat += $sign*$latSec/3600;

		#convert longitude from degMinSec to decimal degrees
		if ($lonDir eq "W") {
			$sign = -1;
		} else {
			$sign = 1;
		}

		$lon = $sign*$lonDeg;
		$lon += $sign*$lonMin/60;
		$lon += $sign*$lonSec/3600;

		#eliminate any apostraphes and spaces in cities
		$city =~ s/\'//;
		$city =~ s/\s/-/;

		#handle any lone -'s in number fields
		$freq =~ s/^-$/NULL/;
		$erp =~ s/^-$/-99999999999/;
		$haat =~ s/^-$/-99999999999/;
		$lat =~ s/^-$/NULL/;
		$lon =~ s/^-$/NULL/;

		#skip entries that lack ERP or HAAT ratings
		if (($erp==-99999999999)||($haat==-99999999999))
		{
			print("SKIPPING radio station $call: \t\tmissing ERP or HAAT rating\n");
			$invalidErpHaat++;
			#sleep(3);
			next;
		}

		#develop a unique ID for each station
		$uniqueId = $call."_".$freq."_".$city."_".nearest(.0001,$lat)."_".nearest(.0001,$lon);
		$uniqueId =~ s/\./_/;

		#remember all uniqueId's for expiration checking later
		#stores uniqueId's in a 2D array, 1st dim = channel, 2nd dim = the uniqueIds
		push @{ $uniqueIds[$channel-200] }, $uniqueId;

		print "contents for channel $channel:\n";
		
		#my $size = @{$uniqueIds[$channel-200]};
		#print ("size is $size\n");
		#sleep(1);
		#for ($k=0; $k<$size; $k++)
		#{
		#	print "$uniqueIds[$channel-200][$k], ";
		#}

		#does an entry for this station exist already?
		$count = $fmStations->selectrow_array("SELECT count(*) FROM stations WHERE uniqueid=\'$uniqueId\';", undef);
		#print("count is $count\n");
		if ($count > 0) {
			#update the entry for this station
			print("updating record for $uniqueId\n");
			$statement = $fmStations->prepare("UPDATE stations SET call=\'$call\', genre=\'\', channel=$channel, class=\'$class\', service=\'$service\', freq=$freq, frequnits=\'$freqUnits\', status=\'$status\', city=\'$city\', state=\'$state\', country=\'$country\', erp=$erp, erpunits=\'$erpUnits\', haat=$haat, haatunits=\'$haatUnits\', lat=$lat, lon=$lon WHERE uniqueid=\'$uniqueId\';");
            $statement->execute();
		} else {
			#print("NEW STATION: $uniqueId\n");
			print("CREATING RECORD for $uniqueId\n");
			$statement = $fmStations->prepare("INSERT into stations (uniqueid, call, genre, channel, class, service, freq, frequnits, status, city, state, country, erp, erpunits, haat, haatunits, lat, lon) VALUES (\'$uniqueId\', \'$call\', \'\', $channel, \'$class\', \'$service\', $freq, \'$freqUnits\', \'$status\', \'$city\', \'$state\', \'$country\', $erp, \'$erpUnits\', $haat, \'$haatUnits\', $lat, $lon);");
			$statement->execute();
		}

		#Time::HiRes::usleep(100000);

	}
}

print("\n\nDATABASE UPDATED SUCCESSFULLY\n\n");


print("\n======================================================\nPHASE IV: IDENTIFY AND REMOVE EXPIRED STATIONS\n======================================================\n\n");

sleep(2);

#holds the uniqueIds for station database entries that are absent from the new FCC dataset
@expiredStations;
$expirationCount=0;

print("Identifying Expired Stations in:\n\t\t\t3...\n");
Time::HiRes::usleep(500000);
print("\t\t\t\t2...\n");
Time::HiRes::usleep(500000);
print("\t\t\t\t\t1...\n\n");
Time::HiRes::usleep(500000);

print("Cross-Referencing Records...\n\nStations Absent From New FCC Data:\n");

for ($i=200; $i<=300; $i++)
{
	print("\tChannel $i:\n");
	$statement = $fmStations->prepare("SELECT uniqueid from stations where channel=$i;");
	$statement->execute();

	Time::HiRes::usleep(25000);

	#match each uniqueId from the database with its counterpart in the 2D @uniqueIds array
	while(my $row = $statement->fetchrow_hashref()) {
		$theId = $row->{'uniqueid'};
		$result = searchTerm($theId, @uniqueIds[$i-200]);

		#store expired stations
		if ($result==0)
		{
			push(@expiredStations,$theId);
			print("\t\t$theId\n");
			$expirationCount++;
		}
	}
}

print("\n\n$expirationCount EXPIRED RADIO STATIONS IDENTIFIED\n");

print("removing expired stations from the database...\n");

sleep(1);

#delete groups of up to 25 stations per query
$delsPerQuery = 25;
$length = @expiredStations;

#remove expired stations
if ($length>0)
{
	$numDeletions = 0;

	#delete groups of stations 25 at a time
	for ($i=0; $i<ceil($length/$delsPerQuery); $i++)
	{
		#begin the SQL deletion statement
		$statementStr = "DELETE from stations where";

		#delete up to 25 stations
		INNER: 
		{
			for ($j=0; $j<$delsPerQuery; $j++)
			{
				if ($j>0)
				{
					$statementStr = $statementStr." OR ";
				}

				$statementStr = $statementStr." uniqueid='".$expiredStations[$delsPerQuery*$i+$j]."'";

				$numDeletions++;

				#break out from the inner loop if expiredStations array length is reached
				if ($delsPerQuery*$i+$j == $length-1)
				{
					last INNER;
				}
			}
		}

		$statementStr = $statementStr.";";

		#perform the deletions for this group
		print "\n$statementStr\n";

		$statement = $fmStations->prepare($statementStr);
		$statement->execute();
	}

	print "\n$numDeletions EXPIRED STATIONS REMOVED\n";
}


close($saveFile.".txt");
$fmStations->disconnect();

print "\n\nSynchronization Complete!\n\n";
$mins = (time()-$startTime)/60;
print "\nTime Elapsed: ".floor($mins)." minutes ".floor(($mins%1)*60)." seconds\n";