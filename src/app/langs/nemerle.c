/* Nemerle */
#include "../lang_def.h"

LANGUAGE(nemerle) {
	.name = "nemerle",
	.title = "Nemerle",
	.flags = F_HASHPRE,
	.extensions = "n",

	.keywords = WORDS(
		"_", "abstract", "and", "array", "as", "base", "catch", "class", "def",
		"do", "else", "extends", "extern", "false", "finally", "for", "foreach",
		"fun", "if", "implements", "in", "interface", "internal", "lock",
		"macro", "match", "module", "mutable", "namespace", "new", "null",
		"out", "override", "params", "private", "protected", "public", "ref",
		"repeat", "sealed", "static", "struct", "syntax", "this", "throw",
		"true", "try", "type", "typeof", "unless", "until", "using", "variant",
		"virtual", "when", "where", "while"),

	.types = WORDS(
		"bool", "byte", "char", "decimal", "double", "float", "int", "list",
		"long", "object", "sbyte", "short", "string", "uint", "ulong", "ushort",
		"void"),

	.preproc = WORDS(
		"define", "elif", "else", "endif", "endregion", "error", "if", "ifdef",
		"ifndef", "line", "pragma", "region", "undef", "using", "warning"),

	.line_comments = { "//" },
	.block_comments = { { "/" "*", "*" "/" } },
	.quotes = "'\"",

	.blocks = BLOCKS(
		PRE_BLOCK("region", "endregion"),
		PRE_BLOCK("if", "endif"),
		PRE_BLOCK("ifdef", "endif"),
		PRE_BLOCK("ifndef", "endif"),
		OP_BLOCK("{", "}")),
};
