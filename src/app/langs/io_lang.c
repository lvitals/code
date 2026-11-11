/* Io */
#include "../lang_def.h"

LANGUAGE(io_lang) {
	.name = "io_lang",
	.title = "Io",
	.extensions = "io",

	.keywords = WORDS(
		"block", "clone", "do", "else", "foreach", "forward", "hasSlot", "if",
		"method", "print", "proto", "self", "setSlot", "super", "type", "while",
		"write"),

	.types = WORDS(
		"Block", "Buffer", "CFunction", "Date", "Duration", "File", "Future",
		"LinkedList", "List", "Map", "Message", "Nil", "Nop", "Number",
		"Object", "String", "WeakLink"),

	.line_comments = { "#", "//" },
	.block_comments = { { "/" "*", "*" "/" } },
	.long_strings = { { "\"\"\"", "\"\"\"" } },
	.quotes = "'\"",

	.blocks = BLOCKS(
		OP_BLOCK("(", ")")),
};
