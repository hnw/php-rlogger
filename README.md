# php-rlogger [![Build Status](https://travis-ci.org/hnw/php-rlogger.svg?branch=master)](https://travis-ci.org/hnw/php-rlogger)

A PHP extension for rlogd (see: https://github.com/pandax381/rlogd )

# Requirement

- PHP 5.3.x - 7.0.x
- [rlogd](https://github.com/pandax381/rlogd)

# Installation

First of all, prepare `librlog.a` from rlogd package.

```
sudo apt-get install libev-dev libpcre3-dev
git clone https://github.com/pandax381/rlogd.git rlogd
cd rlogd
./autogen.sh
./configure
make
cd ..
```

Now build and install php-rlogger.

```
git clone https://github.com/hnw/php-rlogger.git
cd php-rlogger
phpize
./configure --with-rlogd-src-dir=../rlogd/
make
make install
```

Then edit your `php.ini`.

```
extension=rlogger.so
```

# Usage

## Rlogger::__construct()

Open socket for logging.

```
int Rlogger::__construct([string address, [int timeout]])
```

Specify `address` for rloggerd listening address.

## Rlogger::write()

Write message with tag.

```
int Rlogger::write(string tag, string str)
```

## Rlogger::close()

Close socket.

```
void Rlogger::close()
```

Note: All open sockets would be closed in object destructor. So, it is not necessary to call `close()` explicitly.

## Ini settings

### rlogger.address

Specify default address for `Rlogger::__construct()` (default: `"unix:///var/run/rlogd/rloggerd.sock"` )

### rlogger.timeout

Specify default timeout msec for `Rlogger::__construct()` (default: 3000)


## Example

```php
<?php
$rlogger = new Rlogger();
$rlogger->write("example.acc", "Normal log messages");
try {
    // Critical operations...
} catch (Exception $e) {
    $rlogger->write("example.err", $e->getMessage());
    throw $e;
}
```

## License

The MIT License

Copyright (c) 2015 Yoshio HANAWA

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
