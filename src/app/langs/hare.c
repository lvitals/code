/* Hare */
#include "../lang_def.h"

LANGUAGE(hare) {
	.name = "hare",
	.title = "Hare",
	.extensions = "ha",

	.keywords = WORDS(
		"_", "as", "break", "case", "const", "continue", "def", "defer", "else",
		"export", "fn", "for", "if", "is", "let", "match", "nullable", "return",
		"static", "switch", "type", "use", "yield"),

	.types = WORDS(
		"bool", "enum", "f32", "f64", "i16", "i32", "i64", "i8", "int", "never",
		"opaque", "rune", "size", "str", "struct", "u16", "u32", "u64", "u8",
		"uint", "uintptr", "union", "valist"),

	.constants = WORDS(
		"done", "false", "null", "true", "void"),

	.functions = WORDS(
		"abort", "align", "alloc", "append", "assert", "delete", "free",
		"insert", "len", "offset", "vaarg", "vaend", "vastart"),

	.line_comments = { "//" },
	.quotes = "'\"`",

	.blocks = BLOCKS(
		OP_BLOCK("{", "}")),
};
