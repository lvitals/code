/* Dart */
#include "../lang_def.h"

LANGUAGE(dart) {
	.name = "dart",
	.title = "Dart",
	.flags = F_BACKTICK | F_ATTR,
	.extensions = "dart",

	.keywords = WORDS(
		"abstract", "as", "assert", "async", "await", "base", "break", "case",
		"catch", "class", "continue", "covariant", "default", "do", "else",
		"enum", "extends", "extension", "external", "factory", "finally", "for",
		"get", "if", "implements", "in", "interface", "is", "late", "mixin",
		"of", "on", "operator", "required", "rethrow", "return", "sealed",
		"set", "super", "switch", "sync", "this", "throw", "try", "when",
		"while", "with", "yield"),

	.types = WORDS(
		"Function", "Future", "Iterable", "List", "Map", "Null", "Object",
		"Set", "Stream", "String", "bool", "const", "double", "dynamic",
		"final", "int", "new", "static", "type", "typedef", "var", "void"),

	.constants = WORDS(
		"false", "null", "true"),

	.preproc = WORDS(
		"deferred", "export", "hide", "import", "library", "part"),

	.line_comments = { "//" },
	.block_comments = { { "/" "*", "*" "/" } },
	.long_strings = { { "'''", "'''" }, { "\"\"\"", "\"\"\"" } },
	.quotes = "'\"",

	.blocks = BLOCKS(
		OP_BLOCK("{", "}"),
		OP_BLOCK("(", ")"),
		OP_BLOCK("[", "]")),
};
