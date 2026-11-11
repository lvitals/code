/* Fantom */
#include "../lang_def.h"

LANGUAGE(fantom) {
	.name = "fantom",
	.title = "Fantom",
	.extensions = "fan",

	.keywords = WORDS(
		"abstract", "and", "as", "assert", "break", "case", "catch", "class",
		"compare", "const", "continue", "decrement", "default", "div", "do",
		"each", "else", "enum", "equals", "false", "final", "finally", "for",
		"foreach", "get", "goto", "if", "increment", "internal", "inverse",
		"is", "isnot", "lshift", "minus", "mixin", "mod", "mult", "native",
		"negate", "new", "null", "once", "or", "override", "plus", "private",
		"protected", "public", "readonly", "return", "rshift", "serializable",
		"set", "slice", "static", "super", "switch", "this", "throw", "true",
		"try", "using", "virtual", "void", "volatile", "while", "xor"),

	.types = WORDS(
		"Bool", "Decimal", "Duration", "Env", "Err", "Float", "Int", "List",
		"Map", "Obj", "Range", "Str", "Type", "Uri", "Void"),

	.line_comments = { "//" },
	.block_comments = { { "/" "*", "*" "/" } },
	.quotes = "'\"`",

	.blocks = BLOCKS(
		OP_BLOCK("{", "}")),
};
