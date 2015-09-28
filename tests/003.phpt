--TEST--
Check for Rlogger::write()
--SKIPIF--
<?php
	extension_loaded('rlogger') or die('skip rlogger not available');
    $required_class = array("rlogger");
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
$socket_path = tempnam("/tmp", "rlogger_") . ".sock";
$socket_url  = "unix://" . $socket_path;

$socket = stream_socket_server($socket_url);
$rlogger = new Rlogger($socket_url);
$ret = $rlogger->write("example.acc", "foobar");
var_dump($ret);
fclose($socket);
unlink($socket_path);
--EXPECT--
bool(true)
