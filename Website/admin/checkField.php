<script>
function checkField(field)
{
  var r = true;
  var valid = "ABCDEFGHIJKLMNOPQRSTUVWXYZÃÁÉÍÓÚÜÊÔÇçabcdefghijklmnopqrstuvwxyzáéíóúãêôõÕü-_@0123456789. ";
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