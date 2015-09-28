--TEST--
Check for Rlogger constructor exceptions
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

$socket_url  = "foobar://" . $socket_path;
try {
    $rlogger = new Rlogger($socket_url);
} catch (Exception $e) {
    $rlogger = null;
    var_dump($e->getMessage());
}
var_dump($rlogger);

$socket_url  = "unix://" . $socket_path;
try {
    $rlogger = new Rlogger($socket_url);
} catch (Exception $e) {
    var_dump($e->getMessage());
    $rlogger = null;
}
var_dump($rlogger);
--EXPECTF--
warning: getaddrinfo: %s, host=%s, port=%s (common.c:%d)
string(%d) "Unable to open socket: foobar://%s"
NULL
warning: connect: %s, path=%s (common.c:%d)
string(%d) "Unable to open socket: unix://%s"
NULL
