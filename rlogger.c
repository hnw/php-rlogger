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
#include "php_rlogger.h"

ZEND_DECLARE_MODULE_GLOBALS(rlogger)

/* True global resources - no need for thread safety here */
static int le_rlogger;

#define RLOGGER_CHECK_INITIALIZED(rlogger_obj) \
        if (!(rlogger_obj) || !(rlogger_obj->initialized)) { \
			zend_throw_exception(zend_exception_get_default(TSRMLS_C), "The Rlogger object has not been correctly initialised", 0 TSRMLS_CC); \
                RETURN_FALSE; \
        }

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("rlogger.address", "unix:///var/run/rlogd/rloggerd.sock", PHP_INI_ALL, OnUpdateString, address, zend_rlogger_globals, rlogger_globals)
    STD_PHP_INI_ENTRY("rlogger.timeout", "3000", PHP_INI_ALL, OnUpdateLong, timeout, zend_rlogger_globals, rlogger_globals) // 3sec
PHP_INI_END()
/* }}} */

/* Handlers */
static zend_object_handlers rlogger_object_handlers;

/* Class entries */
zend_class_entry *php_rlogger_entry;

/* {{{ proto int Rlogger::__construct([string address, [int timeout]])
   __constructor for Rlogger. */
PHP_METHOD(rlogger, __construct)
{
	zval *object = getThis();
	php_rlogger_object *rlogger_obj;
	struct rlogger *rlogger;
	char *address = NULL;
	size_t address_len;
	int timeout;
	zend_error_handling error_handling;

#if PHP_VERSION_ID >= 70000
	rlogger_obj = Z_RLOGGER_P(object);
#else
	rlogger_obj = (php_rlogger_object *)zend_object_store_get_object(object TSRMLS_CC);
#endif

	if (rlogger_obj->initialized) {
		zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Already initialized Rlogger Object", 0 TSRMLS_CC);
	}

	zend_replace_error_handling(EH_THROW, NULL, &error_handling TSRMLS_CC);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sl", &address, &address_len, &timeout) == FAILURE) {
		zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}
	zend_restore_error_handling(&error_handling TSRMLS_CC);

	switch (ZEND_NUM_ARGS()) {
	case 0:
		address = INI_STR("rlogger.address");
	case 1:
		timeout = INI_INT("rlogger.timeout");
	}

	rlogger = rlog_open(address, timeout);
	if (rlogger == NULL) {
		zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C), 0 TSRMLS_CC, "Unable to open socket: %s", address);
		return;
	}

	rlogger_obj->rlogger = rlogger;
	rlogger_obj->initialized = 1;

	zend_update_property_string(php_rlogger_entry, object, "address", sizeof("address") - 1, address TSRMLS_CC);
}
/* }}} */

/* {{{ proto int Rlogger::write(string tag, string str)
 */
PHP_METHOD(rlogger, write)
{
	zval *object = getThis();
	php_rlogger_object *rlogger_obj;
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
	rlogger_obj = Z_RLOGGER_P(object);
#else
	rlogger_obj = (php_rlogger_object *)zend_object_store_get_object(object TSRMLS_CC);
#endif

	RLOGGER_CHECK_INITIALIZED(rlogger_obj);

	return_code = rlog_write(rlogger_obj->rlogger, tag, (size_t)tag_len, str, (size_t)str_len);

	if (return_code == 0) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto void Rlogger::close()
 */
PHP_METHOD(rlogger, close)
{
	zval *object = getThis();
	php_rlogger_object *rlogger_obj;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

#if PHP_VERSION_ID >= 70000
	rlogger_obj = Z_RLOGGER_P(object);
#else
	rlogger_obj = (php_rlogger_object *)zend_object_store_get_object(object TSRMLS_CC);
#endif

	if (rlogger_obj->initialized) {
		if (rlogger_obj->rlogger) {
			rlog_close(rlogger_obj->rlogger);
			rlogger_obj->rlogger = NULL;
		}
		rlogger_obj->initialized = 0;
		zend_update_property_string(php_rlogger_entry, object, "address", sizeof("address") - 1, "" TSRMLS_CC);
	}

	return;
}
/* }}} */

/* {{{ php_rlogger_init_globals
 */
static void php_rlogger_init_globals(zend_rlogger_globals *rlogger_globals TSRMLS_DC)
{
	rlogger_globals->address = NULL;
	rlogger_globals->timeout = 3000;
}
/* }}} */

static void php_rlogger_object_free_storage(
#if PHP_VERSION_ID >= 70000
										 zend_object *object
#else
										 void *object TSRMLS_DC
#endif
										 ) /* {{{ */
{
	php_rlogger_object *intern;
#if PHP_VERSION_ID >= 70000
	intern = php_rlogger_from_obj(object);
#else
	intern = (php_rlogger_object *) object;
#endif

	if (!intern) {
		return;
	}
	if (intern->initialized) {
		if (intern->rlogger) {
			rlog_close(intern->rlogger);
			intern->rlogger = NULL;
		}
		intern->initialized = 0;
	}
	zend_object_std_dtor(&intern->zo TSRMLS_CC);
#if PHP_VERSION_ID < 70000
	efree(intern);
#endif
}


#if PHP_VERSION_ID >= 70000
static zend_object *php_rlogger_object_new
#else
static zend_object_value php_rlogger_object_new
#endif
    (zend_class_entry *class_type TSRMLS_DC) /* {{{ */
{
#if PHP_VERSION_ID < 70000
		zend_object_value retval;
#endif
        php_rlogger_object *intern;
		zval *tmp;

        /* Allocate memory for it */
#if PHP_VERSION_ID >= 70000
        intern = ecalloc(1, sizeof(php_rlogger_object) + zend_object_properties_size(class_type));
#else
		intern = emalloc(sizeof(php_rlogger_object));
		memset(intern, 0, sizeof(php_rlogger_object));
#endif

        zend_object_std_init(&intern->zo, class_type TSRMLS_CC);
#if PHP_VERSION_ID >= 50400
        object_properties_init(&intern->zo, class_type);
#else
		zend_hash_copy(intern->zo.properties, &class_type->default_properties, (copy_ctor_func_t) zval_property_ctor,(void *) &tmp, sizeof(zval *));
#endif

#if PHP_VERSION_ID >= 70000
        intern->zo.handlers = &rlogger_object_handlers;
        return &intern->zo;
#else
		retval.handle = zend_objects_store_put(intern, NULL, (zend_objects_free_object_storage_t) php_rlogger_object_free_storage, NULL TSRMLS_CC);
        retval.handlers = (zend_object_handlers *) &rlogger_object_handlers;
        return retval;
#endif
}
/* }}} */

/* {{{ rlogger_class_methods[]
 */
const zend_function_entry rlogger_class_methods[] = {
	PHP_ME(rlogger, write,       NULL, ZEND_ACC_PUBLIC)
	PHP_ME(rlogger, close,       NULL, ZEND_ACC_PUBLIC)
	PHP_ME(rlogger, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_FE_END
};
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(rlogger)
{
	zend_class_entry ce;

#if PHP_VERSION_ID >= 70000 && defined(COMPILE_DL_GET) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	memcpy(&rlogger_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

	/* Register Rlogger Class */
	INIT_CLASS_ENTRY(ce, "Rlogger", rlogger_class_methods);
	ce.create_object = php_rlogger_object_new;
#if PHP_VERSION_ID >= 70000
	rlogger_object_handlers.offset = XtOffsetOf(php_rlogger_object, zo);
	rlogger_object_handlers.free_obj = php_rlogger_object_free_storage;
#endif
	rlogger_object_handlers.clone_obj = NULL;
	php_rlogger_entry = zend_register_internal_class(&ce TSRMLS_CC);

	zend_declare_property_string(php_rlogger_entry, "address", sizeof("address")-1, "", ZEND_ACC_PROTECTED TSRMLS_CC);

	ZEND_INIT_MODULE_GLOBALS(rlogger, php_rlogger_init_globals, NULL);
	REGISTER_INI_ENTRIES();

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(rlogger)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(rlogger)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(rlogger)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(rlogger)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "rlogger support", "enabled");
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ rlogger_module_entry
 */
zend_module_entry rlogger_module_entry = {
	STANDARD_MODULE_HEADER,
	"rlogger",
	NULL,
	PHP_MINIT(rlogger),
	PHP_MSHUTDOWN(rlogger),
	PHP_RINIT(rlogger),
	PHP_RSHUTDOWN(rlogger),
	PHP_MINFO(rlogger),
	PHP_RLOGGER_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_RLOGGER
#    if PHP_VERSION_ID >= 70000 && defined(ZTS)
ZEND_TSRMLS_CACHE_DEFINE();
#    endif
ZEND_GET_MODULE(rlogger)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
