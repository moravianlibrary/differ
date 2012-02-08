<?php
require("config.php");
require("common.php");

$config = config();

function sort_by_date($obj1, $obj2) {
   return ($obj1["modified"] < $obj2["modified"]);
}

function main() {
  global $config;
  $smarty = smarty_init();
  $files = array();
  if ($handle = opendir($config['destination'])) {
    while (false !== ($file = readdir($handle))) {
      if ($file != "." && $file != ".." && (substr($file, -strlen(".png")) != ".png")) {
        $modified = filemtime($config['destination'] . $file);
        $filesize = round(filesize($config['destination'] . $file) / (1024*1024), 2);
        $pathinfo = pathinfo($config['destination'] . $file);
        $files[] = array("name" => $file, "modified" => date ("Y-m-d H:i:s", $modified),
          "size" => $filesize, "ext" => strtolower($pathinfo['extension']));
        // $files[] = $file;
      }
    }
  } else {
    $smarty->assign("error", "error");
  }
  uasort($files, "sort_by_date");
  $smarty->assign("files", $files);
  $profiles = array();
  if ($handle = opendir($config['profiles'])) {
    while (false !== ($file = readdir($handle))) {
      if ($file != "." && $file != "..") {
        $modified = filemtime($config['profiles'] . $file);
        $profiles[] = array("name" => $file, "modified" => date ("Y-m-d H:i:s", $modified));
        // $files[] = $file;
      }
    }
  } else {
    $smarty->assign("error", "error");
  }
  $smarty->assign("profiles", $profiles);
  $success = $_GET["success"];
  if ($success == "false") {
    $smarty->assign("error", $_GET["message"]);
  }
  // $smarty->display('index.tpl');
  $smarty->assign('selected', 'files');
  $smarty->assign('template', 'index.tpl');
  $smarty->display('layout.tpl');
}

function delete_all_files() {
   global $config;
   $dir = escapeshellcmd($config['destination']);
   $result = array();
   shell_exec("rm $dir/*.jp2");
   shell_exec("rm $dir/*.jpg");
   shell_exec("rm $dir/*.jpf");
   shell_exec("rm $dir/*.jpx");
   shell_exec("rm $dir/*.jpeg");
   shell_exec("rm $dir/*.tiff");
   shell_exec("rm $dir/*.tif");
   shell_exec("rm $dir/*.JP2");
   shell_exec("rm $dir/*.JPG");
   shell_exec("rm $dir/*.JPEG");
   shell_exec("rm $dir/*.TIFF");
   shell_exec("rm $dir/*.TIF");
   shell_exec("rm $dir/*.JPF");
   shell_exec("rm $dir/*.JPX");
   shell_exec("rm $dir/*.djvu");
   shell_exec("rm $dir/*.DJVU");
   header("Location: index.php");
}

function delete_images($files) {
   global $config;
   $dir = escapeshellcmd($config['destination']);
   foreach($files as $file) {
      remove_file($dir, $file);
   }
}

function delete_profiles($files) {
   global $config;
   $dir = escapeshellcmd($config['profiles']);
   foreach($files as $file) {
      remove_file($dir, $file);
   }
}

function remove_file($dir, $file) {
   $path = realpath($dir . "/" . $file);
   if (strpos($path, $dir) === 0) {
      unlink($path);
   }
}

function delete_all_profiles() {
   global $config;
   $dir = escapeshellcmd($config['profiles']);
   $result = array();
   shell_exec("rm $dir/*.xml");
   header("Location: index.php");
}

$action = $_REQUEST["action"];
if ($action == "compare" || $action == "EXECUTE") {
   header("Location: compare.php?" . $_SERVER['QUERY_STRING']);
} else if ($action == "delete_all_files") {
   delete_all_files();
} else if ($action == "delete_all_profiles") {
   delete_all_profiles();
} else if ($action == "delete_images" && isset($_REQUEST["file"])) {
   delete_images($_REQUEST["file"]);
   header("Location: index.php");
} else if ($action == "delete_profiles" && isset($_REQUEST["profile"])) {
   delete_profiles($_REQUEST["profile"]);
   header("Location: index.php");
} else {
   main();
}

?>
