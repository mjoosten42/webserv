#!/usr/bin/php
<?php
	print("Content-Type: ");
	// exit;
	print("text/plain\n");
	// exit;
	print("\n");

	print("Current dir: " . getcwd());
	phpinfo(INFO_ENVIRONMENT);
?>
