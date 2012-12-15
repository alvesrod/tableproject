<script type="text/javascript" src="../jscripts/tiny_mce/tiny_mce.js"></script>
<script type="text/javascript">
	tinyMCE.init({
		// General options
		mode : "textareas",
		theme : "advanced",
		plugins : "autolink,lists,pagebreak,style,advhr,advimage,advlink,iespell,inlinepopups,preview,media,print,contextmenu,paste,directionality,fullscreen,noneditable,visualchars,nonbreaking,xhtmlxtras,template,wordcount,advlist,visualblocks",

		// Theme options
		theme_advanced_buttons1 : "bold,italic,underline,strikethrough,|,justifyleft,justifycenter,justifyright,justifyfull,styleselect,formatselect,fontselect,fontsizeselect",
		theme_advanced_buttons2 : "bullist,numlist,|,undo,redo,|,link,unlink,|,preview,|,forecolor,backcolor,|,sub,sup,|,charmap,iespell,media,advhr,|,fullscreen",
		theme_advanced_toolbar_location : "top",
		theme_advanced_toolbar_align : "left",
		theme_advanced_statusbar_location : "bottom",
		theme_advanced_resizing : true,
	});

  	  function formcheck(form)
      {
         		document.MM_returnValue = false;
          	 	if ( (form.elements[2].value=="") || (form.elements[2].length < 2) )
          			alert("You need to type the version number.");
            	else
            		if (!checkField(form.elements[2]))
                  		alert("Invalid characters on the version number.");
            		else
            			document.MM_returnValue = true;    
     }

  function checkField(field)
  {
    var r = true;
    var valid = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_.";
    for(i=0;i<field.value.length;i++)
    {
      c = field.value.substring(i,i+1);
      if (valid.indexOf(c)==-1)
      {
        r = false;
        break;
      }
    }
    return r;
  }
</script>