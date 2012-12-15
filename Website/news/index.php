<?php include '../components/header.php'; ?>

		<li class="home"><a href="../">Menu</a></li>
		<li><a href="../files/">Files</a></li>

<?php include '../components/title.php'; ?>

		<li class="home"><a href="../">Home</a></li>
		<li class="about"><a href="../about/">What is this?</a></li>
		<li class="contact"><a href="../contact/">Contact</a></li>
	
</ul></div></div><div id="bodyPan"><div id="bodyleftPan"><br><br><h2>

	Old News

</h2><br><?php

 	include '../components/loadSQLList.php';

  	do { if ($news != "") { ?>
  	
  	
  	<div id="servicesPan"></div>
  	<div id="servicesBodyPan">
  	<p class="services"><?php echo $time; ?></p>
	<p><?php echo $news; ?></p>
	 </div>
	 <div id="servicesBottomPan"></div>
  	
	<?php	 
	 } if (!($reg = mysql_fetch_array($sql))) break;
	 $news = $reg['description'];
	 //$version = $reg['version'];
	 //$vlog = $reg['versionlog'];
	 $time = getTime($reg['time']);
	 ?>
	
	<?php } while (true); 

	include '../components/downloadLink.php';
	include '../components/foot.php';
?>              
