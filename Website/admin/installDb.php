<?php

include 'dbLoginInfo.php';

  session_start();
  if ((!isset($_SESSION['sm']))||($_SESSION['sm']==false)) echo '<script> window.location="../index.php"</script>';
 else {
if (  (isset($_SESSION['name_db'])) && (isset($_SESSION['name_host'])) && (isset($_SESSION['name_login'])) && (isset($_SESSION['name_pass']))  )  {
  $connection = @mysql_connect($_SESSION['name_host'],$_SESSION['name_login'],$_SESSION['name_pass']) or die(mysql_error());
  $db = $_SESSION['name_db'];
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
	<body bgcolor="#000000">
	<td valign="top">
        <img src="img/s.gif" class="vspacer" height="15">
        <img border="0" src="img/i5.png"><div style="padding-left:20px;padding-right:20px;">
    	<h2 align="center"><font color="#C0C0C0">Engine Setup</font></h2>
	    <div style="padding: 5px;" align="center" valign="middle">
	    <table cellspacing="0" cellpadding="0" border="0" width="497"></td>
	<tr>
    <td width="21"><img src="img/i1.gif"></td>
	    <td bgcolor="#858380" align="right"></td>
	    <td width="21"><img src="img/i2.gif"></td>
	    <td width="4" valign="middle" rowspan="3"></td>
    </tr><tr><td bgcolor="#858380" height="94"></td><td bgcolor="#858380" valign="top" height="94">

<?php
    if (!mysql_select_db($db))
  {
    echo "Database $db wasn't found.";
    mysql_query("CREATE DATABASE $db") or die(mysql_error());
    mysql_select_db($db) or die(mysql_error());
    echo "<br>New Database created.";
  } else echo "Database $db was found!";

mysql_query("CREATE TABLE IF NOT EXISTS `hosts` (
  `ip` varchar(18) NOT NULL,
  `port` smallint(5) unsigned NOT NULL,
  `time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY `ip` (`ip`)
)") or die("Could not create the hosts table: ".mysql_error());

mysql_query("CREATE TABLE IF NOT EXISTS `updates` (
  `time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `description` text NOT NULL,
  `version` varchar(10) NOT NULL,
  `versionlog` text NOT NULL,
  UNIQUE KEY `version` (`version`),
  PRIMARY KEY `time` (`time`)
)") or die("Could not create the updates table: ".mysql_error());


  $filename ="dbLoginInfo.php";
  $fp = @fopen($filename,w) or dye("<br> Error: Data could not be saved");
  fputs($fp,"<?php \n");

  fputs($fp,'$DB_HOST_NAME="'.$_SESSION['name_host'].'"; ');
  fputs($fp,'$DB_HOST_USER="'.$_SESSION['name_login'].'"; ');
  fputs($fp,'$DB_HOST_PASSWORD="'.$_SESSION['name_pass'].'"; ');
  fputs($fp,'$DB_NAME="'.$_SESSION['name_db'].'"; ');
  fputs($fp,'$ROOM_DAYS='.number_format($_SESSION['room_days']).'; ');
  fputs($fp,'$CPASSWORD="'.$_SESSION['config_pass'].'"; ');
  fputs($fp,'$APP_NAME="'.$_SESSION['app_name'].'"; ');
  fputs($fp,'$APP_SECRET="'.$_SESSION['app_secret'].'"; ');

  fputs($fp,"?>");
  @fclose($fp);
  $_SESSION['sm']=false;
  echo "<br>Done!";
  session_regenerate_id();
?>


</font></p></form></td><td bgcolor="#858380" height="94"></td>
</tr><tr><td width="21"><img src="img/i3.gif"></td>
<td bgcolor="#858380" align="right" style="font-size:0.75em;padding-right:15px;"></td>
<td width="21"><img src="img/i4.gif"></td></tr></table>
</div><p></div>

<?php
}}
?>