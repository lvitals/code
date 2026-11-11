/* Vala */
#include "../lang_def.h"

LANGUAGE(vala) {
	.name = "vala",
	.title = "Vala",
	.flags = F_ATTR,
	.extensions = "vala",

	.keywords = WORDS(
		"abstract", "as", "base", "break", "case", "catch", "class", "const",
		"construct", "continue", "default", "delegate", "delete", "do",
		"dynamic", "else", "ensures", "enum", "errordomain", "extern", "false",
		"finally", "for", "foreach", "get", "if", "in", "inline", "interface",
		"is", "lock", "namespace", "new", "null", "out", "override", "private",
		"protected", "public", "ref", "requires", "return", "set", "signal",
		"sizeof", "static", "struct", "switch", "this", "throw", "throws",
		"true", "try", "typeof", "using", "value", "var", "virtual", "void",
		"volatile", "weak", "while"),

	.types = WORDS(
		"bool", "char", "double", "float", "int", "int16", "int32", "int64",
		"int8", "long", "short", "size_t", "ssize_t", "string", "uchar", "uint",
		"uint16", "uint32", "uint64", "uint8", "ulong", "unichar", "ushort"),

	.line_comments = { "//" },
	.block_comments = { { "/" "*", "*" "/" } },
	.long_strings = { { "\"\"\"", "\"\"\"" } },
	.quotes = "'\"",

	.blocks = BLOCKS(
		OP_BLOCK("{", "}")),
};
