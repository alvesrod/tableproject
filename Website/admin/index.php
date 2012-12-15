<?php 
include '../components/head.php';   
include '../components/top.php';
include 'admin.php';
include 'adminscript.php'; 

$loadPage = false;
$connection = @mysql_connect($DB_HOST_NAME,$DB_HOST_USER,$DB_HOST_PASSWORD);
$latestVersion = "Undefined";
if ($connection)
{
	if (@mysql_select_db($DB_NAME)) {
		$sql = @mysql_query("SELECT * FROM updates ORDER BY time DESC LIMIT 0,1");
		if ($sql)
		{
		  if (@mysql_num_rows($sql)==1) {
		    $reg = mysql_fetch_array($sql);
		    $version = $reg['version'];
		    $latestVersion = $version;
		  }

		}
	}
}
?>

		<li class="home"><a href="../">Menu</a></li>
		<li><a href="../files/">Files</a></li>

<?php include '../components/title.php'; ?>

		<li class="home"><a href="../">Home</a></li>
		<li class="about"><a href="../about/">What is this?</a></li>
		<li class="contact"><a href="../contact/">Contact</a></li>

</ul></div></div><div id="bodyPan"><div id="bodyleftPan"><br><br><h2>

Update

</h2>

<p class="greentext">
	
	
    <?php

    if ($displaySuccess) {
    	echo "Version and news updated!<br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>";
     } else echo "Please, fill the form below:";
     
    if ($displayForm) { //Continue only if admin.php allows.
  ?>
  </p><br>
  	<form method="POST" action="index.php?s=1" onsubmit="formcheck(this);return document.MM_returnValue;" enctype="multipart/form-data">
  	<table border="0" width="121%" id="table1">
  		<tr>
          		<td width="145" align="right">&nbsp;</td>
  			<td>
  			<font color="#CCCCCC">News:</td>
  		</tr>
  		
  		<tr>
          		<td width="145" align="right">&nbsp;</td>
  			<td>
          			<textarea id="elm1" name="T1" rows="10">
						<?php if (isset($_SESSION['news'])) echo $_SESSION['news']; ?>
					</textarea>
  			</td>
  		</tr>
  
  		<tr>
          		<td width="145" align="right">&nbsp;</td>
  			<td>
  			&nbsp;</td>
  		</tr>
		<tr>
          		<td width="145" align="right">&nbsp;</td>
  			<td>
  			<font color="#CCCCCC">Version changes:</td>
  		</tr>
  
  		<tr>
  			<td width="145" align="right">&nbsp;</td>
  	    		<td>
  	    		    <textarea id="elm2" name="T4" rows="10">
						<?php if (isset($_SESSION['vlog'])) echo $_SESSION['vlog']; ?>
					</textarea>
  			</td>
  		</tr>

  		<tr>
          		<td width="145" align="right">&nbsp;</td>
  			<td>
  			&nbsp;</td>
  		</tr>

  		<tr>
  			<td width="145" align="right"><font color="#CCCCCC">Version:</td>
  			<td>
  				<input type="text" name="T2" size="32" value= <?php if (isset($_SESSION['app_version'])) echo $_SESSION['app_version']; ?>>
  			</td>
  		</tr>
  		<tr>
  			<td width="145" align="right"><font color="#CCCCCC">Files:</td>
  	    		<td>
  				<input type="file" name="filesToUpload[]" multiple="" min="1" max="300" value="">
  			</td>
  		</tr>
  		
  		<tr>
          		<td width="145" align="right">&nbsp;</td>
  			<td>
  			      <input type="checkbox" name="erasebox" value="1" <?php if ( (isset($_SESSION['erasebox'])) && ($_SESSION['erasebox']) ) echo "CHECKED"; ?>>Erase already uploaded files.
  			<input type="submit" value="Submit" name="B1" style="float: right"></td>
  		</tr>
  		
  		<tr>
  		<td width="145" align="right">&nbsp;</td>
  		<td></td>
  		

		
		
  	</table>
  	</form><br><a href="config.php">Setup Database</a>
	
	<?php } 

	include '../components/downloadLink.php';
	include '../components/foot.php';

	?>
