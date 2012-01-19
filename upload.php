<?php
error_reporting(E_ALL);
ini_set("display_errors", 1); 
include("config.php");

$config = config();
$filename = basename($_FILES['uploadedfile']['name']);
$path_info = pathinfo($filename);
if (!file_exists($filename)) {
  header("Location: index.php?success=false&message=no+file+specified");
}
$extension = strtolower($path_info["extension"]);
$destination = $config["destination"] . $filename;
$destination_profiles = $config["profiles"] . $filename;

if ($extension == "xml" && move_uploaded_file($_FILES['uploadedfile']['tmp_name'], $destination_profiles)) {
  header("Location: index.php?success=true");
} else if (!in_array($extension, $config["extensions"])) {
  header("Location: index.php?success=false&message=unsupported+filetype");
} else if (move_uploaded_file($_FILES['uploadedfile']['tmp_name'], $destination)) {
  header("Location: index.php?success=true");
} else {
  header("Location: index.php?success=false&message=upload+failed");
}

?>
