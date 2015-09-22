/*
The MIT License

Copyright (c) 2015 Yoshio HANAWA

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef PHP_RLOG_H
#define PHP_RLOG_H

#include "Zend/zend_types.h"

extern zend_module_entry rlog_module_entry;
#define phpext_rlog_ptr &rlog_module_entry

#define PHP_RLOG_VERSION "0.1.0"

#ifdef PHP_WIN32
#	define PHP_RLOG_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_RLOG_API __attribute__ ((visibility("default")))
#else
#	define PHP_RLOG_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

typedef struct _php_rlog_object {
#if PHP_VERSION_ID < 70000
	zend_object zo;
#endif
	struct rlog *rlog;
	int initialized;
#if PHP_VERSION_ID >= 70000
	zend_object zo;
#endif
} php_rlog_object;

#if PHP_VERSION_ID >= 70000
static inline php_rlog_object *php_rlog_from_obj(zend_object *obj) {
        return (php_rlog_object*)((char*)(obj) - XtOffsetOf(php_rlog_object, zo));
}

#define Z_RLOG_P(zv)  php_rlog_from_obj(Z_OBJ_P((zv)))
#endif

ZEND_BEGIN_MODULE_GLOBALS(rlog)
	char *address;
#if PHP_VERSION_ID >= 70000
	zend_long timeout;
#else
	long timeout;
#endif
ZEND_END_MODULE_GLOBALS(rlog)

#if PHP_VERSION_ID >= 70000
#    define RLOG_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(rlog, v)
#    if defined(ZTS) && defined(COMPILE_DL_RLOG)
ZEND_TSRMLS_CACHE_EXTERN();
#    endif
#else
#    ifdef ZTS
#        define RLOG_G(v) TSRMG(rlog_globals_id, zend_rlog_globals *, v)
#    else
#        define RLOG_G(v) (rlog_globals.v)
#    endif
#endif

#endif	/* PHP_RLOG_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
