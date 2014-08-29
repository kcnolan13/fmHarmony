<?php

	if(!empty($_GET['uniqueId']))
	{
		$uniqueId = $_GET['uniqueId'];
		$lat = $_GET['lat'];
		$lon = $_GET['lon'];

		//echo "Received data: $uniqueId @ $lat, $lon\r\n";

		$result = shell_exec("/usr/bin/perl /var/www/html/server/kmlBuilder.pl $uniqueId $lat $lon");

		echo "$result\r\n";

	}
	
?>