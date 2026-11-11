/* Pike */
#include "../lang_def.h"

LANGUAGE(pike) {
	.name = "pike",
	.title = "Pike",
	.flags = F_HASHPRE,
	.extensions = "pike pmod",

	.keywords = WORDS(
		"break", "case", "catch", "constant", "continue", "default", "do",
		"else", "extern", "final", "for", "foreach", "gauge", "if", "import",
		"inherit", "inline", "lambda", "local", "nomask", "optional", "private",
		"protected", "public", "return", "sscanf", "static", "switch",
		"variant", "while"),

	.types = WORDS(
		"array", "class", "float", "function", "int", "mapping", "mixed",
		"multiset", "object", "program", "string", "void"),

	.line_comments = { "//" },
	.block_comments = { { "/" "*", "*" "/" } },
	.quotes = "'\"",

	.blocks = BLOCKS(
		OP_BLOCK("{", "}")),
};
