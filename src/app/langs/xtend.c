/* Xtend */
#include "../lang_def.h"

LANGUAGE(xtend) {
	.name = "xtend",
	.title = "Xtend",
	.flags = F_ATTR,
	.extensions = "xtend",

	.keywords = WORDS(
		"AFTER", "BEFORE", "ENDFOR", "ENDIF", "FOR", "IF", "SEPARATOR",
		"abstract", "annotation", "as", "case", "catch", "class", "create",
		"def", "default", "dispatch", "do", "else", "enum", "extends",
		"extension", "false", "final", "finally", "for", "if", "implements",
		"import", "instanceof", "interface", "it", "new", "null", "override",
		"package", "private", "protected", "public", "return", "self", "static",
		"super", "switch", "synchronized", "this", "throw", "throws", "true",
		"try", "typeof", "val", "var", "while"),

	.types = WORDS(
		"Boolean", "Byte", "Character", "Double", "Float", "Integer", "Long",
		"Short", "String", "boolean", "byte", "char", "double", "float", "int",
		"long", "short", "void"),

	.line_comments = { "//" },
	.block_comments = { { "/" "*", "*" "/" } },
	.quotes = "'\"",

	.blocks = BLOCKS(
		OP_BLOCK("{", "}")),
};
