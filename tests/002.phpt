--TEST--
Check for rlog_open()
--SKIPIF--
<?php
	extension_loaded('rlog') or die('skip rlog not available');
    $required_func = array("rlog_open");
	foreach ($required_func as $func_name) {
		if (!function_exists($func_name)) {
			die("skip $func_name() function is not available.");
		}
	}
?>
--FILE--
<?php
$rlog=rlog_open();
var_dump($rlog);
--EXPECTREGEX--
.*resource.*
