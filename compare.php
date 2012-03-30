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
      exec($cmd, $result_array);
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

function jp2_properties($file) {
   $cmd = "/usr/bin/kdu_expand -record -quiet /dev/stdout  -i '$file'";
   $output = exec_help("kdu_expand -record '$file'", $cmd);
   $result = array();
   foreach(split("\n", $output) as $line) {
      list ($key, $value) = split("=", $line);
      if ($value) {
         $result[$key] = $value;
      }
   }
   return $result;
}

function jp2_profile($file, $profile) {
   $result = jp2_properties($file);
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
      exec("convert -resize 350x '$in' '$out'");
   }
}

function resize_bigger($in, $out) {
   if (!file_exists($out)) {
      exec("convert -resize 500x '$in' '$out'");
   }
}

function compare($file1, $file2) {
   global $config;
   $file1 = escapeshellcmd($config['destination'] . $file1);
   if ($file2 != "null") {
     $file2 = escapeshellcmd($config['destination'] . $file2);
   }
   $image1 = tempnam($config['tmp_dir'], "") . ".ppm";
   $image2 = tempnam($config['tmp_dir'], "") . ".ppm";
   $diff = tempnam($config['tmp_dir'], "") . ".ppm";
   $compare_bin = $config['compare_bin'];
   $cmd = "$compare_bin -1 '$image1' -2 '$image2' '$file1' '$file2' '$diff'";
   $result = exec_help("compare '$file1' '$file2'", $cmd);
   if ($result == "") {
      return false;
   }
   $xml = simplexml_load_string($result);
   $img1 = escapeshellcmd($config['destination'] . $xml->{"image1"}->{"checksum"} . ".png");
   $img2 = escapeshellcmd($config['destination'] . $xml->{"image2"}->{"checksum"} . ".png");
   $img_diff = escapeshellcmd($config['destination'] . $xml->{"image1"}->{"checksum"} . "_" . $xml->{"image2"}->{"checksum"} . ".png");
   $img1_big = escapeshellcmd($config['destination'] . $xml->{"image1"}->{"checksum"} . "_big.png");
   $img2_big = escapeshellcmd($config['destination'] . $xml->{"image2"}->{"checksum"} . "_big.png");
   $img_diff_big = escapeshellcmd($config['destination'] . $xml->{"image1"}->{"checksum"} . "_" . $xml->{"image2"}->{"checksum"} . "_big.png");
   if (!file_exists($img1)) {
      resize($image1, $img1);
      resize_bigger($image1, $img1_big);
      unlink($image1);
   }
   if (!file_exists($img2)) {
      resize($image2, $img2);
      resize_bigger($image2, $img2_big);
      unlink($image2);
   }
   if (!file_exists($img_diff)) {
      resize($diff, $img_diff);
      resize_bigger($diff, $img_diff_big);
      unlink($diff);
   }
   //jhove($xml->{"image1"}, $file1);
   //jhove($xml->{"image2"}, $file2);
   return $xml;
}

function jhove($file) {
   global $config;
   $file = escapeshellcmd($file);
   $java = $config['java'];
   $jhove = $config['jhove'];
   $jhove_conf = $config['jhove_conf'];
   $img_jhove = simplexml_load_string(exec_help("jhove '$file'", "$java -jar $jhove -h xml '$file' -c $jhove_conf"));
   $xml = simplexml_load_string("<jhove></jhove>");
   $xml->addChild("format", $img_jhove->{"repInfo"}->{"format"});
   $xml->addChild("status", $img_jhove->{"repInfo"}->{"status"});
   return $xml;
}

function validate($file) {
   global $config;
   $fits = $config['fits'];
   $xml = simplexml_load_string("<validation></validation>");
   $img_fits = exec_help("$fits '$file'", "$fits -i '$file'");
   $img_fits = substr($img_fits, strpos($img_fits, "<?xml version"));
   $img_fits = simplexml_load_string(str_replace('xmlns=', 'ns=', $img_fits));
   $img_fits->registerXPathNamespace('fits', 'http://hul.harvard.edu/ois/xml/ns/fits/fits_output');
   // Identification
   $node = $xml->addChild("result");
   $node->addAttribute("type", "Identification");
   $format = $img_fits->xpath("//identification/identity[last()]/@format");
   if (strpos($format[0], "Usage") !== false) {
      $format = $img_fits->xpath("//identification/identity/@format");
   }
   $node->addChild("format", $format[0])->addAttribute("tool", "jhove");
   // Validation
   $node = $xml->addChild("result");
   $node->addAttribute("type", "Validation");
   $formed = $img_fits->xpath("//filestatus/well-formed/text()");
   $valid = $img_fits->xpath("//filestatus/valid/text()");
   $status_str = ($formed[0] == "true")?"Well formed":"Bad formed";
   $status_str .= ($valid[0] == "true")?" and valid":" and invalid";
   $node->addChild("status", $status_str)->addAttribute("tool", "jhove");
   // Characterization
   $node = $xml->addChild("result");
   $node->addAttribute("type", "Characterization");
   $width = $img_fits->xpath("//metadata/image/imageWidth");
   $height = $img_fits->xpath("//metadata/image/imageHeight");
   $node->addChild("width", (string) $width[0])->addAttribute("tool", "Jhove");
   $node->addChild("width", (string) $width[0])->addAttribute("tool", "exif");
   $node->addChild("height", (string) $height[0])->addAttribute("tool", "Jhove");
   $node->addChild("height", (string) $height[0])->addAttribute("tool", "exif");
   foreach($config["extract"] as $key) {
      $values = $img_fits->xpath("//metadata/image/$key");
      if ($values) {
         foreach($values as $value) {
            $new_node = $node->addChild($key, (string) $value);
            $new_node->addAttribute("tool", $value->attributes()->{'toolname'});
            $new_node->addAttribute("status", $value->attributes()->{'status'});
         }
      }
   }
   if ($format == "JP2 EXIF" || $format == "JPEG 2000 JP2" || $format == "JPEG 2000") {
      $result = jp2_properties($file);
      foreach($result as $key => $value) {
         $node->addChild($key, (string) $value)->addAttribute("tool", "kdu_expand");
      }
   }
   $result = jp2_properties($file);
   foreach($result as $key => $value) {
      $node->addChild($key, (string) $value)->addAttribute("tool", "kdu");
   }
   if ($format[0] == "DJVU" || $format[0] == "DJVU (multi-page)" || $format[0] == "jhove DJVU") {
      $djvu_dump = exec_help("djvudump '$file'", "djvudump '$file'");
      $node = $xml->addChild("djvu_dump", htmlentities($djvu_dump));
   }
   return $xml;
}

$files = $_GET['file'];
$op = $_GET['op'];
$action = $_GET['action'];
if ((count($files) == 2 || count($files) == 1) && ($action == "compare" || $action == "execute")) {
   if (count($files) == 2) {
      $xml = compare($files[0], $files[1]);
   } else {
      $xml = compare($files[0], "null");
   }
   $smarty = smarty_init();
   if ($xml == false || $xml->{"error"}) {
      $smarty->assign("error", ($xml == false)?"Error processing image":$xml->{"error"});
      $smarty->assign('template', 'error.tpl');
      $smarty->assign('selected', 'compare');
      $smarty->display('layout.tpl');
      exit(1);
   }
   $images = array();
   $count = 0;
   foreach((count($files)==2)?array("image1", "image2"):array("image1") as $image) {
      $data = array();
      $data["thumbnail"] = "images/" . $xml->{$image}->{"checksum"} . ".png";
      $data["big"] = "images/" . $xml->{$image}->{"checksum"} . "_big.png";
      $data["hash"] = $xml->{$image}->{"checksum"};
      $data["height"] = $xml->{$image}->{"height"};
      $data["width"] = $xml->{$image}->{"width"};
      $data["format"] = $xml->{$image}->{"format"};
      $data["name"] = $files[$count];
      $images[] = $data;
      $count++;
   }
   $smarty->assign("images", $images);
   $smarty->assign("compare", (count($files)==2)?true:false);
   $diff = array();
   $psnr = array();
   $psnr["avg"] = $xml->{"psnr"}->attributes()->{"avg"};
   $psnr["r"] = $xml->{"psnr"}->attributes()->{"r"};
   $psnr["g"] = $xml->{"psnr"}->attributes()->{"g"};
   $psnr["b"] = $xml->{"psnr"}->attributes()->{"b"};
   $diff["psnr"] = $psnr;
   $diff["thumbnail"] = "images/" . $xml->{"image1"}->{"checksum"} . "_" . $xml->{"image2"}->{"checksum"} . ".png";
   $diff["big"] = "images/" . $xml->{"image1"}->{"checksum"} . "_" . $xml->{"image2"}->{"checksum"} . "_big.png";
   if (((string) $xml->{"image1"}->{"checksum"}) == ((string) $xml->{"image2"}->{"checksum"})) {
      $diff["equal"] = "yes";
   } else {
      $diff["equal"] = "no";
   }
   $histogram = array();
   foreach(array("histogram", "histogram1", "histogram2") as $hist) {
      foreach (array("r", "g", "b") as $col) {
         $result = array();
         foreach($xml->{$hist}->{"rgb"} as $rgb) {
            $level = (string) $rgb->attributes()->{"level"};
            $pixs = (string) $rgb->attributes()->{$col};
            $result[$level] = $pixs;
         }
         $histogram[$col] = $result;
      }
      $diff[$hist] = $histogram;
   }
   $smarty->assign("diff", $diff);
   foreach(array("", "1", "2") as $cvs_id) {
      $cvs_export = array();
      foreach($xml->{"histogram" . $cvs_id }->{"rgb"} as $rgb) {
         $add = array();
         $add['r'] = $rgb->attributes()->{"r"};
         $add['g'] = $rgb->attributes()->{"g"};
         $add['b'] = $rgb->attributes()->{"b"};
         $cvs_export[] = $add;
      }
      $smarty->assign("cvs_export" . $cvs_id, $cvs_export);
   }
   $profiles = array();
   foreach (get_files_in_directory($config["profiles"], "xml") as $profile_file) {
      $profiles[] = $profile_file;
   }
   $smarty->assign("cputime", (string) $xml->{"cputime"});
   $smarty->assign("profiles", $profiles);
   $smarty->assign('template', 'result.tpl');
   $smarty->assign('selected', 'compare');
   $smarty->display('layout.tpl');
   // $smarty->display('result.tpl');
} else if ($op == "jhove" &&  count($files) == 1) {
   $file1 = escapeshellcmd($config['destination'] . $files[0]);
   header("content-type: text/xml");
   $result = jhove($file1);
   //header("content-type: text/xml");
   print $result->asXML();
} else if ($op == "validate" && count($files) == 1) {
   $file1 = escapeshellcmd($config['destination'] . $files[0]);
   header("content-type: text/xml");
   $result = validate($file1);
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

