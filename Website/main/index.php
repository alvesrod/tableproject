<?php include '../components/header.php'; ?>

		<li class="company">Menu</li>
		<li><a href="../files/">Files</a></li>

<?php include '../components/title.php'; ?>

		<li class="home">Home</li>
		<li class="about"><a href="../about/">What is this?</a></li>
		<li class="contact"><a href="../contact/">Contact</a></li>

</ul></div></div><div id="bodyPan"><div id="bodyleftPan"><h2>


Latest News


</h2><p class="greentext"><?php echo $time; ?></p><p><?php echo $news; ?></p>
<ul><li class="more"><a href="../news/">


Previous news


</a></li></ul><?php 

include '../components/downloadLink.php';
include 'mainText.php';
include '../components/facebookComments.php';
include '../components/foot.php'; 

?>