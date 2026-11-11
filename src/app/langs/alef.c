/* Alef */
#include "../lang_def.h"

LANGUAGE(alef) {
	.name = "alef",
	.title = "Alef",
	.flags = F_HASHPRE,
	.extensions = "l",

	.keywords = WORDS(
		"alloc", "alt", "become", "break", "case", "check", "const", "continue",
		"default", "do", "else", "extern", "for", "goto", "if", "inline",
		"intern", "par", "private", "proc", "raise", "register", "rescue",
		"return", "sizeof", "static", "switch", "task", "typedef", "unalloc",
		"volatile", "while", "zerox"),

	.types = WORDS(
		"Biobuf", "Fmt", "Lock", "QLock", "Rendez", "Rune", "adt", "aggr",
		"byte", "chan", "char", "double", "enum", "float", "int", "lint",
		"long", "poly", "short", "signed", "sint", "tuple", "uint", "ulint",
		"union", "unsigned", "usint", "void"),

	.constants = WORDS(
		"NULL", "nil", "stderr", "stdin", "stdout"),

	.functions = WORDS(
		"abort", "alloc", "exit", "exits", "fprint", "free", "malloc", "print",
		"sleep", "sprint", "terminate", "unalloc"),

	.preproc = WORDS(
		"define", "defined", "elif", "else", "endif", "error", "if", "ifdef",
		"ifndef", "include", "lib", "line", "pragma", "undef"),

	.line_comments = { "//" },
	.block_comments = { { "/" "*", "*" "/" } },
	.quotes = "'\"",

	.blocks = BLOCKS(
		PRE_BLOCK("#if", "#endif"),
		PRE_BLOCK("#ifdef", "#endif"),
		PRE_BLOCK("#ifndef", "#endif"),
		OP_BLOCK("{", "}")),
};
