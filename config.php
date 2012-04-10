<?php

function config() {
  $config = array(
    "destination" => "/home/differ/production/images/",
    "output" => "/home/differ/production/output/",
    "profiles" => "/home/differ/production/profiles/",
    "java" => "/opt/jdk1.6.0_20/bin/java",
    "jhove" => "/home/differ/jhove/bin/JhoveApp.jar",
    "jhove_conf" => "/home/differ/jhove/conf/jhove.conf",
    "fits" => "/home/differ/fits/fits.sh",
    "application_dir" => "/home/differ/production/",
    "extensions" => array("jpg", "jpeg", "tiff", "tif", "jp2", "djvu", "jpx", "jpf", "TIF", "TIFF"),
    "tmp_dir" => "/tmp/",
    "compare_bin" => "/home/differ/projects/compare/compare.sh",
    "extract" => array("bitsPerSample", "xSamplingFrequency", "ySamplingFrequency", "iccProfileName",
       "colorSpace", "compressionScheme", "exifVersion"),
  );
  return $config;
}

?>
