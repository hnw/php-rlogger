--TEST--
Check for constructor and destructor of Rlog
--SKIPIF--
<?php
	extension_loaded('rlog') or die('skip rlog not available');
    $required_class = array("rlog");
    foreach ($required_class as $class_name) {
        if (!class_exists($class_name)) {
            die("skip $class_name class is not available.");
        }
    }
?>
--FILE--
<?php
$socket_path = tempnam("/tmp", "rlog_") . ".sock";
$socket_url  = "unix://" . $socket_path;

$socket = stream_socket_server($socket_url);
$rlog1 = new Rlog($socket_url);
var_dump(is_object($rlog1));
$rlog1 = null;
$rlog2 = new Rlog($socket_url);
fclose($socket);
unlink($socket_path);
--EXPECT--
bool(true)
