<?php

include("geoipcity.inc");
include("geoipregionvars.php");

$gi = geoip_open("GeoLiteCity.dat",GEOIP_STANDARD);

  $i=$_SERVER['REMOTE_ADDR'];
  echo "$i\n";
  $record = geoip_record_by_addr($gi,$i);
  
	print $record->country_code . " " . $record->country_code3 . " " . $record->country_name . "\n";
	print $record->region . " " . $GEOIP_REGION_NAME[$record->country_code][$record->region] . "\n";
	print $record->city . "\n";
  
  //print $record->postal_code . "\n";
  //print $record->latitude . "\n";
  //print $record->longitude . "\n";
  //print $record->metro_code . "\n";
  //print $record->area_code . "\n";
  //print $record->continent_code . "\n";
  geoip_close($gi);
?>
