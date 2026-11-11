/* Antlr */
#include "../lang_def.h"

LANGUAGE(antlr) {
	.name = "antlr",
	.title = "Antlr",
	.extensions = "g g4",

	.keywords = WORDS(
		"abstract", "break", "case", "catch", "continue", "default", "do",
		"else", "extends", "false", "final", "finally", "for", "header", "if",
		"implements", "import", "instanceof", "native", "new", "null",
		"options", "package", "private", "protected", "public", "return",
		"static", "strictfp", "super", "switch", "synchronized", "this",
		"throw", "throws", "tokens", "transient", "true", "try", "volatile",
		"while"),

	.types = WORDS(
		"boolean", "byte", "char", "class", "double", "float", "int",
		"interface", "long", "short", "void"),

	.line_comments = { "//" },
	.block_comments = { { "/" "*", "*" "/" } },
	.quotes = "'",

	.blocks = BLOCKS(
		OP_BLOCK(":", ";"),
		OP_BLOCK("(", ")"),
		OP_BLOCK("{", "}")),
};
