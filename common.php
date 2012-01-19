<?php
// Smarty
require_once("Smarty.class.php");

function smarty_init() {
  $smarty = new Smarty();
  $smarty->template_dir = '/home/differ/test/templates/';
  $smarty->config_dir   = '/home/differ/test/configs/';
  $smarty->compile_dir  = '/tmp/smarty/templates_c/';
  $smarty->cache_dir    = '/tmp/smarty/cache/';
  return $smarty;
}

function get_files_in_directory($dir, $extension) {
   $result = array();
   if ($handle = opendir($dir)) {
      while (false !== ($file = readdir($handle))) {
        if ($file != "." && $file != ".." && (substr($file, -strlen($extension)) == $extension)) {
          $result[] = $file;
        }
      }
   }
   return $result;
}

function check_path($dir, $name) {
   $path = realpath($dir . "/" . $file);
   return (strpos($path, $dir) === 0);
}

?>
