--TEST--
Check for Rlog properties
--SKIPIF--
<?php
	extension_loaded('rlog') or die('skip rlog not available');
    $required_class = array("rlog");
    foreach ($required_class as $class_name) {
        if (!class_exists($class_name)) {
            die("skip $class_name class is not available.");
        }
    }
	$required_func = array("stream_socket_server");
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
$rlog = new Rlog($socket_url);
var_dump($rlog);
$rlog->close();
var_dump($rlog);
fclose($socket);
unlink($socket_path);
--EXPECTF--
%sRlog%s#%d (%d) {
  %saddress%s=>
  string(%d) "unix://%s/rlog_%s.sock"
}
%sRlog%s#%d (%d) {
  %saddress%s=>
  string(0) ""
}
