/* Objective-C */
#include "../lang_def.h"

LANGUAGE(objective_c) {
	.name = "objective_c",
	.title = "Objective-C",
	.flags = F_HASHPRE | F_ATTR,
	.extensions = "m mm objc",

	.keywords = WORDS(
		"@catch", "@class", "@defs", "@encode", "@end", "@finally",
		"@implementation", "@interface", "@private", "@protected", "@protocol",
		"@public", "@selector", "@synchronized", "@throw", "@try", "FALSE",
		"METHOD_NULL", "NO", "NULL", "Nil", "TRUE", "YES", "_Bool", "_Complex",
		"_Imaginary", "_Pragma", "asm", "auto", "break", "bycopy", "byref",
		"case", "const", "continue", "default", "do", "else", "extern", "false",
		"for", "goto", "if", "in", "inline", "inout", "nil", "oneway", "out",
		"register", "restrict", "return", "self", "sizeof", "static", "super",
		"switch", "true", "typedef", "void", "volatile", "while"),

	.types = WORDS(
		"BOOL", "Class", "IMP", "MetaClass", "Object", "Protocol", "SEL", "STR",
		"TypedStream", "apply_t", "id", "retval_t"),

	.preproc = WORDS(
		"define", "elif", "else", "endif", "error", "if", "ifdef", "ifndef",
		"import", "include", "line", "pragma", "undef", "warning"),

	.line_comments = { "//" },
	.block_comments = { { "/" "*", "*" "/" } },
	.quotes = "'\"",

	.blocks = BLOCKS(
		PRE_BLOCK("region", "endregion"),
		PRE_BLOCK("if", "endif"),
		PRE_BLOCK("ifdef", "endif"),
		PRE_BLOCK("ifndef", "endif"),
		OP_BLOCK("{", "}")),
};
