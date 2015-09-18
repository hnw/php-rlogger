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

static void rlog_dtor(zend_resource *rsrc);

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
	struct rlog *rlog;
	char *address = NULL;
	size_t address_len;
	zend_long timeout;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sl", &address, &address_len, &timeout) == FAILURE) {
		return;
	}
	switch (ZEND_NUM_ARGS()) {
	case 0:
		address = INI_STR("rlog.target");
	case 1:
		timeout = INI_INT("rlog.timeout");
	}

	php_error_docref(NULL, E_WARNING, "address='%s', timeout=%d", address, timeout);

	rlog = rlog_open(address, timeout);

	RETURN_RES(zend_register_resource(rlog, le_rlog));
}
/* }}} */

/* {{{ proto int rlog_write(resource r, string tag, string str)
 */
PHP_FUNCTION(rlog_write)
{
	zval *RLOG;
	struct rlog *rlog;
	char *tag = NULL, *str = NULL;
	size_t tag_len, str_len;
	int res;

	php_error_docref(NULL, E_WARNING, "rlog:write 1");
		
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rss", &RLOG, &tag, &tag_len, &str, &str_len) == FAILURE) {
		return;
	}

	php_error_docref(NULL, E_WARNING, "rlog:write %s %d %s %d", tag, tag_len, str, str_len);

	#if PHP_VERSION_ID >= 70000
	if ((rlog = (struct rlog *)zend_fetch_resource(Z_RES_P(RLOG), le_rlog_name, le_rlog)) == NULL) {
		RETURN_FALSE;
	}
#else
	ZEND_FETCH_RESOURCE(rlog, struct rlog *, &RLOG, -1, le_rlog_name, le_rlog);
#endif
	
	res = rlog_write(rlog, tag, tag_len, str, str_len);

	php_error_docref(NULL, E_WARNING, "rlog:write 2");
		
	RETURN_LONG(res);
}
/* }}} */

/* {{{ proto void rlog_close(resource r)
 */
PHP_FUNCTION(rlog_close)
{
	zval *RLOG;
	struct rlog *rlog;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &RLOG) == FAILURE) {
		return;
	}

#if PHP_VERSION_ID >= 70000
	if ((rlog = (struct rlog *)zend_fetch_resource(Z_RES_P(RLOG), le_rlog_name, le_rlog)) == NULL) {
		RETURN_FALSE;
	}
#else
	ZEND_FETCH_RESOURCE(rlog, struct rlog *, &RLOG, -1, le_rlog_name, le_rlog);
#endif

	rlog_close(rlog);

	return;
}
/* }}} */

/* {{{ xml_parser_dtor() */
static void rlog_dtor(zend_resource *rsrc)
{
	struct rlog *rlog = (struct rlog *)rsrc->ptr;
	efree(rlog);
}
/* }}} */

/* {{{ php_rlog_init_globals
 */
static void php_rlog_init_globals(zend_rlog_globals *rlog_globals)
{
	rlog_globals->target = NULL;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(rlog)
{
	le_rlog = zend_register_list_destructors_ex(rlog_dtor, NULL, le_rlog_name, module_number);

	REGISTER_INI_ENTRIES();
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
