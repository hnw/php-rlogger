--TEST--
Check for rlog_write()
--SKIPIF--
<?php
	extension_loaded('rlog') or die('skip rlog not available');
	$required_func = array("rlog_open", "rlog_write", "stream_socket_server");
	foreach ($required_func as $func_name) {
		if (!function_exists($func_name)) {
			die("skip $func_name() function is not available.");
		}
	}
?>
--FILE--
<?php
$socket_path = tempnam("/tmp", "rlog_") . ".sock";
$socket_url  = "unix://" . $socket_path;

$socket = stream_socket_server($socket_url);
$rlog = rlog_open($socket_url);
$ret = rlog_write($rlog, "example.acc", "foobar");
var_dump($ret);
fclose($socket);
unlink($socket_path);
--EXPECT--
bool(true)
