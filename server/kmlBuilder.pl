#load modules
use DBI;
use Math::Round;
use Math::Trig;
#use warnings;
use Time::HiRes;
use LWP::UserAgent;
use POSIX qw(ceil floor);

open (OUT, ">", "kmlBuildLog.txt") or die("unable to open text file for writing");

print "I see you want a KML file. I'll get right to it!\n";

print OUT "=========================\nnew KML build\n=========================\n\nclient: $ARGV[0]\nLat: $ARGV[1]\nLON: $ARGV[2]\n";

usleep(100000);

print "Build Complete.";

close(OUT);