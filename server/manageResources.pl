#=================================================================
# APPLICATION NAME: manageResources.pl
#
#
# DESCRIPTION: deletes KML files that are no longer in use, and removes the KML log file when it gets too big
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
use File::Find::Rule;

$logFile = "/home/kyle/fmHarmony/server/KML/kmlBuildLog.log";
$logSize = -s $logFile;

#remove files older than 5 mins
my $keepable = time() - 86400/(96*3);

#keep log less than 1 MB
if ($logSize > 1000000)
{
	unlink($logFile);
	open(OUT, ">", $logFile);
	print OUT "------------\nolder builds removed to minimize file size\n------------\n";
	close(OUT);
	`chmod 777 $logFile`;
}

@oldKmlFiles = File::Find::Rule->file()->name('*.kml')->mtime("<=$keepable")->in('/home/kyle/fmHarmony/server/KML/');

#remove the old KML files
unlink @oldKmlFiles;

@extraTxtFiles = File::Find::Rule->file()->name('*.txt')->in('/home/kyle/fmHarmony/server/KML/');

#remove the old txt files
unlink @oldTxtFiles;