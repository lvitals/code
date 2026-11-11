/* Boo */
#include "../lang_def.h"

LANGUAGE(boo) {
	.name = "boo",
	.title = "Boo",
	.extensions = "boo",

	.keywords = WORDS(
		"abstract", "and", "as", "break", "callable", "cast", "class",
		"constructor", "continue", "def", "destructor", "do", "elif", "else",
		"ensure", "enum", "event", "except", "false", "final", "for", "from",
		"get", "given", "goto", "if", "import", "in", "interface", "internal",
		"is", "isa", "namespace", "not", "null", "of", "or", "otherwise",
		"override", "partial", "pass", "private", "protected", "public",
		"raise", "ref", "return", "self", "set", "static", "struct", "super",
		"transient", "true", "try", "unless", "virtual", "when", "while",
		"yield"),

	.types = WORDS(
		"bool", "byte", "char", "date", "decimal", "double", "duck", "float",
		"int", "long", "object", "operator", "regex", "sbyte", "short",
		"single", "string", "timespan", "uint", "ulong", "ushort"),

	.functions = WORDS(
		"__eval__", "__switch__", "array", "assert", "checked", "enumerate",
		"filter", "getter", "len", "lock", "map", "matrix", "max", "min",
		"normalArrayIndexing", "print", "property", "range", "rawArrayIndexing",
		"required", "typeof", "unchecked", "using", "yieldAll", "zip"),

	.line_comments = { "#" },
	.block_comments = { { "/" "*", "*" "/" } },
	.long_strings = { { "\"\"\"", "\"\"\"" } },
	.quotes = "'\"",
};
