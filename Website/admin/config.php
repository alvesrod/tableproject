<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
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
	<tr>
	    <td bgcolor="#858380" height="94"></td>
		<td bgcolor="#858380" valign="top" height="94">
		<h3 class="tipTitle"><font color="#FF5050">Admin Warning!</font></h3>
		<font size="2" color="#FFFFFF">&nbsp;
		This page was made to setup the App Database.<br>
		&nbsp; If this is the first config, the password is "pass"</font><p>
						<b>
						<font size="3" color="#AAAAAA">Admin Access:&nbsp;</font>
						</b>
						</p>
						<form method="POST" action="config.php?a=true">
							<p><b><font color="#FFFFFF">Password:</font></b> <input type="password" name="p" size="20"> <input type="submit" value="Create Dabatase" name="B1">
							<font color="#FF0000">
							<?php
	  include 'dbLoginInfo.php';
	  if ( isset($_GET['a']) && ($_GET['a']==true) && isset($_POST['p']) )
	  {
	    if ($_POST['p']==$CPASSWORD)
	    {
	      session_start();
	      $_SESSION['sm']=true;
	      echo '<script> window.location="setDb.php" </script>';
	    }
	    else echo "<br>Wrong password.";
	  }
	?>
    </font></p></form></td><td bgcolor="#858380" height="94"></td>
	</tr><tr><td width="21"><img src="img/i3.gif"></td>
    <td bgcolor="#858380" align="right" style="font-size:0.75em;padding-right:15px;"></td>
    <td width="21"><img src="img/i4.gif"></td></tr></table>
	</div><p></div>

	</body>
	</html>