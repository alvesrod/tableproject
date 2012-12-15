<?php 

include_once("timeConversion.php");

include '../components/head.php';

include '../components/loadPageSQL.php';   

if ($loadPage)
	include '../components/top.php';
else die("Page could not be loaded.");
?>