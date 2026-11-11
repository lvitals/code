/* IDL */
#include "../lang_def.h"

LANGUAGE(idl) {
	.name = "idl",
	.title = "IDL",
	.flags = F_HASHPRE,
	.extensions = "idl odl",

	.keywords = WORDS(
		"FALSE", "TRUE", "abstract", "attribute", "case", "const", "context",
		"custom", "default", "enum", "exception", "factory", "in", "inout",
		"interface", "local", "module", "native", "oneway", "out", "private",
		"public", "raises", "readonly", "struct", "support", "switch",
		"truncatable", "typedef", "union", "valuetype"),

	.types = WORDS(
		"Object", "ValueBase", "any", "boolean", "char", "double", "fixed",
		"float", "long", "octet", "sequence", "short", "string", "unsigned",
		"void", "wchar", "wstring"),

	.preproc = WORDS(
		"define", "elif", "else", "endif", "if", "ifdef", "ifndef", "include",
		"pragma", "undef", "warning"),

	.line_comments = { "//" },
	.block_comments = { { "/" "*", "*" "/" } },
	.quotes = "'\"",
};
