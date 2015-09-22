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
#include "Zend/zend_exceptions.h"
#include "php_rlog.h"

ZEND_DECLARE_MODULE_GLOBALS(rlog)

/* True global resources - no need for thread safety here */
static int le_rlog;

#define RLOG_CHECK_INITIALIZED(rlog_obj) \
        if (!(rlog_obj) || !(rlog_obj->initialized)) { \
			zend_throw_exception(zend_exception_get_default(TSRMLS_C), "The Rlog object has not been correctly initialised", 0 TSRMLS_CC); \
                RETURN_FALSE; \
        }

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("rlog.address", "unix:///var/run/rlogd/rloggerd.sock", PHP_INI_ALL, OnUpdateString, address, zend_rlog_globals, rlog_globals)
    STD_PHP_INI_ENTRY("rlog.timeout", "3000", PHP_INI_ALL, OnUpdateLong, timeout, zend_rlog_globals, rlog_globals) // 3sec
PHP_INI_END()
/* }}} */

/* Handlers */
static zend_object_handlers rlog_object_handlers;

/* Class entries */
zend_class_entry *php_rlog_entry;

/* {{{ proto int Rlog::__construct([string address, [int timeout]])
   __constructor for Rlog. */
PHP_METHOD(rlog, __construct)
{
	zval *object = getThis();
	php_rlog_object *rlog_obj;
	struct rlog *rlog;
	char *address = NULL;
	size_t address_len;
	int timeout;
	zend_error_handling error_handling;

#if PHP_VERSION_ID >= 70000
	rlog_obj = Z_RLOG_P(object);
#else
	rlog_obj = (php_rlog_object *)zend_object_store_get_object(object TSRMLS_CC);
#endif

	if (rlog_obj->initialized) {
		zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Already initialized Rlog Object", 0 TSRMLS_CC);
	}

	zend_replace_error_handling(EH_THROW, NULL, &error_handling TSRMLS_CC);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sl", &address, &address_len, &timeout) == FAILURE) {
		zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}
	zend_restore_error_handling(&error_handling TSRMLS_CC);

	switch (ZEND_NUM_ARGS()) {
	case 0:
		address = INI_STR("rlog.address");
	case 1:
		timeout = INI_INT("rlog.timeout");
	}

	rlog = rlog_open(address, timeout);
	if (rlog == NULL) {
		zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C), 0 TSRMLS_CC, "Unable to open socket: %s", address);
		return;
	}

	rlog_obj->rlog = rlog;
	rlog_obj->initialized = 1;

	zend_update_property_string(php_rlog_entry, object, "address", sizeof("address") - 1, address TSRMLS_CC);
}
/* }}} */

/* {{{ proto int Rlog::write(string tag, string str)
 */
PHP_METHOD(rlog, write)
{
	zval *object = getThis();
	php_rlog_object *rlog_obj;
	char *tag = NULL, *str = NULL;
#if PHP_VERSION_ID >= 70000
	size_t tag_len, str_len;
#else
	int tag_len, str_len;
#endif
	int return_code;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &tag, &tag_len, &str, &str_len) == FAILURE) {
		return;
	}

#if PHP_VERSION_ID >= 70000
	rlog_obj = Z_RLOG_P(object);
#else
	rlog_obj = (php_rlog_object *)zend_object_store_get_object(object TSRMLS_CC);
#endif

	RLOG_CHECK_INITIALIZED(rlog_obj);

	return_code = rlog_write(rlog_obj->rlog, tag, (size_t)tag_len, str, (size_t)str_len);

	if (return_code == 0) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto void Rlog::close()
 */
PHP_METHOD(rlog, close)
{
	zval *object = getThis();
	php_rlog_object *rlog_obj;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

#if PHP_VERSION_ID >= 70000
	rlog_obj = Z_RLOG_P(object);
#else
	rlog_obj = (php_rlog_object *)zend_object_store_get_object(object TSRMLS_CC);
#endif

	if (rlog_obj->initialized) {
		if (rlog_obj->rlog) {
			rlog_close(rlog_obj->rlog);
			rlog_obj->rlog = NULL;
		}
		rlog_obj->initialized = 0;
		zend_update_property_string(php_rlog_entry, object, "address", sizeof("address") - 1, "" TSRMLS_CC);
	}

	return;
}
/* }}} */

/* {{{ php_rlog_init_globals
 */
static void php_rlog_init_globals(zend_rlog_globals *rlog_globals TSRMLS_DC)
{
	rlog_globals->address = NULL;
	rlog_globals->timeout = 3000;
}
/* }}} */

static void php_rlog_object_free_storage(
#if PHP_VERSION_ID >= 70000
										 zend_object *object
#else
										 void *object TSRMLS_DC
#endif
										 ) /* {{{ */
{
	php_rlog_object *intern;
#if PHP_VERSION_ID >= 70000
	intern = php_rlog_from_obj(object);
#else
	intern = (php_rlog_object *) object;
#endif

	if (!intern) {
		return;
	}
	if (intern->initialized) {
		if (intern->rlog) {
			rlog_close(intern->rlog);
			intern->rlog = NULL;
		}
		intern->initialized = 0;
	}
	zend_object_std_dtor(&intern->zo TSRMLS_CC);
#if PHP_VERSION_ID < 70000
	efree(intern);
#endif
}


#if PHP_VERSION_ID >= 70000
static zend_object *php_rlog_object_new
#else
static zend_object_value php_rlog_object_new
#endif
    (zend_class_entry *class_type TSRMLS_DC) /* {{{ */
{
#if PHP_VERSION_ID < 70000
		zend_object_value retval;
#endif
        php_rlog_object *intern;
		zval *tmp;

        /* Allocate memory for it */
#if PHP_VERSION_ID >= 70000
        intern = ecalloc(1, sizeof(php_rlog_object) + zend_object_properties_size(class_type));
#else
		intern = emalloc(sizeof(php_rlog_object));
		memset(intern, 0, sizeof(php_rlog_object));
#endif

        zend_object_std_init(&intern->zo, class_type TSRMLS_CC);
#if PHP_VERSION_ID >= 50400
        object_properties_init(&intern->zo, class_type);
#else
		zend_hash_copy(intern->zo.properties, &class_type->default_properties, (copy_ctor_func_t) zval_property_ctor,(void *) &tmp, sizeof(zval *));
#endif

#if PHP_VERSION_ID >= 70000
        intern->zo.handlers = &rlog_object_handlers;
        return &intern->zo;
#else
		retval.handle = zend_objects_store_put(intern, NULL, (zend_objects_free_object_storage_t) php_rlog_object_free_storage, NULL TSRMLS_CC);
        retval.handlers = (zend_object_handlers *) &rlog_object_handlers;
        return retval;
#endif
}
/* }}} */

/* {{{ rlog_class_methods[]
 */
const zend_function_entry rlog_class_methods[] = {
	PHP_ME(rlog, write,       NULL, ZEND_ACC_PUBLIC)
	PHP_ME(rlog, close,       NULL, ZEND_ACC_PUBLIC)
	PHP_ME(rlog, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_FE_END
};
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(rlog)
{
	zend_class_entry ce;

#if PHP_VERSION_ID >= 70000 && defined(COMPILE_DL_GET) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	memcpy(&rlog_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

	/* Register Rlog Class */
	INIT_CLASS_ENTRY(ce, "Rlog", rlog_class_methods);
	ce.create_object = php_rlog_object_new;
#if PHP_VERSION_ID >= 70000
	rlog_object_handlers.offset = XtOffsetOf(php_rlog_object, zo);
	rlog_object_handlers.free_obj = php_rlog_object_free_storage;
#endif
	rlog_object_handlers.clone_obj = NULL;
	php_rlog_entry = zend_register_internal_class(&ce TSRMLS_CC);

	zend_declare_property_string(php_rlog_entry, "address", sizeof("address")-1, "", ZEND_ACC_PROTECTED TSRMLS_CC);

	ZEND_INIT_MODULE_GLOBALS(rlog, php_rlog_init_globals, NULL);
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

/* {{{ rlog_module_entry
 */
zend_module_entry rlog_module_entry = {
	STANDARD_MODULE_HEADER,
	"rlog",
	NULL,
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
