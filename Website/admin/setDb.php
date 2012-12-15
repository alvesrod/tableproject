<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<?php

include 'dbLoginInfo.php';    

  session_start();
  if ((!isset($_SESSION['sm']))||($_SESSION['sm']==false)) echo '<script> href.location="../index.php" </script>';
  else {
  if (isset($_GET['access']))
  {
    $_SESSION['name_host'] = addslashes(htmlspecialchars($_POST['T1']));
    $_SESSION['name_login'] = addslashes(htmlspecialchars($_POST['T2']));
    $_SESSION['name_pass'] = addslashes(htmlspecialchars($_POST['T3']));
    $_SESSION['name_db'] = addslashes(htmlspecialchars($_POST['T4']));
    $_SESSION['room_days'] = addslashes(htmlspecialchars($_POST['T5']));
    $_SESSION['config_pass'] = addslashes(htmlspecialchars($_POST['T6']));
    $_SESSION['app_secret'] = addslashes(htmlspecialchars($_POST['T7']));
    $_SESSION['app_name'] = addslashes(htmlspecialchars($_POST['T8']));
  }
?>
<html>
	<?php include 'checkField.php'; ?>
	<script>
	  function formcheck(form)
      {
        var ok = true;
        document.MM_returnValue = false;
        for(var i=0;i<form.elements.length-1;i++)
        {
          if ((form.elements[i].value=="")&&(i!=2))
          {
            alert("Field "+(i+1)+" is empty.");
            ok  = false;
            break;
          }
          if (!checkField(form.elements[i]))
          {
            alert("Invalid characters on field "+(i+1));
            ok  = false;
            break;
          }
        }
        if (ok) {document.MM_returnValue = true;}
      }
	</script>

	<body bgcolor="#000000">
	<td valign="top">
    <img src="img/s.gif" class="vspacer" height="15">
    <img border="0" src="img/i5.png"><div style="padding-left:20px;padding-right:20px;">
	<h2 align="center"><font color="#C0C0C0">Engine Setup</font></h2>
	<div style="padding: 5px;" align="center" valign="middle">
	<table cellspacing="0" cellpadding="0" border="0" width="497">
	<tr></td><td width="21"><img src="img/i1.gif"></td>
	    <td bgcolor="#858380" align="right"></td>
	    <td width="21"><img src="img/i2.gif"></td>
	    <td width="4" valign="middle" rowspan="3"></td>
    </tr>
	<tr><form method="POST" action="setDb.php?access=true" onsubmit="formcheck(this);return document.MM_returnValue;">
	    <td bgcolor="#858380" height="94"></td>
		<td bgcolor="#858380" valign="top" height="94">
		<font size="2" color="#FFFFFF">&nbsp;
       <table border="0" width="100%" id="table1" bgcolor="#333333"><tr>
        <td width="145" align="right"><font color="#CCCCCC">Host:</td>
        <td bgcolor="#666666">
        <input type="text" name="T1" size="26" value=<?php if (isset($_SESSION['name_host'])) echo $_SESSION['name_host']; else echo $DB_HOST_NAME; ?> ></td>
		</tr><tr>
		<td width="145" align="right"><font color="#CCCCCC">Database username:</td>
		<td bgcolor="#444444">
		<input type="text" name="T2" size="26" value= <?php if (isset($_SESSION['name_login'])) echo $_SESSION['name_login']; else echo $DB_HOST_USER; ?>></td>
		</tr><tr><td width="145" align="right"><font color="#CCCCCC">Database password:</td>
	    <td bgcolor="#666666"><input type="text" name="T3" size="26" value= <?php if (isset($_SESSION['name_pass'])) echo $_SESSION['name_pass']; else echo $DB_HOST_PASSWORD; ?>></td>
		</tr><tr><td width="145" align="right"><font color="#CCCCCC">Database Name:</td><td bgcolor="#444444">
	    <input type="text" name="T4" size="26" value= <?php if (isset($_SESSION['name_db'])) echo $_SESSION['name_db']; else echo $DB_NAME; ?>></td>
		</tr><tr><td width="145" align="right"><font color="#CCCCCC">Room Days:</td><td bgcolor="#666666">
	    <input type="text" name="T5" size="26" value= <?php if (isset($_SESSION['room_days'])) echo $_SESSION['room_days']; else echo $ROOM_DAYS; ?>></td>
		</tr>
		<tr><td width="145" align="right"><font color="#CCCCCC">This config password:</td><td bgcolor="#444444">
	    <input type="text" name="T6" size="26" value= <?php if (isset($_SESSION['config_pass'])) echo $_SESSION['config_pass']; else echo $CPASSWORD; ?>></td>
		</tr>
		<tr><td width="145" align="right"><font color="#CCCCCC">App name:</td><td bgcolor="#666666">
	    <input type="text" name="T8" size="26" value= <?php if (isset($_SESSION['app_name'])) echo $_SESSION['app_name']; else echo $APP_NAME; ?>></td>
		</tr>
		<tr><td width="145" align="right"><font color="#CCCCCC">App secret:</td><td bgcolor="#666666">
	    <input type="text" name="T7" size="26" value= <?php if (isset($_SESSION['app_secret'])) echo $_SESSION['app_secret']; else echo $APP_SECRET; ?>></td>
		</tr>
		</table>
		<p><font color="#FFFFFF" size="1">*If the data below is not being saved, check if the user has permission to edit the dbLoginInfo.php file.</font></p>
		<font color="#330000" size="4">
				<?php
				if (isset($_GET['access']))
				{
				    $connection = mysql_connect($_SESSION['name_host'],$_SESSION['name_login'],$_SESSION['name_pass']);
                    		    if ($connection) echo '<script> window.location="installDb.php" </script>';
                    		    else echo "<b>Database error: </b>".mysql_error();
				}
                 ?>

				</font>
	    <p align="center"><input type="submit" value="Submit" name="B1"></p>
    </font></p></form></td><td bgcolor="#858380" height="94"></td>
	</tr><tr><td width="21"><img src="img/i3.gif"></td>
    <td bgcolor="#858380" align="right" style="font-size:0.75em;padding-right:15px;"></td>
    <td width="21"><img src="img/i4.gif"></td></tr></table>
	</div><p></div>
    <?php }
    ?>