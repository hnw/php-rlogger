--TEST--
Check for Rlog constructor exceptions
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

$socket_url  = "foobar://" . $socket_path;
try {
    $rlog = new Rlog($socket_url);
} catch (Exception $e) {
    $rlog = null;
    var_dump($e->getMessage());
}
var_dump($rlog);

$socket_url  = "unix://" . $socket_path;
try {
    $rlog = new Rlog($socket_url);
} catch (Exception $e) {
    var_dump($e->getMessage());
    $rlog = null;
}
var_dump($rlog);
--EXPECTF--
warning: getaddrinfo: nodename nor servname provided, or not known, host=%s, port=%s (common.c:%d)
string(%d) "Unable to open socket: foobar://%s"
NULL
warning: connect: No such file or directory, path=%s (common.c:%d)
string(%d) "Unable to open socket: unix://%s"
NULL
