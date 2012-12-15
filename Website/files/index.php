<?php include '../components/header.php'; ?>

		<li class="home"><a href="../">Menu</a></li>
		<li class="company">Files</li>

<?php include '../components/title.php'; ?>

		<li class="home"><a href="../">Home</a></li>
		<li class="about"><a href="../about/">What is this?</a></li>
		<li class="contact"><a href="../contact/">Contact</a></li>
	
	</ul></div></div>
	<div id="bodyPan">
  	<div id="bodyleftPan"><h2>

	Latest Files

	</h2><br>
  	
	<a href="https://github.com/alvesrod/tableproject"><font color="#9C0707">>>> Download Source Code</font></a><br><br>
  	<?php 

	include '../components/loadSQLList.php';

  	do { ?>
  	
  	<div id="servicesPan"></div>
  	<div id="servicesBodyPan">

	<?php $zipfilename = '_' . str_replace(' ','',$APP_NAME) . 'v'.$version.'.zip'; ?>
	

  	<p> <?php echo '<a href="../download/'. $version . '/' . $zipfilename .'"'.'><span>Download version '.$version.'</span></a>'; ?> </p>
	<p><b>Published: </b><?php echo $time; ?></p>
	<div><?php echo $vlog; ?></div></div>
	<div id="servicesBottomPan"></div>
  	
	 <?php	 

	 	if (!($reg = mysql_fetch_array($sql))) break;
	 	$version = $reg['version'];
	 	$vlog = $reg['versionlog'];
	 	$time = getTime($reg['time']);

	} while (true);

	include '../components/downloadLink.php';
	include '../components/foot.php';

	?>