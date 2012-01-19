<?php

function config() {
  $config = array(
    "destination" => "/home/differ/test/images/",
    "output" => "/home/differ/test/output/",
    "profiles" => "/home/differ/test/profiles/",
    "extensions" => array("jpg", "jpeg", "tiff", "tif", "jp2", "djvu", "jpx", "jpf"),
    "tmp_dir" => "/tmp/",
    "compare_bin" => "/home/xrosecky/projects/compare/compare.sh",
    "extract" => array("bitsPerSample", "xSamplingFrequency", "ySamplingFrequency", "iccProfileName",
       "colorSpace", "compressionScheme", "exifVersion"),
  );
  return $config;
}

?>
