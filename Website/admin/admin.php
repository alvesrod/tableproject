<?php

  /* TYPES OF FILE ALLOWED TO UPLOAD: */
  $allowedExts = array("jpg", "jpeg", "gif", "png", "txt", "exe", "dll", "zip", "pdf", "rar", "app", "so", "dylib");
  
  $UPLOAD_FOLDER = "download";
  include 'dbLoginInfo.php';
  include '../components/sqlEntriesTotal.php';

  function removedir($dirname)
      {
          if (is_dir($dirname))
          	$dir_handle = opendir($dirname);
	  else return true;
          if (!$dir_handle)
          	return false;
          while($file = readdir($dir_handle)) {
              if ($file != "." && $file != "..") {
                  if (!is_dir($dirname."/".$file))
                  unlink($dirname."/".$file);
                  else
                  {
                      $a=$dirname.'/'.$file;
                      removedir($a);
                  }
              }
          }
          closedir($dir_handle);
          rmdir($dirname);
          return true;
      }

  function zipFilesAndStore($files,$name,$path)
  {
    $zip = new ZipArchive();
    $archive = $path.$name;
    if (!file_exists($archive))
        $result = $zip->open ($archive, ZipArchive::CREATE);
    else
        $result = $zip->open ($archive);
  	if ($result == false) {
      		return false;
    }
  	foreach($files as $filename) {
    		$zip->addFile($path.$filename,$filename);
    }
  	$zip->close();
  	return file_exists($archive);
  }


  session_start();
  $displayForm = false;
  $displaySuccess = false;
  $connection = @mysql_connect($DB_HOST_NAME,$DB_HOST_USER,$DB_HOST_PASSWORD);
  if (!$connection) echo "<script>window.location='config.php';</script>";
  else {
  	if (!@mysql_select_db($DB_NAME)) echo "<script>window.location='config.php';</script>";
  	else {
    	   if (isset($_GET['s']))
    	   {
	      if ( (!isset($_POST['T1'])) || (!isset($_POST['T2'])) || (!isset($_POST['T4'])) )
		 echo "<script>window.location='../error/?e=5';</script>";
	      else {
     	      $_SESSION['news'] = addslashes($_POST['T1']);
       	      $_SESSION['app_version'] = addslashes(htmlspecialchars($_POST['T2']));
              $_SESSION['erasebox'] = ( (isset($_POST['erasebox'])) && (($_POST['erasebox']) == "1") );
    	      $_SESSION['vlog'] = addslashes($_POST['T4']);

    	      $n = $_SESSION['news'];
    	      $a = $_SESSION['app_version'];
    	      $l = $_SESSION['vlog'];
              $path = "../" . $UPLOAD_FOLDER . "/" . $a . "/";
              $filename = $path."index.txt";

    	      $fileCount = count($_FILES["filesToUpload"]["name"]);
    	      if ( ($fileCount == 0) && (!file_exists($filename))  ) echo "<script>window.location='../error/?e=6';</script>";
  	       else {
  	       		  $justUpdate = false;
                  if ( ($fileCount == 1) && (file_exists($filename)) && (end(explode(".", $_FILES["filesToUpload"]["name"][0])) == "") && (!$_SESSION['erasebox']) )
                      $justUpdate = true;
		
                  $success = true;
                  if ($_SESSION['erasebox'])
    	        	    removedir($path);

		  @mkdir($path);

		  if (!file_exists($filename))
                            $fp = @fopen($filename,w);
                        else
                            $fp = @fopen($filename,a);


  		        if (!$fp) echo "<script>window.location='../error/?e=7';</script>";
  		        else {
                      if (!$justUpdate)
      		            for ($i = 0; $i < $fileCount; $i++) {
      		                $extension = end(explode(".", $_FILES["filesToUpload"]["name"][$i]));
      		                if ( ($_FILES["filesToUpload"]["size"][$i] < 10000000000) && in_array($extension, $allowedExts) )
        		            {
        			                if ($_FILES["filesToUpload"]["error"][$i] > 0)
          			            {
      			                    $success = false;
      			                    echo "<script>window.location='../error/?e=10';</script>";
      			                    break;
          			            }
       			                else
          			            {

						$filepath = $path . "/" . $_FILES["filesToUpload"]["name"][$i];
						
						if ($extension == "zip") {
							$zip = new ZipArchive();
    						$result = $zip->open($_FILES["filesToUpload"]["tmp_name"][$i]);
						  	if ($result == true) {
						  		$files = array();
						  		unset($_FILES["filesToUpload"]["name"][$i]);
								for($i = 0; $i < $zip->numFiles; $i++) {
									$filename = $zip->getNameIndex($i);
									if (substr($filename, -1) == "/") continue; //Don't add folders
									array_push($_FILES["filesToUpload"]["name"],$filename);
									
									
									if (!file_exists($path . "/" .$filename)) {
										array_push($files,$filename);
									}
									
								}
								$zip->extractTo($path);
								foreach ($files as $f) {
									fputs($fp, $f."\n".filesize($path . "/" .$f)."\n");
								}
								$zip->close();
								
							} else $success = false;
							

						} else {
						
          			                if (!file_exists($filepath))
      			                        fputs($fp, $_FILES["filesToUpload"]["name"][$i]."\n".$_FILES["filesToUpload"]["size"][$i]."\n");
          			                move_uploaded_file($_FILES["filesToUpload"]["tmp_name"][$i], $filepath);
						}

        			            }
      		                }
  		                    else
    		                {
  			                    $success = false;
  			                    echo "<script>window.location='../error/?e=8'</script>";
  			                    break;
    		                }
  		                }
  	                    @fclose($fp);
    		            if ($success == true) {
				    $zipfilename = '_' . str_replace(' ','',$APP_NAME) . 'v'.$a.'.zip';
                                    if ( (!$justUpdate) && (!zipFilesAndStore($_FILES["filesToUpload"]["name"], $zipfilename ,$path)) )
                                        echo "<script>window.location='../error/?e=9';</script>";
            					    else {
          	      	        	      $sql = @mysql_query("INSERT INTO updates (description, version, versionlog) VALUES ('$n','$a', '$l') ON DUPLICATE KEY UPDATE time = NOW(), description = '$n', version = '$a', versionlog = '$l'");
             	      		          if (!$sql) echo "<script>window.location='../error/?e=3';</script>";
                                      else {

					//REMOVE OLD FILEs
					$sql = mysql_query("SELECT version FROM updates ORDER BY time DESC LIMIT $newsLimit,100");
					if ($sql) {
						while ($reg = mysql_fetch_array($sql)) {
    	     						$a = $reg['version'];
              						$path = "../" . $UPLOAD_FOLDER . "/" . $a . "/";
							if (removedir($path))
								@mysql_query("DELETE LOW_PRIORITY FROM updates WHERE version = '$a'");  
						}

					}
					                                      
					$displaySuccess = true;
                                        $news = $n;
  		    			$version = $a;
                                      }
                                    }
                        }
  		            }
  	            }
		}
    	   } else {
  	      $sql = @mysql_query("SELECT description, version, versionlog FROM updates ORDER BY time DESC LIMIT 0,1");
  	      if (!$sql) echo "<script>window.location='../error/?e=3';</script>";
  	      else {
  		  if (@mysql_num_rows($sql)==1) {
  		    $reg = mysql_fetch_array($sql);
  		    $_SESSION['news'] = $reg['description'];
  		    $news = $_SESSION['news'];
  		    $_SESSION['app_version'] = $reg['version'];
  		    $version = $_SESSION['app_version'];
  		    $_SESSION['vlog'] = $reg['versionlog'];
  		  } else {
  		    $_SESSION['news'] = "No news yet.";
  		    $news = $_SESSION['news'];
  		    $_SESSION['app_version'] = "0.0b";
  		    $version = $_SESSION['app_version'];
  		    $_SESSION['vlog'] = "No version log yet.";
  		  }
  	      }
          $displayForm = true;
    }}}
  ?>