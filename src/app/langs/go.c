/* Go */
#include "../lang_def.h"

LANGUAGE(go) {
	.name = "go",
	.title = "Go",
	.flags = F_BACKTICK,
	.extensions = "go",

	.keywords = WORDS(
		"break", "case", "chan", "const", "continue", "default", "defer",
		"else", "fallthrough", "for", "func", "go", "goto", "if", "import",
		"interface", "map", "package", "range", "return", "select", "struct",
		"switch", "type", "var"),

	.types = WORDS(
		"any", "bool", "byte", "comparable", "complex128", "complex64", "error",
		"float32", "float64", "int", "int16", "int32", "int64", "int8", "rune",
		"string", "uint", "uint16", "uint32", "uint64", "uint8", "uintptr"),

	.constants = WORDS(
		"false", "iota", "nil", "true"),

	.functions = WORDS(
		"append", "cap", "close", "complex", "copy", "delete", "imag", "len",
		"make", "new", "panic", "print", "println", "real", "recover"),

	.line_comments = { "//" },
	.block_comments = { { "/" "*", "*" "/" } },
	.quotes = "'\"`",

	.blocks = BLOCKS(
		OP_BLOCK("{", "}")),
};
