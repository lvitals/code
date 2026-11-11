/* PHP */
#include "../lang_def.h"

LANGUAGE(php) {
	.name = "php",
	.title = "PHP",
	.flags = F_VARS,
	.extensions = "inc php php3 php4 phtml",
	.interpreters = "php",

	.keywords = WORDS(
		"ArithmeticError", "AssertionError", "Closure", "Directory",
		"DivisionByZeroError", "Error", "ErrorException", "Exception",
		"Generator", "ParseError", "Throwable", "TypeError",
		"__PHP_Incomplete_Class", "__halt_compiler", "abstract", "and", "array",
		"as", "break", "callable", "case", "catch", "class", "clone", "const",
		"continue", "declare", "default", "die", "do", "echo", "else", "elseif",
		"empty", "enddeclare", "endfor", "endforeach", "endif", "endswitch",
		"endwhile", "eval", "exit", "extends", "final", "finally", "fn", "for",
		"foreach", "from", "function", "global", "goto", "if", "implements",
		"include", "include_once", "instanceof", "insteadof", "interface",
		"isset", "list", "namespace", "new", "or", "parent", "php_user_filter",
		"print", "private", "protected", "public", "require", "require_once",
		"return", "self", "static", "stdClass", "switch", "throw", "trait",
		"try", "unset", "use", "var", "while", "xor", "yield"),

	.types = WORDS(
		"bool", "false", "float", "int", "iterable", "null", "object", "string",
		"true", "void"),

	.constants = WORDS(
		"DEFAULT_INCLUDE_PATH", "E_ALL", "E_COMPILE_ERROR", "E_CORE_ERROR",
		"E_CORE_WARNING", "E_DEPRECATED", "E_ERROR", "E_NOTICE", "E_PARSE",
		"E_STRICT", "E_USER_DEPRECATED", "E_USER_ERROR", "E_USER_NOTICE",
		"E_USER_WARNING", "E_WARNING", "PEAR_EXTENSION_DIR", "PEAR_INSTALL_DIR",
		"PHP_BINARY", "PHP_BINDIR", "PHP_CONFIG_FILE_PATH",
		"PHP_CONFIG_FILE_SCAN_DIR", "PHP_DATADIR", "PHP_DEBUG", "PHP_EOL",
		"PHP_EXTENSION_DIR", "PHP_EXTRA_VERSION", "PHP_FD_SETSIZE",
		"PHP_FLOAT_DIG", "PHP_FLOAT_EPSILON", "PHP_FLOAT_MAX", "PHP_FLOAT_MIN",
		"PHP_INT_MAX", "PHP_INT_MIN", "PHP_INT_SIZE", "PHP_LIBDIR",
		"PHP_LOCALSTATEDIR", "PHP_MAJOR_VERSION", "PHP_MANDIR",
		"PHP_MAXPATHLEN", "PHP_MINOR_VERSION", "PHP_OS", "PHP_OS_FAMILY",
		"PHP_PREFIX", "PHP_RELEASE_VERSION", "PHP_SAPI", "PHP_SHLIB_SUFFIX",
		"PHP_SYSCONFDIR", "PHP_VERSION", "PHP_VERSION_ID", "PHP_ZTS",
		"__CLASS__", "__COMPILER_HALT_OFFSET__", "__DIR__", "__FILE__",
		"__FUNCTION__", "__LINE__", "__METHOD__", "__NAMESPACE__", "__TRAIT__"),

	.line_comments = { "//", "#" },
	.block_comments = { { "/" "*", "*" "/" } },
	.quotes = "'\"`",

	.blocks = BLOCKS(
		PRE_BLOCK("<?", "?>"),
		OP_BLOCK("{", "}"),
		OP_BLOCK("(", ")")),
};
