<?php
include 'admin/dbLoginInfo.php';

header("Expires: ".gmdate("D, d M Y H:i:s")."GMT");
header("Cache-Control: no-cache, must-revalidate");
header("Pragma: no-cache");

/*
a: the action type (0 = remove ip, 1 = add ips, 2 = get news and location, >2 get a ips).
p: the port.
k: the key
r: result (0 = removed room, 1 = fail, 2 = got list, 3 = got news and location, 4 = published room)
*/

$UPLOAD_FOLDER = "download";

  function full_url()
  {
      $s = empty($_SERVER["HTTPS"]) ? '' : ($_SERVER["HTTPS"] == "on") ? "s" : "";
      $protocol = substr(strtolower($_SERVER["SERVER_PROTOCOL"]), 0, strpos(strtolower($_SERVER["SERVER_PROTOCOL"]), "/")) . $s;
      $url = $protocol . "://" . $_SERVER['SERVER_NAME']. $_SERVER['REQUEST_URI'];
      $url = substr($url, 0, strrpos($url, "/")); //to remove "connect.php" from the addr.
      return $url;
  }


$errorMsg = "";
if (!isset($_POST['a'])) die("1\n{*}Missing type.");
$connection = @mysql_connect($DB_HOST_NAME,$DB_HOST_USER,$DB_HOST_PASSWORD) or die("1\n{*}Could not connect to Database Host: ".mysql_error());
if (!mysql_select_db($DB_NAME)) die("1\n{*}Could not access the database: ".mysql_error());

/* Protect against SQL Injection: */
$action = addslashes(htmlspecialchars($_POST['a']));
$ip = $_SERVER['REMOTE_ADDR'];
$port = addslashes(htmlspecialchars($_POST['p']));

if ($action > 2) {
	if (!isset($_POST['k'])) die("1\n{*}Missing key.");	
	if ( md5($action.$APP_SECRET) != $_POST['k'] ) die("1\n{*}Invalid key.");

 	//GET IPS:
	$sql = mysql_query("SELECT * FROM hosts ORDER BY time DESC LIMIT 0,$action") or die("1\n{*}Could not get ips: ".mysql_error());
	echo "2";
	while ($reg = mysql_fetch_array($sql))
		echo "\n{*}".$reg['ip']." ".$reg['port'];	
} else {
	if ($action == 2) {

		//GET UPDATES LIST:
		$sql = mysql_query("SELECT * FROM updates ORDER BY time DESC LIMIT 0,1") or die("1\n{*}Could not get news: ".mysql_error());
		$reg = mysql_fetch_array($sql);	
		echo "3";	

		//GET TIME WELL FORMATTED:
		include_once("components/timeConversion.php");
		echo "\n{*}".getTime($reg['time']);
		echo "\n{*}".$reg['description'];	
		echo "\n{*}".$reg['version'];
		echo "\n{*}".$reg['versionlog'];
		$url = 	full_url();
		echo "\n{*}".$url."/".$UPLOAD_FOLDER."/".$reg['version'];



		//GET IP:
		$i=$_SERVER['REMOTE_ADDR'];
		echo "\n{*}$i";

		//GET GEO LOCATION:
		include("GeoLocationAPI/geoipcity.inc");
		include("GeoLocationAPI/geoipregionvars.php");
		$gi = geoip_open("GeoLocationAPI/GeoLiteCity.dat",GEOIP_STANDARD);
		$record = geoip_record_by_addr($gi,$i);
		print "\n{*}".$record->city.", ".$record->region.". ".$record->country_name;
		geoip_close($gi);

	} else {	
		if (!isset($_POST['p'])) die("1\n{*}Missing port.");
		if (!isset($_POST['k'])) die("1\n{*}Missing key.");
		if ( md5($port.$APP_SECRET) != $_POST['k'] ) die("1\n{*}Invalid key.");
		if ($action == 0) {
			//REMOVE IP:
			$sql = mysql_query("DELETE FROM hosts WHERE ip = '$ip' AND port = '$port'") or die("1\n{*}Could remove ip from list: ".mysql_error());
			echo "0";
		} else if ($action == 1) { 
			//ADD IP:
			$sql = mysql_query("INSERT INTO hosts (ip, port) VALUES ('$ip','$port') ON DUPLICATE KEY UPDATE time = NOW(), port = '$port'") or die("1\n{*}Could not add ip to list: ".mysql_error());
			$sql = mysql_query("DELETE LOW_PRIORITY FROM hosts where time < NOW() - INTERVAL $ROOM_DAYS DAY") or die("1\n{*}Could not delete ips: ".mysql_error());
			echo "4";
		}
		
	}
}
?>