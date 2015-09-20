/*
The MIT License

Copyright (c) 2015 Yoshio HANAWA

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "librlog.h"

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_rlog.h"

ZEND_DECLARE_MODULE_GLOBALS(rlog)

/* True global resources - no need for thread safety here */
static int le_rlog;
#define le_rlog_name "rlog"

#if PHP_VERSION_ID >= 70000
# define RLOG_FETCH_RESOURCE(php_rlog_ptr, z_ptr) do { \
		if ((php_rlog_ptr = (php_rlog *)zend_fetch_resource(Z_RES_P(z_ptr), le_rlog_name, le_rlog)) == NULL) { \
			RETURN_FALSE; \
		} \
} while (0)
#else
#define RLOG_FETCH_RESOURCE(php_rlog_ptr, z_ptr) ZEND_FETCH_RESOURCE(php_rlog_ptr, php_rlog *, &z_ptr, -1, le_rlog_name, le_rlog)
#endif

static void rlog_dtor(
#if PHP_VERSION_ID >= 70000
					  zend_resource *rsrc
#else
					  zend_rsrc_list_entry *rsrc
#endif
					  TSRMLS_DC);


/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("rlog.target", "unix:///var/run/rlogd/rloggerd.sock", PHP_INI_ALL, OnUpdateString, target, zend_rlog_globals, rlog_globals)
    STD_PHP_INI_ENTRY("rlog.timeout", "3000", PHP_INI_ALL, OnUpdateLong, timeout, zend_rlog_globals, rlog_globals) // 3sec
PHP_INI_END()
/* }}} */

/* {{{ proto resource rlog_open(string address, int timeout)
 */
PHP_FUNCTION(rlog_open)
{
	php_rlog *rlog =emalloc(sizeof(php_rlog));
	char *address = NULL;
	size_t address_len;
	int timeout;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sl", &address, &address_len, &timeout) == FAILURE) {
		return;
	}
	switch (ZEND_NUM_ARGS()) {
	case 0:
		address = INI_STR("rlog.target");
	case 1:
		timeout = INI_INT("rlog.timeout");
	}

	rlog->ptr = rlog_open(address, timeout);
	if (rlog->ptr == NULL) {
		efree(rlog);
		RETURN_NULL();
	}

#if PHP_VERSION_ID >= 70000
	RETURN_RES(zend_register_resource(rlog, le_rlog));
#else
	ZEND_REGISTER_RESOURCE(return_value, rlog, le_rlog);
#endif
}
/* }}} */

/* {{{ proto bool rlog_write(resource r, string tag, string str)
 */
PHP_FUNCTION(rlog_write)
{
	zval *res;
	php_rlog *rlog;
	char *tag = NULL, *str = NULL;
#if PHP_VERSION_ID >= 70000
	size_t tag_len, str_len;
#else
	int tag_len, str_len;
#endif
	int return_code;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rss", &res, &tag, &tag_len, &str, &str_len) == FAILURE) {
		return;
	}
	RLOG_FETCH_RESOURCE(rlog, res);

	return_code = rlog_write(rlog->ptr, tag, (size_t)tag_len, str, (size_t)str_len);

	if (return_code == 0) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto void rlog_close(resource r)
 */
PHP_FUNCTION(rlog_close)
{
	zval *res;
	php_rlog *rlog;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &res) == FAILURE) {
		return;
	}

	RLOG_FETCH_RESOURCE(rlog, res);

#if PHP_VERSION_ID >= 70000
	zend_list_close(Z_RES_P(res));
#else
	zend_list_delete(Z_RESVAL_P(res));
#endif

	return;
}
/* }}} */

/* {{{ rlog_dtor() */

static void rlog_dtor(
#if PHP_VERSION_ID >= 70000
					  zend_resource *rsrc
#else
					  zend_rsrc_list_entry *rsrc
#endif
					  TSRMLS_DC)
{
	php_rlog *rlog = (php_rlog *)rsrc->ptr;

	rlog_close(rlog->ptr);
	efree(rlog);
}
/* }}} */

/* {{{ php_rlog_init_globals
 */
static void php_rlog_init_globals(zend_rlog_globals *rlog_globals TSRMLS_DC)
{
	rlog_globals->target = NULL;
	rlog_globals->timeout = 3000;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(rlog)
{
#if PHP_VERSION_ID >= 70000 && defined(COMPILE_DL_GET) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	ZEND_INIT_MODULE_GLOBALS(rlog, php_rlog_init_globals, NULL);
	REGISTER_INI_ENTRIES();

	le_rlog = zend_register_list_destructors_ex(rlog_dtor, NULL, le_rlog_name, module_number);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(rlog)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(rlog)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(rlog)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(rlog)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "rlog support", "enabled");
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ rlog_functions[]
 *
 * Every user visible function must have an entry in rlog_functions[].
 */
const zend_function_entry rlog_functions[] = {
	PHP_FE(rlog_open,	NULL)
	PHP_FE(rlog_write,	NULL)
	PHP_FE(rlog_close,	NULL)
	PHP_FE_END	/* Must be the last line in rlog_functions[] */
};
/* }}} */

/* {{{ rlog_module_entry
 */
zend_module_entry rlog_module_entry = {
	STANDARD_MODULE_HEADER,
	"rlog",
	rlog_functions,
	PHP_MINIT(rlog),
	PHP_MSHUTDOWN(rlog),
	PHP_RINIT(rlog),
	PHP_RSHUTDOWN(rlog),
	PHP_MINFO(rlog),
	PHP_RLOG_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_RLOG
#    if PHP_VERSION_ID >= 70000 && defined(ZTS)
ZEND_TSRMLS_CACHE_DEFINE();
#    endif
ZEND_GET_MODULE(rlog)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
