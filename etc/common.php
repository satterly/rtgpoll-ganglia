<?
 /* Begin with reasonable defaults */
 $VERSION = "0.7.4";
 $host="localhost";
 $user="snmp";
 $pass="rtgdefault";
 $db="rtg";
 $refresh=300;
 
 /* Default locations to find RTG configuration file */
 $configs[] = 'rtg.conf';
 $configs[] = '/usr/local/rtg/etc/rtg.conf';
 $configs[] = '/usr/local/rtg/etc/rtg.conf';
 $configs[] = '/etc/rtg.conf';

 while ($conf = each($configs)) {
   $fp = @fopen($conf['value'], "r");
   if ($fp) {
     while (!feof($fp)) {
       $line = fgets($fp, 4096);
       if (!feof($fp) && $line[0] != '#' && $line[0] != ' ' && $line[0] != '\n') {
         $cVals = preg_split("/[\s]+/", $line, 2);
         if (!strcasecmp($cVals[0], "DB_Host")) $host = chop($cVals[1]);
         else if (!strcasecmp($cVals[0], "DB_User")) $user = chop($cVals[1]);
         else if (!strcasecmp($cVals[0], "DB_Pass")) $pass = chop($cVals[1]);
         else if (!strcasecmp($cVals[0], "DB_Database")) $db = chop($cVals[1]);
       }
     }
     break;
   }
 }
?>
