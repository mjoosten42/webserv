#!/usr/bin/php
<?php

sleep(1);

echo "Content-Type: text/plain\n";
echo "\n";

print_r($_ENV);
print_r($_FILES);
print_r($_POST);
print_r($_GET);

$uploaddir = "uploads/";
$uploadfile = $uploaddir . basename($_FILES['userfile']['name']);

echo "Upload name: ($uploadfile) \n";
echo "Info: " . ($_FILES['userfile']['tmp_name']) . "\n";

if (move_uploaded_file($_FILES['userfile']['tmp_name'], $uploadfile)) {
    echo "File is valid, and was successfully uploaded.\n";
} else {
    echo "Possible file upload attack!\n";
}

?>
