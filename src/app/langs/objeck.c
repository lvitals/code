/* Objeck */
#include "../lang_def.h"

LANGUAGE(objeck) {
	.name = "objeck",
	.title = "Objeck",
	.extensions = "obs",

	.keywords = WORDS(
		"As", "New", "Parent", "abstract", "alias", "and", "break", "bundle",
		"class", "consts", "continue", "critical", "do", "each", "else", "enum",
		"false", "for", "from", "function", "if", "implements", "interface",
		"label", "leaving", "method", "native", "or", "other", "private",
		"public", "return", "reverse", "select", "static", "true", "use",
		"virtual", "while", "xor"),

	.types = WORDS(
		"BaseArrayHolder", "Bool", "BoolArrayHolder", "BoolHolder", "Byte",
		"ByteArrayHolder", "ByteHolder", "Char", "CharArrayHolder",
		"CharHolder", "Float", "FloatArrayHolder", "FloatHolder", "Func2Holder",
		"Func3Holder", "Func4Holder", "FuncHolder", "Int", "IntArrayHolder",
		"IntHolder", "Nil", "String", "StringArrayHolder"),

	.line_comments = { "#" },
	.block_comments = { { "#~", "~#" } },
	.quotes = "'\"",

	.blocks = BLOCKS(
		OP_BLOCK("{", "}")),
};
