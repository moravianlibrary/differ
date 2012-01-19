<?php

require("config.php");
require("common.php");
$config = config();

$smarty = smarty_init();

$parameters = array(
   "Clevels" => array("translate" => "Decomposition levels (Clevels)", "value" => "5"),
   "Clayers" => array("translate" => "Quality layers (Clayers)", "value" => "5"),
   "Corder" => array("translate" => "Progression order (Corder)", "value" => "RPCL"),
   "Creversible" => array("translate" => "Compression (Creversible)", "value" => "yes"),
   "Ckernels" => array("translate" => "Transformation (Ckernels)", "value" => "W9X7"),
   "Cblk" => array("translate" => "Code block size (Cblk)", "value" => "256x256"),
   "Cuse_precints" => array("translate" => "Regions of interests (Cuse_precints)", "value" => "true"),
   "Cuse_sop" => array("translate" => "Start of packet header (Cuse_sop)", "value" => "yes"),
   "Cuse_eph" => array("translate" => "End of packet header (Cuse_eph)", "value" => "yes"),
);

$op = $_REQUEST['op'];

if ($op == "create") {
   $name = $_REQUEST['name'];
   $out = "<profile>\n";
   $out .= "<name>$name</name>\n";
   foreach($parameters as $key => $value) {
      if ($_REQUEST[$key]) {
         $value=$_REQUEST[$key . "_value"];
         $out .= "<metadata key='$key'>$value</metadata>\n";
      }
   }
   $out .= "</profile>\n";
   if (preg_match("/[A-Za-z]/", $name)) {
      $file = $config['profiles'] . $name . ".xml";
      $output = fopen($file, "w");
      fwrite($output, $out);
      fclose($output);
      header("Location: index.php");
   }
} else {
   $smarty->assign('parameters', $parameters);
   $smarty->assign('template', 'profile.tpl');
   $smarty->assign('selected', 'profile');
   $smarty->display('layout.tpl');
}
