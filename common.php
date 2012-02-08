<?php
// Smarty
require_once("Smarty.class.php");
require_once("config.php");

function smarty_init() {
  $config = config();
  $smarty = new Smarty();
  $smarty->template_dir = $config['application_dir'] . '/templates/';
  $smarty->config_dir   = $config['application_dir'] . '/configs/';
  $smarty->compile_dir  = $config['tmp_dir'] . '/smarty/templates_c/';
  $smarty->cache_dir    = $config['tmp_dir'] . '/smarty/cache/';
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
