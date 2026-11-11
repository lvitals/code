/* Myrddin */
#include "../lang_def.h"

LANGUAGE(myrddin) {
	.name = "myrddin",
	.title = "Myrddin",
	.extensions = "myr",

	.keywords = WORDS(
		"break", "const", "continue", "elif", "else", "extern", "false", "for",
		"generic", "goto", "if", "impl", "in", "match", "pkg", "pkglocal",
		"sizeof", "struct", "trait", "true", "type", "union", "use", "var",
		"while"),

	.types = WORDS(
		"bool", "byte", "char", "flt32", "flt64", "int", "int16", "int32",
		"int64", "int8", "uint", "uint16", "uint32", "uint64", "uint8", "void"),

	.line_comments = { "//" },
	.block_comments = { { "/" "*", "*" "/" } },
	.quotes = "'\"",
};
