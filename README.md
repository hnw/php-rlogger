# php-rlog [![Build Status](https://travis-ci.org/hnw/php-rlog.svg?branch=master)](https://travis-ci.org/hnw/php-rlog)

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
cd src
make
cd ..
```

Now build and install php-rlog.

```
git clone https://github.com/hnw/php-rlog.git
cd php-rlog
phpize
./configure --with-librlog-dir=../rlogd/
make
make install
```

Then edit your 'php.ini'.

```
extension=rlog.so
```

# Usage

## Rlog::__construct()

Open socket for logging.

```
int Rlog::__construct([string address, [int timeout]])
```

Specify `address` for rloggerd listening address.

## Rlog::write()

Write message with tag.

```
int Rlog::write(string tag, string str)
```

## Rlog::close()

Close socket.

```
void Rlog::close()
```

Note: All open sockets would be closed in object destructor. So, it is not necessary to call `close()` explicitly.

## Ini settings

### rlog.address

Specify default address for `Rlog::__construct()` (default: `"unix:///var/run/rlogd/rloggerd.sock"` )

### rlog.timeout

Specify default timeout msec for `Rlog::__construct()` (default: 3000)


## Example

```php
<?php
$rlog = new Rlog();
$rlog->write("example.acc", "Normal log messages");
try {
    // Critical operations...
} catch (Exception $e) {
    $rlog->write("example.err", $e->getMessage());
    throw $e;
}
```

## License

The MIT License

Copyright (c) 2015 Yoshio HANAWA

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
