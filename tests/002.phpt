--TEST--
Check for constructor and destructor of Rlogger
--SKIPIF--
<?php
	extension_loaded('rlogger') or die('skip rlogger not available');
    $required_class = array("rlogger");
    foreach ($required_class as $class_name) {
        if (!class_exists($class_name)) {
            die("skip $class_name class is not available.");
        }
    }
?>
--FILE--
<?php
$socket_path = tempnam("/tmp", "rlogger_") . ".sock";
$socket_url  = "unix://" . $socket_path;

$socket = stream_socket_server($socket_url);
$rlogger1 = new Rlogger($socket_url);
var_dump(is_object($rlogger1));
$rlogger1 = null;
$rlogger2 = new Rlogger($socket_url);
fclose($socket);
unlink($socket_path);
--EXPECT--
bool(true)
