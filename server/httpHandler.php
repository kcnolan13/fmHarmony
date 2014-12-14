<?php
	#invoke the KML builder for connecting clients (client data passed via HTTP GET)

	if(!empty($_GET['uniqueId']))
	{
		$uniqueId = $_GET['uniqueId'];
		$lat = $_GET['lat'];
		$lon = $_GET['lon'];
		$rad = $_GET['rad'];

		echo "Received data: $uniqueId @ $lat, $lon\r\n";

		#build a KML file for this client's uniqueId, based on lat, lon, and desired radius
		$result = shell_exec("/usr/bin/perl /var/www/html/fmHarmony/kmlBuilder.pl $uniqueId $lat $lon $rad");

		echo "$result\r\n";

	}
	
?>