/* Standard ML */
#include "../lang_def.h"

LANGUAGE(sml) {
	.name = "sml",
	.title = "Standard ML",
	.extensions = "sml fun sig",

	.keywords = WORDS(
		"abstype", "and", "andalso", "as", "case", "datatype", "do", "else",
		"end", "eqtype", "exception", "fn", "fun", "functor", "handle", "if",
		"in", "include", "infix", "infixr", "let", "local", "nonfix", "of",
		"op", "open", "orelse", "raise", "rec", "sharing", "sig", "signature",
		"struct", "structure", "then", "type", "val", "while", "with",
		"withtype"),

	.types = WORDS(
		"array", "bool", "char", "exn", "int", "list", "option", "order",
		"real", "ref", "string", "substring", "unit", "vector", "word"),

	.constants = WORDS(
		"false", "nil", "true"),

	.functions = WORDS(
		"abs", "app", "before", "ceil", "chr", "concat", "div", "exnMessage",
		"exnName", "explode", "floor", "foldl", "foldr", "getOpt", "hd",
		"ignore", "implode", "isSome", "length", "map", "mod", "not", "null",
		"o", "ord", "print", "real", "rev", "round", "size", "str", "substring",
		"tl", "trunc", "valOf", "vector"),

	.line_comments = { "(*)" },
	.block_comments = { { "(*", "*)" } },
	.quotes = "\"",
};
