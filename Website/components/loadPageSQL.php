<?php
$loadPage = false;
$connection = @mysql_connect($DB_HOST_NAME,$DB_HOST_USER,$DB_HOST_PASSWORD);
if (!$connection) echo "<script>window.location='../error/?e=1';</script>";
else {
	if (!isset($newsLimit)) $newsLimit = 1;
	if (!@mysql_select_db($DB_NAME)) echo "<script>window.location='../error/?e=2';</script>";	else {
		$sql = @mysql_query("SELECT * FROM updates ORDER BY time DESC LIMIT 0,$newsLimit");
		if (!$sql) echo "<script>window.location='../error/?e=3';</script>";
		else {
		  if (@mysql_num_rows($sql)>=1) {
		    $reg = mysql_fetch_array($sql);
		    $news = $reg['description'];
		    $version = $reg['version'];
		    $latestVersion = $version;
		    $vlog = $reg['versionlog'];
		    $time = getTime($reg['time']);
		    $loadPage = true;
		  } else  echo "<script>window.location='../error/?e=4';</script>";

		}
	}
}
//$loadPage is true if you can load the page from here.
?>