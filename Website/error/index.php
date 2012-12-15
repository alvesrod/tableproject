<?php 
include '../components/head.php'; 
include '../components/top.php'; 
?>
		
		<li class="home"><a href="../">Menu</a></li>
		<li><a href="../files/">Files</a></li>

<?php include '../components/title.php'; ?>

		<li class="home"><a href="../">Home</a></li>
		<li class="about"><a href="../about/">What is this?</a></li>
		<li class="contact"><a href="../contact/">Contact</a></li>
</ul></div></div>
<div id="bodyPan"><div id="bodyleftPan">&nbsp;

  	<h2>Error</h2>
	<p class="greentext">The page could not be loaded. </p>
	<p class="greentext">Please, try again later.</p>
	<p class="greentext">&nbsp;</p>	
	<p class="greentext" align="center">&nbsp;</p>
	<p class="greentext" align="center">&nbsp;</p>
	<p class="greentext" align="center">&nbsp;</p>
	<p class="greentext" align="center">&nbsp;</p>
	<p class="greentext" align="center"></p>
	<p class="greentext">&nbsp;</p>
	<p class="greentext">&nbsp;</p><?php

    	if (isset($_GET['e'])) {
    		switch($_GET['e']) {
    			case 1:
    				echo "The website could not connect to the database.";
    				break;
    			case 2:
    				echo "The website could not get information from the Database.";
    				break;
    			case 3:
    				echo "The website could not get the latest news.";
    				break;
    			case 4:
    				echo "There are no news or versions in the database.";
    				break;
    			case 5:
    				echo "There were information missing in the last form.";
    				break;
    			case 6:
    				echo "At least 1 file should be uploaded.";
    				break;
    			case 7:
    				echo "The index file could not be generated.";
    				break;
    			case 8:
    				echo "The file uploaded was invalid.";
    				break;
    			case 9:
    				echo "The website could not zip the files uploaded.";
    				break;
    			case 10:
    				echo "There was an error when trying to upload a file.";
    				break;
    		}
    	}
    ?>

</div><div id="bodyrightPan"><div id="loginPan"><h1 align="center"><div id="errorPan"><h3>

The website failed!

</h3></div></h1><h1 align="center">&nbsp;</h1>
<p align="center">&nbsp;</div><div id="loginBottomPan">&nbsp;</div><br><p>


<?php 
include '../components/foot.php';
?>