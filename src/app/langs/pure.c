/* Pure */
#include "../lang_def.h"

LANGUAGE(pure) {
	.name = "pure",
	.title = "Pure",
	.extensions = "pure",

	.keywords = WORDS(
		"case", "const", "def", "else", "end", "extern", "if", "infix",
		"infixl", "infixr", "interface", "let", "namespace", "nonfix", "of",
		"otherwise", "outfix", "postfix", "prefix", "private", "public", "then",
		"type", "using", "when", "with"),

	.line_comments = { "//" },
	.block_comments = { { "/" "*", "*" "/" } },
	.quotes = "\"",
};
