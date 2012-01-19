<?php

require("config.php");
require("common.php");
$config = config();

function exec_help($desc, $cmd) {
   global $config;
   $md5 = md5($desc);
   $output_file = $config['output'] . $md5 . ".xml";
   if (file_exists($output_file)) {
      return file_get_contents($output_file);
   } else {
      $result_array = array();
      exec($cmd, &$result_array);
      $result = "";
      foreach($result_array as $line) {
         $result .= $line . "\n";
      }
      $fh = fopen($output_file, 'cw');
      fwrite($fh, $result);
      fclose($fh);
      return $result;
   }
}

function jp2_profile($file, $profile) {
   $cmd = "/usr/bin/kdu_expand -record -quiet /dev/stdout  -i '$file'";
   $output = exec_help("kdu_expand -record '$file'", $cmd);
   $result = array();
   foreach(split("\n", $output) as $line) {
      list ($key, $value) = split("=", $line);
      if ($value) {
         $result[$key] = $value;
      }
   }
   $xml = simplexml_load_string(file_get_contents($profile));
   $valid = true;
   $result_xml = simplexml_load_string("<profile></profile>");
   foreach($xml->{'metadata'} as $entry) {
      $key = (string) $entry->attributes()->{"key"};
      $value = $entry;
      $valid = ($result[$key] == $value);
      $element = $result_xml->addChild("metadata", $img_jhove->{"repInfo"}->{"format"});
      $element->addAttribute("key", $key);
      $element->addAttribute("expected", $value);
      $element->addAttribute("value", (string) $result[$key]);
      $element->addAttribute("valid", $valid);
   }
   $name = (string) $xml->{'name'};
   // $result[$name] = $valid;
   return $result_xml;
}

function resize($in, $out) {
   if (!file_exists($out)) {
      exec("convert -resize 128x '$in' '$out'");
   }
}

function compare($file1, $file2) {
   global $config;
   $file1 = escapeshellcmd($config['destination'] . $file1);
   $file2 = escapeshellcmd($config['destination'] . $file2);
   $image1 = tempnam($config['tmp_dir'], "") . ".ppm";
   $image2 = tempnam($config['tmp_dir'], "") . ".ppm";
   $diff = tempnam($config['tmp_dir'], "") . ".ppm";
   $cmd = "/home/xrosecky/projects/compare/compare -1 '$image1' -2 '$image2' '$file1' '$file2' '$diff'";
   $result = exec_help("compare '$file1' '$file2'", $cmd);
   $xml = simplexml_load_string($result);
   $img1 = escapeshellcmd($config['destination'] . $xml->{"image1"}->{"checksum"} . ".png");
   $img2 = escapeshellcmd($config['destination'] . $xml->{"image2"}->{"checksum"} . ".png");
   $img_diff = escapeshellcmd($config['destination'] . $xml->{"image1"}->{"checksum"} . "_" . $xml->{"image2"}->{"checksum"} . ".png");
   resize($image1, $img1);
   resize($image2, $img2);
   resize($diff, $img_diff);
   jhove($xml->{"image1"}, $file1);
   jhove($xml->{"image2"}, $file2);
   return $xml;
}

function jhove($file) {
   $file = escapeshellcmd($file);
   $img_jhove = simplexml_load_string(exec_help("jhove '$file'", "/opt/jdk1.6.0_20/bin/java -jar /home/xrosecky/jhove/bin/JhoveApp.jar -h xml '$file' -c /home/xrosecky/jhove/conf/jhove.conf"));
   $xml = simplexml_load_string("<jhove></jhove>");
   $xml->addChild("format", $img_jhove->{"repInfo"}->{"format"});
   $xml->addChild("status", $img_jhove->{"repInfo"}->{"status"});
   return $xml;
}

$files = $_GET['file'];
$op = $_GET['op'];
if (count($files) == 2) {
   $xml = compare($files[0], $files[1]);
   $smarty = smarty_init();
   $images = array();
   $count = 0;
   foreach(array("image1", "image2") as $image) {
      $data = array();
      $data["thumbnail"] = "images/" . $xml->{$image}->{"checksum"} . ".png";
      $data["hash"] = $xml->{$image}->{"checksum"};
      $data["height"] = $xml->{$image}->{"height"};
      $data["width"] = $xml->{$image}->{"width"};
      $data["format"] = $xml->{$image}->{"format"};
      $data["name"] = $files[$count];
      $images[] = $data;
      $count++;
   }
   $smarty->assign("images", $images);
   $diff = array();
   $psnr = array();
   $psnr["r"] = $xml->{"psnr"}->attributes()->{"r"};
   $psnr["g"] = $xml->{"psnr"}->attributes()->{"g"};
   $psnr["b"] = $xml->{"psnr"}->attributes()->{"b"};
   $diff["psnr"] = $psnr;
   $diff["thumbnail"] = "images/" . $xml->{"image1"}->{"checksum"} . "_" . $xml->{"image2"}->{"checksum"} . ".png";
   $histogram = array();
   foreach (array("r", "g", "b") as $col) {
      $result = array();
      foreach($xml->{"histogram"}->{"rgb"} as $rgb) {
         $level = (string) $rgb->attributes()->{"level"};
         $pixs = (string) $rgb->attributes()->{$col};
         $result[$level] = $pixs;
      }
      $histogram[$col] = $result; 
   }
   $diff["histogram"] = $histogram;
   $smarty->assign("diff", $diff);
   $profiles = array();
   foreach (get_files_in_directory($config["profiles"], "xml") as $profile_file) {
      $profiles[] = $profile_file;
   }
   $smarty->assign("profiles", $profiles);
   $smarty->display('result2.tpl');
} else if ($op == "jhove" &&  count($files) == 1) {
   $file1 = escapeshellcmd($config['destination'] . $files[0]);
   header("content-type: text/xml");
   $result = jhove($file1);
   header("content-type: text/xml");
   print $result->asXML();
} else if ($op == "jp2_profile" &&  count($files) == 1) {
   $file1 = escapeshellcmd($config['destination'] . $files[0]);
   $profile = $config['profiles'] . $_GET["profile"];
   header("content-type: text/xml");
   $result = jp2_profile($file1, $profile);
   header("content-type: text/xml");
   print $result->asXML();
} else {
   print "failed<BR>";
}

/*
header("content-type: text/xml");
print $xml->asXML();
unlink($image1);
unlink($image2);
unlink($diff);
*/

?>

