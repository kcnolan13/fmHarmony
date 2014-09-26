
#=================================================================
# APPLICATION NAME: kmlBuilder.pl
#
#
# DESCRIPTION: Builds unique KML files for radio stations within 100 miles of user. Triggered by fmHarmony clients via HTTP request.
#
# ENVIRONMENT PARAMS:
#
# INPUT PARAMETERS: clientUniqueId, Lat, Lon
#
# OUTPUT PARAMETERS:
#
# AUTHORS: Kyle Nolan, Marcel Marki
#
# DATE: 29 AUG 2014
#  
#================================================================

#load modules
use DBI;
use Math::Round;
use Math::Trig;
#use warnings;
use Time::HiRes;
use LWP::UserAgent;
use POSIX qw(ceil floor);

use Math::Trig ':pi';


#---- GLOBAL VARS ----#
$fNameTemp = "./KML/".$ARGV[0]."_temp.kml";
$fName = "./KML/".$ARGV[0].".kml";
#find stations within this many miles of the user
$stationRadius = 50;
$styleTabs = "\t\t\t\t\t\t\t";
$stationScale = 1;
$stationIcon = "./icons/station";
@stationData = ("call", "freq", "genre", "city", "state", "erp", "erpunits", "haat", "haatunits", "lat", "lon");
@stationDataNickNames = ("CallSign", "Tune To", "Genre", "City of Origin", "State", "Effective Radiated Power", "erpUnits", "Height Above Average Terrain", "haatUnits", "Latitude", "Longitude");

$fontSize = 2;
$fontFamily = "Arial";

$miles2Km = 1.60934;
$km2m = 1000;

#---- SUBROUTINES ----#

#returns distance in miles
sub earthDistance {
	my ($lat1, $lon1, $lat2, $lon2) = @_;
	my $theta = $lon1 - $lon2;
	my $distance = 60*1.1515*rad2deg(acos(sin(deg2rad($lat1))*sin(deg2rad($lat2))+cos(deg2rad($lat1))*cos(deg2rad($lat2))*cos(deg2rad($theta))));
	return ($distance);
}

#open build log for appending and temp KML file for writing (this will be used later to overwrite any preexisting KML files for this client)
open (OUT, ">>", "./KML/kmlBuildLog.txt") or die("ERROR: unable to append to KML build log file\n");
#print("opening $fNameTemp\n");
open (TEMP, ">", $fNameTemp) or die("ERROR: unable to create $fNameTemp\n");
#print("output files opened\n");

#let the client know we will work on a KML for them
print "192.168.0.3 Says:\n\tI see you want a KML file. I'll get right to it!\n\n";

#make a new entry in the build log
print OUT "\n________________________________________________________________________________________________________________________________________________\n\n\nnew KML build: $fName\n\n\nclient: $ARGV[0]\nLat: $ARGV[1]\nLON: $ARGV[2]\n\n";

$userLat = $ARGV[1];
$userLon = $ARGV[2];

#---- BUILD THE ACTUAL KML FILE ----#
	
	#establish a connection to the fmStations database
	$fmStations = DBI->connect("DBI:Pg:dbname=fmStations;host=localhost","postgres","",{'RaiseError'=>1}) or die "Failed to connect to fmStations database";

	print OUT "Established a connection to the fmStations database\n";

	#generate the cookie-cutter xml and kml headings
    print TEMP '<?xml version="1.0" encoding="UTF-8"?>'."\n".'<kml xmlns="http://www.opengis.net/kml/2.2">'."\n";
    #create and name the document
    print TEMP "\t".'<Document>'."\n\t\t".'<name>Radio Stations Within '.$stationRadius.' Miles</name>'."\n";

    #---- GENERATE STYLES ----#
    	print OUT "\n===========================\nGenerating Styles\n===========================\n";
    	#make a style for each station quality: station1.png - station5.png
    	print OUT "\n\tbeign";
    	for ($i=1; $i<=5; $i++)
    	{
    		print OUT ".";
            print TEMP $styleTabs.'<Style id="station'.$i."\">\n".$styleTabs."\t".'<IconStyle>'."\n".$styleTabs."\t\t".'<scale>'.$stationScale.'</scale>'."\n";
            print TEMP $styleTabs."\t\t".'<Icon>'."\n".$styleTabs."\t\t\t".'<href>'.$stationIcon.$i.".png".'</href>'."\n";
            print TEMP $styleTabs."\t\t".'</Icon>'."\n".$styleTabs."\t".'</IconStyle>'."\n".$styleTabs.'</Style>'."\n\n";
        }
        print OUT "end\n";

	# ------ GENERATE STATION PLACEMARKS ------ #
		print OUT "\n===========================\nGenerating Placemarks\n===========================\n\n";
        print TEMP "\t\t".'<Folder id="stationPlacemarks">'."\n";

        #fetch all the station data we want from the fmStations database
                $stationDataString = "";
                for ($i=0; $i<@stationData; $i++)
                {
                    $stationDataString = $stationDataString.$stationData[$i];
                    if ($i<@stationData-1) {
                        $stationDataString = $stationDataString.", ";
                    }
                }

                #$query = $fmStations->prepare("SELECT call from stations WHERE freq>106 ORDER BY uniqueid");
                $expression = "SELECT $stationDataString FROM stations WHERE earth_box(ll_to_earth($userLat, $userLon), $stationRadius*$miles2Km*$km2m) @> ll_to_earth(lat,lon) ORDER BY uniqueId";

                print OUT "\tpreparing query . . .\n\t$expression\n\n\tbegin";

                $query = $fmStations->prepare($expression);
                $query->execute();

		#construct the actual station placemarks
            $stationIndex = 0;
                while(my $row = $query->fetchrow_hashref()) {
                        #log fetched station details for those supposedly in range
                        
                        print OUT ".";
                        print TEMP "\t\t\t".'<Placemark>'."\n\t\t\t\t".'<name>'."&lt;font face=$fontFamily size=$fontSize &gt; ".$row->{'call'}.'</name>'."\n";
                        print TEMP "\t\t\t\t".'<description>'."&lt;font face=$fontFamily size=$fontSize&gt; "."\n";

                        #description data
                        print TEMP "\t\t\t\t".nearest(.1,$row->{'freq'}).'&lt;br&gt;'."\n";
                        print TEMP "\t\t\t\t".$row->{'genre'}.'&lt;br&gt;'."\n";

                        print TEMP "\t\t\t\t".'</description>'."\n";

                        #radius --> what is the station's area of good signal effect?
                        $radiusOfEffect = -999;
                        #print TEMP "\t\t\t\t".'<radius>'.$radiusOfEffect.'</radius>'."\n";

                        $distFromUser = nearest(1,earthDistance($userLat, $userLon, $row->{'lat'}, $row->{'lon'}));
                        #print OUT "milesAway = $distFromUser\t\t";
                        #print OUT "$stationData[0] = $row->{$stationData[0]}\t$stationData[1] = $row->{$stationData[1]}\t$stationData[2] = $row->{$stationData[2]}\t$stationData[3] = $row->{$stationData[3]}\t$stationData[4] = $row->{$stationData[4]}\n";
                        
                        #print TEMP "\t\t\t\t".'<distFromUser>'.$distFromUser.'</distFromUser>'."\n";
                        print TEMP "\t\t\t\t".'<index>'."$stationIndex".'</index>'."\n";

                        #attach the right icon for this signal strength
                        	#copute a discrete strength (1-5) to assign to the station
                        	$strength = nearest(1,5-$stationRadius/$distFromUser);

                        $styleUrl = '#'."station".$strength;

                        print TEMP "\t\t\t\t".'<styleUrl>'."$styleUrl".'</styleUrl>'."\n";

                        print TEMP "\t\t\t\t".'<Point>'."\n";
                        #use height above average terrain (HAAT) as elevation
                        print TEMP "\t\t\t\t\t".'<coordinates>'."$row->{'lon'}, $row->{'lat'}, $row->{'haat'}".'</coordinates>'."\n";
                        print TEMP "\t\t\t\t".'</Point>'."\n\t\t\t".'</Placemark>'."\n\n";

                        $stationIndex++;
                    }

                    print OUT "end\n";


        #end the wind barbs section of KML
        print TEMP "\t\t".'</Folder>'."\n\n";

        #finish document
        print TEMP "\t".'</Document>'."\n".'</kml>';

#temp build is now complete
close(TEMP);

print OUT "\n===========================\nFinishing Up\n===========================\n";

#copy over the temp file to its permanent address
$result = `mv $fNameTemp $fName`;

#make sure it got there okay
if (!$result) {
	print OUT "\nGood Copy!\n";
} else {
	print OUT "ERROR: BAD COPY.\n\n";
}

#complete the build log
print OUT "\nBuild Complete.\n\n________________________________________________________________________________________________________________________________________________\n";

#relay back to the client that everything has been taken care of
print "192.168.0.3 Says:\n\tYou, sir, have been served. ($fName)\n";

#finish up
close(OUT);
